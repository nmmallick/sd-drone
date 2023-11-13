#ifndef SOCKET_H_
#define SOCKET_H_

#include <string>
#include <memory>

#include <sys/socket.h>

#include <netinet/in.h>

namespace Socket
{
    typedef struct Result
    {
	std::string ip_addr;
	uint8_t *data;
	size_t bytes_received;
    } Result;

    class Socket {
	typedef uint8_t u8;
    public:

	Socket() = delete;
	Socket(const Socket&) = delete;
	Socket operator=(const Socket&) = delete;

	Socket(uint16_t port, size_t bufflen);

	/**
	 * @brief Recieve <bufflen> number of bytes from a buffer
	 * @param timeout Timeout in seconds to wait ffor <bufflen> bytes
	 *
	 * @return Receipt struct with a pointer to the data, number of bytes
	 *          received and the ip address of the sender
	 */
	Result recv(const size_t &timeout);

    private:
	// Socket descriptor
	int sockfd;

	// Server and client information
	struct sockaddr_in servaddr, cliaddr;

	// Data buffer
	std::unique_ptr<Socket::u8[]> buffer;
	size_t bufflen;
    };

} // namespace socket
#endif
