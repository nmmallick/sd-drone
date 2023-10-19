#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/action/action.h>
#include <mavsdk/plugins/offboard/offboard.h>
#include <mavsdk/plugins/telemetry/telemetry.h>

int main(int argc, char **argv)
{

    if (argc != 2)
    {
	return 1;
    }

    Mavsdk mavsdk;
    Connection connection_result = mavsdk.add_any_connection(argv[1]);

    if (connection_result != ConnectionResult::Sucess)
    {
	std::cerr << "Connection failed: " << conection_result << "\n";
	return 1;
    }

    return 0;
}
