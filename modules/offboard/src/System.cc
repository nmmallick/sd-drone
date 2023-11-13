#include <iostream>
#include <future>
#include <cmath>

#include <System.hh>

namespace offboard
{
    // System:: function defines
    System::System(const system_config &conf)
	: config(conf)
    {

	drone_interface = std::make_unique<DroneInterface>();
	size_t tries = 0;
	while (!drone_interface->connect(config.fcu_uri))
	{
	    if (5 > tries++)
		throw std::runtime_error("could not  connect to drone");
	    std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	// Configuartion for the network interface to rpi4
	config.network_buflen = 16;
	network_interface = std::make_unique<NetworkInterface>(config.port, config.network_buflen);
	network_interface->setDataCallback([this] (uint8_t *buffer) {
					       auto data = *reinterpret_cast<attitude *>(buffer);
					       data.roll = data.roll*(180./M_PI);
					       data.pitch = data.pitch*(180./M_PI);
					       data.yaw = data.yaw*(180./M_PI);

					       // std::cout << " [T:" << data.thrust << ", "
					       // 		 << "R:" << data.roll << ", "
					       // 		 << "P:" << data.pitch << ", "
					       // 		 << "Y:" << data.yaw << "]" << std::endl;

					       drone_interface->setControls(data);
					   });

	network_interface->setConnectionCallback([this] (bool connected)
						 {
						     std::cout << "setting drone hold position" << std::endl;
						     drone_interface->holdPosition(!connected);
						 });

	network_interface->start(100);
    }

    bool System::start_offboard()
    {
	done = false;

	if (!drone_interface->start())
	{
	    std::cerr << "could not start offboard" << std::endl;
	    return false;
	}
	std::cout << "finished startup" << std::endl;

	if (!network_interface->isConnected())
	    drone_interface->holdPosition(true);

	drone_interface->startLoop();

	while (!done)
	{
	    if (!network_interface->isConnected())
	    {
		std::cout << "no network connection" << std::endl;
	    }
	    // Check system health

	    std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	drone_interface->stop();

	return true;
    }

    void System::stop()
    {
	network_interface->stop();
	done = true;
    }


    // NetworkInterface:: function defines //
    NetworkInterface::NetworkInterface(const uint32_t &port, const size_t &buflen)
    {
	// Create the udp socket interface object
	interface = std::make_unique<Socket::Socket>(port, buflen);
    }

    void NetworkInterface::start(const size_t &timeout)
    {
	done = false;
	thread = std::thread([&] (const size_t &timeout)
			     {
				 size_t tries = 0;
				 while (!done)
				 {

				     auto result = interface->recv(timeout);
				     if (result.data)
				     {
					 if (data_cb)
					     data_cb(result.data);

					 // Reset how many times we had a good udp stream
					 if (conn_cb && !good_connection)
					     conn_cb(!good_connection);
					 good_connection = true;

					 tries = 0;
				     }
				     else
				     {
					 // If we get 10 consecutive timeouts we assume
					 // a disconnect
					 if (10 == tries++)
					 {
					     good_connection = false;
					     if (conn_cb)
						 conn_cb(good_connection);
					 } else
					 {
					     tries++;
					 }
				     }
				 }
			     }, timeout);
    }

    void NetworkInterface::setDataCallback(NetworkInterface::DataCallback &&func)
    {
	data_cb = std::move(func);
    }

    void NetworkInterface::setConnectionCallback(NetworkInterface::ConnectionCallback &&func)
    {
	conn_cb = std::move(func);
    }

    void NetworkInterface::stop()
    {
	Lock lk(worker_mutex);
	done = true;
	thread.join();
    }

    NetworkInterface::~NetworkInterface()
    {
	stop();
    }

    bool NetworkInterface::busy() const
    {
	return !thread.joinable();
    }

    bool NetworkInterface::isConnected() const
    {
	return good_connection.load();
    }


    // DroneInterface function definitions //
    DroneInterface::DroneInterface() :
	mavsdk(std::make_unique<mavsdk::Mavsdk>())
    {
    }

    bool DroneInterface::connect(const std::string &uri)
    {
	using namespace mavsdk;
	mavsdk->add_any_connection(uri);

	const auto optional = mavsdk->first_autopilot(3.0);

	if (!optional)
	{
	    std::cout << "could not connect to autopilot" << std::endl;
	    return false;
	}

	autopilot = optional.value();

	telemetry = std::make_unique<Telemetry>(autopilot);
	offboard = std::make_unique<Offboard>(autopilot);
	action = std::make_unique<Action>(autopilot);

	return true;
    }

    bool DroneInterface::start()
    {
	using namespace mavsdk;

	// const auto rc_status = telemetry->rc_status();
	// if (!rc_status.is_available)
	// {
	//     std::cerr << "RC is not available\n";
	//     return false;
	// }

	// Check everything is ok before we start
	while (!telemetry->health_all_ok())
	{
	    std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	std::cout << "System is ready\n";

	// Armed?
	const auto arm_result = action->arm();
	if (arm_result != Action::Result::Success)
	{
	    std::cout << "Could not arm drone" << std::endl;
	    return false;
	}

	// In air?
	const auto takeoff_result = action->takeoff();
	if (takeoff_result != Action::Result::Success)
	{
	    std::cerr << "Takeoff failed: " << takeoff_result << "\n";
	    return false;
	}

	// Wait for take off to be finished
	auto in_air_promise = std::promise<void>{};
	auto in_air_future = in_air_promise.get_future();
	Telemetry::LandedStateHandle handle = telemetry->subscribe_landed_state(
	    [this, &in_air_promise, &handle](Telemetry::LandedState state) {
		if (state == Telemetry::LandedState::InAir) {
		    std::cout << "Taking off has finished\n.";
		    telemetry->unsubscribe_landed_state(handle);
		    in_air_promise.set_value();
		}
	    });

	in_air_future.wait_for(std::chrono::seconds(10));
	if (in_air_future.wait_for(std::chrono::seconds(3)) == std::future_status::timeout)
	{
	    std::cerr << "Takeoff timed out." << std::endl;
	    return false;
	}
	//

	return true;
    }

    bool DroneInterface::stop()
    {
	using namespace mavsdk;

	// Stop the thread
	done = true;

	const auto land_result = action->land();
	if (land_result != Action::Result::Success) {
	    std::cerr << "Landing failed: " << land_result << '\n';
	    return 1;
	}

	/**** [LAND] ****/
	// Check if vehicle is still in air
	while (telemetry->in_air()) {
	    std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	// We are relying on auto-disarming but let's keep watching the telemetry for
	// a bit longer.
	std::this_thread::sleep_for(std::chrono::seconds(3));

	return true;
    }

    bool DroneInterface::startLoop()
    {
	done = false;
	thread = std::thread([this]()
			     {
				 using namespace mavsdk;

				 // Hold position command (empty field)
				 offboard->set_velocity_body(hold_pos);

				 // Start
				 Offboard::Result offboard_result = offboard->start();
				 if (offboard_result != Offboard::Result::Success)
				 {
				     std::cerr << "could not start offboard control\n";
				     return;
				 }

				 std::cout << "Offboard started\n";
				 while (!done)
				 {
				     if (hold_pos_flag)
				     	 offboard->set_velocity_body(hold_pos);
				     else
					 offboard->set_velocity_body(controls);
				 }
			     });

	return true;
    }

    void DroneInterface::setControls(const attitude &input)
    {
	Lock lk(control_mutex);

	controls.right_m_s = (input.roll)*0.1;
	controls.forward_m_s = (input.pitch)*0.1;
	controls.yawspeed_deg_s = (input.yaw)*0.4;

	controls.down_m_s = -input.thrust;
    }

    void DroneInterface::holdPosition(const bool &hold)
    {
	hold_pos_flag = hold;
    }


}
