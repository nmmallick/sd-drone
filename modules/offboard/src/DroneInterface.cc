#include <iostream>
#include <future>

#include <math.h>

#include <DroneInterface.hh>

namespace offboard
{
    DroneInterface::DroneInterface() :
	mavsdk(std::make_unique<mavsdk::Mavsdk>()),
	need_rc(false)
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
	{
	    std::cerr << "Drone is not ready.\n";
	    return false;
	}

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

	return true;
    }

    bool DroneInterface::stop()
    {
	using namespace mavsdk;

	// Start
	Offboard::Result offboard_result = offboard->stop();
	if (offboard_result != Offboard::Result::Success)
	{
	    std::cerr << "could not stop offboard control\n";
	    // kill drone??
	}

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
					 offboard->set_velocity_body(controls);
				     }
				 }
			     });

	return true;
    }

    void DroneInterface::setControls(const attitude &input)
    {
	Lock lk(control_mutex);

	// 5.0 degrees is the threshold in which we want to actually
	// starting caring about head position. This is so any little
	// movement won't contribute to any unwanted drifting
	if (std::abs(input.roll) > 5.0)
	    controls.right_m_s = (input.roll)*0.1;
	else
	    controls.right_m_s = 0.0;

	if (std::abs(input.pitch) > 5.0)
	    controls.forward_m_s = (input.pitch)*0.1;
	else
	    controls.forward_m_s = 0.0;

	if (std::abs(input.yaw) > 5.0)
	{
	    // Since we are squaring the input yaw values, we want to
	    // retain the sign for direction
	    auto sign = input.yaw >= 0 ? 1 : -1;
	    controls.yawspeed_deg_s = sign*pow(input.yaw, 2)*0.05;
	} else
	{
	    controls.yawspeed_deg_s = 0.0;
	}

	controls.down_m_s = -input.thrust;
    }

    void DroneInterface::holdPosition(const bool &hold)
    {
	hold_pos_flag = hold;
    }

    bool DroneInterface::isReady() const
    {
	if (need_rc)
	{
	    const auto rc_status = telemetry->rc_status();
	    if (!rc_status.is_available)
	    {
		std::cerr << "RC is not available\n";
		return false;
	    }
	}

	// Check everything is ok before we start
	std::cout << "Waiting for drone health." << std::endl;
	while (!telemetry->health_all_ok())
	{
	    std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	if (!telemetry->health_all_ok())
	{
	    const auto health = telemetry->health();

	    if (!health.is_gyrometer_calibration_ok)
		std::cerr << "gyrometer not calibrated" << std::endl;

	    if (!health.is_accelerometer_calibration_ok)
		std::cerr << "acceleromter not calibrated" << std::endl;

	    if (!health.is_magnetometer_calibration_ok)
		std::cerr << "magnetometer cnot calibrated" << std::endl;

	    if (!health.is_local_position_ok)
		std::cerr << "local position not ok" << std::endl;

	    if (!health.is_global_position_ok)
		std::cerr << "global position not ok" << std::endl;

	    if (!health.is_home_position_ok)
		std::cerr << "home position not ok" << std::endl;

	    if (!health.is_armable)
		std::cerr << "drone not armable" << std::endl;

	    return false;
	}

	return true;
    }

    bool DroneInterface::isRunning() const
    {
	return !(done.load());
    }

    void DroneInterface::setRequireRC(const bool &val)
    {
	need_rc = val;
    }
}
