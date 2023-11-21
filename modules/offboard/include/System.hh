#pragma once

#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>
#include <optional>

#include <NetworkInterface.hh>
#include <DroneInterface.hh>

namespace offboard
{
    typedef struct system_config
    {
	uint32_t port;
	std::string fcu_uri;
	size_t network_buflen;

	system_config()
	{
	    port = 65432;
	    fcu_uri = "udp:://:14540";
	    network_buflen = 1024;
	}
    } system_config;

    enum class system_status
    {
	GOOD=0,
	NOT_CONNECTED,
	NO_AUTOPILOT,
	UNINITIALIZED
    };

    class System
    {
    public:
	System(const system_config &conf);

	bool run();
	void stop();

    private:

	// Check system health
	system_status check_system_health();

	// Network socket interface to recieve control input
	std::unique_ptr<NetworkInterface> network_interface;
	std::unique_ptr<DroneInterface> drone_interface;

	std::optional<attitude> attitude_data = std::nullopt;

	system_config config;

	// Default to true so if something happens in
	// setup, we never allow offboard to loop
	bool done = true;
	std::atomic<bool> trigger_offboard = false;
    };
}


