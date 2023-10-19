// Server side implementation of UDP client-server model
#include <bits/stdc++.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <math.h>

#include <string>

#define PORT	 65432
#define MAXLINE 16

typedef float f32;

typedef struct Quaternion
{
    f32 q0, q1, q2, q3;
} Quaternion;

typedef struct EulerAngles
{
    f32 roll, pitch, yaw;
} EulerAngles;

// Convert a quaternion to z-y-x euler angles (yaw, pitch, roll)
EulerAngles to_euler(Quaternion q)
{
    EulerAngles angles;

    // Roll
    f32 sinr_cosp = 2*((q.q0*q.q1) + (q.q2*q.q3));
    f32 cosr_cosp = 1 - 2*((q.q1*q.q1) + (q.q2*q.q2));

    angles.roll = atan2(sinr_cosp, cosr_cosp);

    // Pitch
    f32 sinp = sqrt(1 + 2*(q.q0*q.q2 - q.q1*q.q3));
    f32 cosp = sqrt(1 - 2*(q.q0*q.q2 - q.q1*q.q3));

    angles.pitch = 2*atan2(sinp, cosp) - M_PI_2;

    // yaw
    f32 siny_cosp = 2*(q.q0*q.q3 + q.q1*q.q2);
    f32 cosy_cosp = 1 - 2*(q.q2*q.q2 + q.q3*q.q3);

    angles.yaw = atan2(siny_cosp, cosy_cosp);

    return angles;
}

// Driver code
int main() {

    int sockfd;
    char buffer[MAXLINE] = {'\0'};

    struct sockaddr_in servaddr, cliaddr;

    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
	perror("socket creation failed");
	exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // Filling server information
    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    // Bind the socket with the server address
    if ( bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 )
    {
	perror("bind failed");
	exit(EXIT_FAILURE);
    }

    socklen_t len;
    len = sizeof(cliaddr); //len is value/result

    Quaternion q;
    char str_addr[16];
    int n;

    while (1)
    {
	n = recvfrom(sockfd, (char *)buffer, MAXLINE,
		     MSG_WAITALL, ( struct sockaddr *) &cliaddr,
		     &len);

	memcpy(&q.q0, &buffer, 4);
	memcpy(&q.q1, &buffer[4], 4);
	memcpy(&q.q2, &buffer[8], 4);
	memcpy(&q.q3, &buffer[12], 4);

	const auto angles = to_euler(q);

	inet_ntop(cliaddr.sin_family, &cliaddr.sin_addr, str_addr, sizeof(str_addr));
    }

    return 0;
}
