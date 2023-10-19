class Socket {

    typedef uint8_t u8;
public:

    Socket() = delete;
    Socket(size_t buffLen);

    u8 *buffer();
private:

    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    std::unique_ptr<uint8_t[]> buffer;

};



Socket::Socket(size_t buffLen)
{
    // Allocate memory for buffer
    buffer = std::make_unique<Socket::u8[]>(new uint8_t[bufflen]());

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));


    // Filling server information
    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    // Bind the socket with the server address
    if ( bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 )
	throw std::runtime_error("Socket(): could not open socket");

}
