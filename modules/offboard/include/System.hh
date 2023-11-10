#include <Offboard.hh>
#include <Socket.hh>

namespace system
{
    enum class connection_result
    {
	GOOD,
	NO_FCU,
	NO_UDP,
	NO_CONNECTIONS
    };

    struct system_health
    {
	connection_result conn;
    } system_health;

    struct system_config
    {
	std::string client_uri;
	std::string fcu_uri;

    } system_config;

    class System
    {
    public:


    private:

	// Check system health
	system_health check_system_health();

	// Create connections
	connection_result create_connection();

	// Network socket interface to recieve control input
	std::unique_ptr<Socket::Socket> udp_interface_;

	// Interface to PX4 FCU
	std::unique_ptr<mavsdk::Mavsdk> fcu_interface_;

	bool _done_;
    };
}
