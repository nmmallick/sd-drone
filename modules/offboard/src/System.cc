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
	size_t tries = 0;
	drone_interface = std::make_unique<DroneInterface>();
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

					       drone_interface->setControls(data);
					   });

	network_interface->setConnectionCallback([this] (bool connected)
						 {
						     std::cout << "setting drone hold position" << std::endl;
						     drone_interface->holdPosition(!connected);
						 });

	network_interface->start(100);
    }

    bool System::run()
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
}
