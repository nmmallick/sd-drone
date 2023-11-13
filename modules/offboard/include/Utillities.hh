#ifndef UTILLITIES_H_
#define UTILLITIES_H_

#include <stdint.h>

#define TO_DEG 180./M_PI
#define TO_RAD M_PI/180.

typedef float f32;

typedef struct Quaternion
{
    f32 q0, q1, q2, q3;
} Quaternion;

typedef struct EulerAngles
{
    f32 roll, pitch, yaw;
} EulerAngles;

/**
 * @brief Convert a quaternion vector to euler angles (ZYX)
 * @param q Quaternion struct to convert to euler angles
 *
 * @return EulerAngles struct with the convert quaternion
 */
EulerAngles to_euler(Quaternion q, bool inDegress);


/**
 * @brief Convert a serialize buffer of 16 bytes to a quaternion
 * @param buf The buffer contain 16 bytes of data to convert
 *
 * @return A quaternion struct
 */
Quaternion deserialize_buffer(const uint8_t *buf);

#endif



