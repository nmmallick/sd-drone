#include <Socket.hh>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>

#include <cstring>
#include <iostream>
#include <chrono>

namespace Socket
{
    Socket::Socket(uint16_t port, size_t bufflen=1024)
    {
	// Allocate memory for buffer
	this->bufflen = bufflen;

	buffer = std::make_unique<Socket::u8[]>(this->bufflen);
	memset(&servaddr, 0, sizeof(servaddr));

	// Filling server information
	servaddr.sin_family = AF_INET; // IPv4
	servaddr.sin_addr.s_addr = INADDR_ANY; // listen to any route ip on this cpu
	servaddr.sin_port = htons(port);

	// Create the so cket file descriptor
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	    throw std::runtime_error("Socket(): could not open descriptor");

	// Bind the socket with the server address
	if ( bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 )
	    throw std::runtime_error("Socket(): could not open socket");
    }

    Result Socket::recv(const size_t &timeout)
    {
	Result res;
	char str_addr[16] = {'\0'};

	memset(&res, 0, sizeof(Result));
	memset(&cliaddr, 0, sizeof(cliaddr));

	socklen_t len = sizeof(cliaddr);

	auto start = std::chrono::steady_clock::now();
	auto curr = std::chrono::steady_clock::now();

	while ((res.bytes_received < bufflen))
	{
	    auto num_bytes = ::recvfrom(sockfd,
				      (uint8_t *)(buffer.get() + res.bytes_received),
				      (bufflen - res.bytes_received),
				      MSG_DONTWAIT,
				      (struct sockaddr *) &cliaddr,
				      &len);

	    curr = std::chrono::steady_clock::now();
	    auto ellapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(curr - start);

	    if (ellapsed_time.count() > timeout)
	    {
		res.data = NULL;
	    	return res;
	    }

	    if (num_bytes < 0)
	    {
		if (errno == EAGAIN)
		    continue;

		std::cout << strerror(errno) << std::endl;
		break;
	    }

	    res.bytes_received += num_bytes;
	}

	inet_ntop(cliaddr.sin_family, &cliaddr.sin_addr, str_addr, sizeof(str_addr));
	res.data = buffer.get();
	return res;
    }
} // namespace socket
