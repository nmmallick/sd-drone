#include <Socket.hh>
#include <math.h>
#include <cstring>
#include <iostream>

#define PORT 65432

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

    Socket sock(PORT, 16);
    Quaternion q;

    while (1)
    {
	auto rec = sock.recv();

	memcpy(&q.q0, &(rec.data[0]), 4);
	memcpy(&q.q1, &(rec.data[4]), 4);
	memcpy(&q.q2, &(rec.data[8]), 4);
	memcpy(&q.q3, &(rec.data[12]), 4);

	const auto angles = to_euler(q);

	printf("roll: %.2f\tpitch: %.2f\tyaw: %.2f\n", angles.roll, angles.pitch, angles.yaw);
    }

    return 0;
}
