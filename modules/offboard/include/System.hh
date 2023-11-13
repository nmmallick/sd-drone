#pragma once

#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>
#include <optional>

#include <Socket.hh>

#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/action/action.h>
#include <mavsdk/plugins/offboard/offboard.h>
#include <mavsdk/plugins/telemetry/telemetry.h>

namespace offboard
{
    typedef struct attitude
    {
	float roll,
	    pitch,
	    yaw,
	    thrust;
	attitude()
	{
	    roll = 0.0;
	    pitch = 0.0;
	    yaw = 0.0;
	    thrust = 0.0;
	}
    } attitude;

    typedef struct system_config
    {
	uint32_t port;
	std::string fcu_uri;
	size_t network_buflen;

	system_config()
	{
	    port = 65432;
	    fcu_uri = "udp:://192.168.0.4:14540";
	    network_buflen = 1024;
	}
    } system_config;


    /**
     * @brief Recieve only network interface
     */
    class NetworkInterface
    {
	// Aliasing
	using DataCallback = std::function<void(uint8_t *)>;
	using ConnectionCallback = std::function<void(bool)>;

	using SocketPtr = std::unique_ptr<Socket::Socket>;

	using Port = uint32_t;

	using Worker = std::thread;
	using Mutex = std::mutex;
	using Lock = std::lock_guard<Mutex>;

	using Flag = std::atomic<bool>;
    public:

	NetworkInterface(const uint32_t &port, const size_t &buflen);
	~NetworkInterface();

	void setDataCallback(DataCallback &&cb);
	void setConnectionCallback(ConnectionCallback &&cb);

	void start(const size_t &timeout);
	void stop();

	bool busy() const;
	bool isConnected() const;

    private:
	Port PORT;
	SocketPtr interface = nullptr;
	DataCallback data_cb;
	ConnectionCallback conn_cb;

	Worker thread;
	Mutex worker_mutex;

	Flag done, good_connection;
    };

    /**
     * @brief Drone interface class
     */

    class DroneInterface
    {
	using Flag = std::atomic<bool>;
	using Controls = mavsdk::Offboard::VelocityBodyYawspeed;
	using Worker = std::thread;
	using Mutex = std::mutex;
	using Lock = std::lock_guard<Mutex>;
    public:
	DroneInterface();
	bool connect(const std::string &uri);

	/**
	 * This starts flight operations
	 * TODO: this should be a start up (init->arm->takeoff)
	 */
	bool start();

	/**
	 *
	 */
	bool startLoop();

	/**
	 * This stops and lands all flight operations
	 */
	bool stop();


	void setControls(const attitude &attitude);
	void holdPosition(const bool &hold);

    private:
	// Interface to PX4 FCU
	std::unique_ptr<mavsdk::Mavsdk> mavsdk = nullptr;
	std::shared_ptr<mavsdk::System> autopilot;

	std::unique_ptr<mavsdk::Action> action;
	std::unique_ptr<mavsdk::Telemetry> telemetry;
	std::unique_ptr<mavsdk::Offboard> offboard;

	Mutex control_mutex;
	const Controls hold_pos = Controls{};
	Controls controls = Controls{};

	Flag hold_pos_flag;
	Flag done;

	Worker thread;
    };

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

	bool start_offboard();
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


