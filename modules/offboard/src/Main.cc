#include <System.hh>
#include <csignal>

offboard::System *sys_ptr_ = nullptr;
bool stop = false;

void signalHandler(int signum)
{
    if (sys_ptr_)
	sys_ptr_->stop();
}

int main(int argc, char **argv)
{
    signal(SIGINT, signalHandler);

    offboard::system_config conf;
    conf.fcu_uri = "udp://:14540";
    conf.port = 65432;

    auto sys_interface = offboard::System(conf);

    sys_ptr_ = &sys_interface;
    sys_interface.start_offboard();

    return 0;
}
