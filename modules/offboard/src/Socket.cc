#include <Socket.hh>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <cstring>
#include <iostream>

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

Receipt Socket::recv()
{
    Receipt rec;
    char str_addr[16] = {'\0'};

    memset(&rec, 0, sizeof(Receipt));
    memset(&cliaddr, 0, sizeof(cliaddr));

    socklen_t len = sizeof(cliaddr);

    float q0, q1, q2, q3;

    rec.bytes_received = recvfrom(sockfd, (uint8_t *)buffer.get(), bufflen,
				  MSG_WAITALL, (struct sockaddr *) &cliaddr,
				  &len);

    inet_ntop(cliaddr.sin_family, &cliaddr.sin_addr, str_addr, sizeof(str_addr));
    rec.data = buffer.get();

    return rec;
}
