#include <functional>
#include <thread>
#include <mutex>
#include <atomic>

#include <Socket.hh>

namespace offboard
{
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
}
