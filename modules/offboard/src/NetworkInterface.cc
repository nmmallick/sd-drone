#include <Socket.hh>
#include <NetworkInterface.hh>

namespace offboard
{
    NetworkInterface::NetworkInterface(const uint32_t &port, const size_t &buflen)
    {
	// Create the udp socket interface object
	interface = std::make_unique<Socket::Socket>(port, buflen);
    }

    void NetworkInterface::start(const size_t &timeout)
    {
	done = false;
	thread = std::thread([&] (const size_t &timeout)
			     {
				 size_t tries = 0;
				 while (!done)
				 {

				     auto result = interface->recv(timeout);
				     if (result.data)
				     {
					 if (data_cb)
					     data_cb(result.data);

					 // Reset how many times we had a good udp stream
					 if (conn_cb && !good_connection)
					     conn_cb(!good_connection);
					 good_connection = true;

					 tries = 0;
				     }
				     else
				     {
					 // If we get 10 consecutive timeouts we assume
					 // a disconnect
					 if (10 == tries++)
					 {
					     good_connection = false;
					     if (conn_cb)
						 conn_cb(good_connection);
					 } else
					 {
					     tries++;
					 }
				     }
				 }
			     }, timeout);
    }

    void NetworkInterface::setDataCallback(NetworkInterface::DataCallback &&func)
    {
	data_cb = std::move(func);
    }

    void NetworkInterface::setConnectionCallback(NetworkInterface::ConnectionCallback &&func)
    {
	conn_cb = std::move(func);
    }

    void NetworkInterface::stop()
    {
	Lock lk(worker_mutex);
	done = true;
	thread.join();
    }

    NetworkInterface::~NetworkInterface()
    {
	stop();
    }

    bool NetworkInterface::busy() const
    {
	return !thread.joinable();
    }

    bool NetworkInterface::isConnected() const
    {
	return good_connection.load();
    }
}
