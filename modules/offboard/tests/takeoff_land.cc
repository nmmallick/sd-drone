#include <chrono>
#include <iostream>

#include <DroneInterface.hh>

int main(int argc, char **argv)
{
    auto drone = offboard::DroneInterface();
    drone.setRequireRC(false);

    drone.connect("udp://:14540");

    // Arm and takeoff
    if (!drone.start())
	throw std::runtime_error("Could not take arm and take off.");

    std::cout << "In air\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::cout << "Landing drone\n";
    drone.stop();

    return 0;
}
