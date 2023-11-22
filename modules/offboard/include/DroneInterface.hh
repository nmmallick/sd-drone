#include <thread>
#include <mutex>
#include <atomic>

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

	bool isReady() const;

	bool isRunning() const;

	void setControls(const attitude &attitude);
	void holdPosition(const bool &hold);

	void setRequireRC(const bool &val);

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

	bool need_rc;
    };
}
