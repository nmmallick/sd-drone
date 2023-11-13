#include <Utillities.hh>
#include <math.h>
#include <cstring>

// Convert a quaternion to z-y-x euler angles (yaw, pitch, roll)
EulerAngles to_euler(Quaternion q, bool inDegrees = false)
{
    EulerAngles angles;

    // Roll
    f32 sinr_cosp = 2*((q.q0*q.q1) + (q.q2*q.q3));
    f32 cosr_cosp = 1 - 2*((q.q1*q.q1) + (q.q2*q.q2));

    angles.roll = atan2(sinr_cosp, cosr_cosp);

    // Pitch
    f32 sinp = sqrt(1 + 2*(q.q0*q.q2 - q.q1*q.q3));
    f32 cosp = sqrt(1 - 2*(q.q0*q.q2 - q.q1*q.q3));

    angles.pitch = (2*atan2(sinp, cosp) - M_PI_2);

    // yaw
    f32 siny_cosp = 2*(q.q0*q.q3 + q.q1*q.q2);
    f32 cosy_cosp = 1 - 2*(q.q2*q.q2 + q.q3*q.q3);

    angles.yaw = atan2(siny_cosp, cosy_cosp);

    if (inDegrees)
    {
	angles.roll = angles.roll*TO_DEG;
	angles.pitch = angles.pitch*TO_DEG;
	angles.yaw = angles.yaw*TO_DEG;
    }

    return angles;
}

Quaternion deserialize_buffer(const uint8_t *buf)
{
    Quaternion q;

    memcpy(&q.q0, &(buf[0]), 4);
    memcpy(&q.q1, &(buf[4]), 4);
    memcpy(&q.q2, &(buf[8]), 4);
    memcpy(&q.q3, &(buf[12]), 4);

    return q;
}
