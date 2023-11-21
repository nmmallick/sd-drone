#include <iostream>
#include <future>

#include <DroneInterface.hh>

namespace offboard
{
    std::string statusToString(const DroneStatus &status)
    {
	switch(status)
	{
	case DroneStatus::BAD_HEALTH:
	    return "Bad Health";
	case DroneStatus::NO_RC:
	    return "No RC";
	case DroneStatus::GOOD:
	    return "Good";
	default:
	    break;
	}
	return "Unknown";
    }

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

	if (!isReady())
	    return false;

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

	return startLoop();
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
				     {
					 offboard->set_velocity_body(hold_pos);
				     }
				     else
				     {
					 Lock lk(control_mutex);
					 offboard->set_velocity_body(controls);
				     }
				 }
			     });

	return true;
    }

    void DroneInterface::setControls(const attitude &input)
    {
	Lock lk(control_mutex);

	controls.right_m_s = (input.roll)*0.1;
	controls.forward_m_s = (input.pitch)*0.1;
	controls.yawspeed_deg_s = (input.yaw)*0.2;

	controls.down_m_s = -input.thrust;
    }

    void DroneInterface::holdPosition(const bool &hold)
    {
	hold_pos_flag = hold;
    }

    bool DroneInterface::isReady() const
    {
	const auto rc_status = telemetry->rc_status();
	if (!rc_status.is_available)
	{
	    std::cerr << "RC is not available\n";
	    return false;
	}

	// Check everything is ok before we start
	if (!telemetry->health_all_ok())
	{
	    return false;
	}

	return true;
    }

    bool DroneInterface::isRunning() const
    {
	return !(done.load());
    }
}
