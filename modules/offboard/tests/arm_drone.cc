#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/action/action.h>
#include <mavsdk/plugins/offboard/action.h>
#include <mavsdk/plugins/telemetry/telemetry.h>

#include <chrono>

int main(int argc, char **argv)
{

    using std::chrono::seconds;
    using std::this_thread::sleep_for;

    mavsdk::Mavsdk mavsdk;
    mavsdk::ConnectionResult connection_result = mavsdk.add_any_connection("udp://192.168.0.4:14540");

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
    auto action = Action{system.value()};
    auto offboard = Offboard{system.value()};
    auto telemetry = Telemetry{system.value()};

    while (!telemetry.health_all_ok())
    {
	std::cout << "Waiting for system to be ready.\n";
	sleep_for(seconds(1));
    }
    std::cout << "System is ready.\n";

    const auto arm_result = action.arm();
    if (arm_result != Action::Result::Success)
    {
	ss << "Arming failed: " << arm_result << "\n";
	throw std::runtime_error(ss.str());
    }

    std::cout << "Armed successful! Looping..." << std::endl;
    while (1);

    return 0;
}
