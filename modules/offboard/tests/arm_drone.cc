 #include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/action/action.h>
#include <mavsdk/plugins/offboard/offboard.h>
#include <mavsdk/plugins/telemetry/telemetry.h>

#include <chrono>
#include <iostream>
#include <thread>

int main(int argc, char **argv)
{

    using std::chrono::seconds;
    using std::this_thread::sleep_for;

    mavsdk::Mavsdk mavsdk;
    mavsdk::ConnectionResult connection_result = mavsdk.add_any_connection("udp://:14540");

    std::stringstream ss;
    if (connection_result != mavsdk::ConnectionResult::Success)
    {
	ss << "Connection failed: " << connection_result << "\n";
	throw std::runtime_error(ss.str());
    }

    auto system = mavsdk.first_autopilot(3.0);
    if (!system)
	throw std::runtime_error("Timed out waiting for system.");

    // Instantiate plugins
    auto action = mavsdk::Action{system.value()};

    const auto arm_result = action.arm();
    if (arm_result != mavsdk::Action::Result::Success)
    {
	ss << "Arming failed: " << arm_result << "\n";
	throw std::runtime_error(ss.str());
    }

    std::cout << "Armed successfully! Looping..." << std::endl;
    while (1);

    return 0;
}
