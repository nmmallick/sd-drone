#include <Socket.hh>
#include <Utillities.hh>

#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/action/action.h>
#include <mavsdk/plugins/offboard/offboard.h>
#include <mavsdk/plugins/telemetry/telemetry.h>

#include <csignal>
#include <chrono>
#include <thread>
#include <sstream>
#include <cstring>
#include <iostream>
#include <math.h>
#include <future>

#define PORT 65432

bool done = false;
using namespace mavsdk;

void handler(int sig_num)
{
    done = true;
}

void offboard_routine(mavsdk::Offboard *offboard)
{
    std::cout << "Starting offboard control\n";
    std::stringstream ss;

    Socket::Socket sock(PORT, 16);
    Quaternion q;

    // Get reference yaw angle
    std::cout << "Calibrating yaw reference angle" << std::endl;
    bool avg_done = false;
    double yaw_ref = 0;
    size_t itr = 0;
    while ((itr <= 1000) && !done)
    {
	const auto res = sock.recv();

	const auto angles = to_euler(deserialize_buffer(res.data), true);
	yaw_ref += angles.yaw;
	itr++;
    }
    yaw_ref/=itr;

    std::cout << "Yaw ref angle : " << yaw_ref << "\n";

    // Hold position command (empty field)
    Offboard::VelocityBodyYawspeed ctl{};
    offboard->set_velocity_body(ctl);

    // Start
    Offboard::Result offboard_result = offboard->start();
    if (offboard_result != Offboard::Result::Success)
    {
	ss << "Offboard start failed: " << offboard_result << "\n";
	throw std::runtime_error(ss.str());
    }
    std::cout << "Offboard started\n";

    while (!done)
    {
	auto rec = sock.recv();

	const auto q = deserialize_buffer(rec.data);
	const auto angles = to_euler(q, true);

	ctl.right_m_s = (angles.roll)*0.1;
	ctl.forward_m_s = (angles.pitch)*0.1;
	ctl.yawspeed_deg_s = (yaw_ref - angles.yaw)*0.4;

	offboard->set_velocity_body(ctl);
    }

    offboard_result = offboard->stop();
    if (offboard_result != Offboard::Result::Success) {
	ss << "Offboard stop failed: " << offboard_result << "\n";
	throw std::runtime_error(ss.str());
    }
    std::cout << "Offboard stopped\n";
}

int main()
{
    using std::chrono::seconds;
    using std::this_thread::sleep_for;

    signal(SIGINT, handler);

    // Connection to simulator
    Mavsdk mavsdk;
    ConnectionResult connection_result = mavsdk.add_any_connection("udp://:14540");

    std::stringstream ss;
    if (connection_result != ConnectionResult::Success)
    {
	ss << "Connection failed: " << connection_result << "\n";
	throw std::runtime_error(ss.str());
    }

    auto system = mavsdk.first_autopilot(3.0);
    if (!system)
	throw std::runtime_error("Timed out waiting for system");

    // Instantiate plugins
    auto action = Action{system.value()};
    auto offboard = Offboard{system.value()};
    auto telemetry = Telemetry{system.value()};

    while (!telemetry.health_all_ok())
    {
	std::cout << "Waiting for system to be ready\n";
	sleep_for(seconds(1));
    }
    std::cout << "System is ready\n";

    // Arm drone
    const auto arm_result = action.arm();
    if (arm_result != Action::Result::Success)
    {
	ss << "Arming failed: " << arm_result << "\n";
	throw std::runtime_error(ss.str());
    }

    const auto takeoff_result = action.takeoff();
    if (takeoff_result != Action::Result::Success)
    {
	ss << "Takeoff failed: " << takeoff_result << "\n";
	throw std::runtime_error(ss.str());
    }

    // Wait for take off to be finished
    auto in_air_promise = std::promise<void>{};
    auto in_air_future = in_air_promise.get_future();
    Telemetry::LandedStateHandle handle = telemetry.subscribe_landed_state(
        [&telemetry, &in_air_promise, &handle](Telemetry::LandedState state) {
            if (state == Telemetry::LandedState::InAir) {
                std::cout << "Taking off has finished\n.";
                telemetry.unsubscribe_landed_state(handle);
                in_air_promise.set_value();
            }
        });
    in_air_future.wait_for(seconds(10));
    if (in_air_future.wait_for(seconds(3)) == std::future_status::timeout)
        throw std::runtime_error("Takeoff timed out.");


    /**** [OFFBOARD] ****/
    //offboard_routine(&offboard);
    const auto rc_status = telemetry.rc_status();
    if (rc_status.is_available)
	std::cout << "RC available\n";

    offboard_routine(&offboard);

    const auto land_result = action.land();
    if (land_result != Action::Result::Success) {
        std::cerr << "Landing failed: " << land_result << '\n';
        return 1;
    }

    /**** [LAND] ****/
    // Check if vehicle is still in air
    while (telemetry.in_air()) {
        std::cout << "Vehicle is landing...\n";
        sleep_for(seconds(1));
    }
    std::cout << "Landed!\n";

    // We are relying on auto-disarming but let's keep watching the telemetry for
    // a bit longer.
    sleep_for(seconds(3));
    std::cout << "Finished...\n";

    return 0;
}
