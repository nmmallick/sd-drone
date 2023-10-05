#include "ekf.h"
#include "cmatrix/core.h"

void init_filter(FilterCtx_TypeDef *ctx)
{
    // Initialize all of the sensor fields
    ctx->imu->acc_data[0] = 0.0;
    ctx->imu->acc_data[1] = 0.0;
    ctx->imu->acc_data[2] = 0.0;

    ctx->imu->gyro_data[0] = 0.0;
    ctx->imu->gyro_data[1] = 0.0;
    ctx->imu->gyro_data[2] = 0.0;

    ctx->imu->mag_data[0] = 0.0;
    ctx->imu->mag_data[1] = 0.0;
    ctx->imu->mag_data[2] = 0.0;
}

// Read IMU
void filter_runOnce(FilterCtx_TypeDef *ctx)
{
    Matrix2f mat = createMat2f(2,2);
    read_imu(ctx->imu);

    // Prediction

    // Update
}

Quaternion to_quaternion(EulerAngles angles)
{
    Quaternion q;

    double cr = cos(att.roll * 0.5);
    double sr = sin(att.roll * 0.5);
    double cp = cos(att.pitch * 0.5);
    double sp = sin(att.pitch * 0.5);
    double cy = cos(att.yaw * 0.5);
    double sy = sin(att.yaw * 0.5);

    q.q1 = cr * cp * cy + sr * sp * sy;
    q.q2 = sr * cp * cy - cr * sp * sy;
    q.q3 = cr * sp * cy + sr * cp * sy;
    q.z = cr * cp * sy - sr * sp * cy;

    return q;
}

void to_euler(Quaternion q)
{
    EulerAngles angles;
    // Roll
    double sinr_cosp = 2*((q.q1*q.q2) + (q.q3*q.q4));
    double cosr_cosp = 1 - 2*((q.q2*q.q2) + (q.q3*q.q3));

    angles.roll = atan2(sinr_cosp, cosr_cosp);

    // Pitch
    double sinp = sqrt(1 + 2*(q.q1*q.q3 - q.q3*q.q4));
    double cosp = sqrt(1 - 2*(q.q1*q.q3 - q.q2*q.q4));

    angles.pitch = 2*atan2(sinp, cosp) - M_PI_2;

    // yaw
    double siny_cosp = 2*(q.q1*q.q4 + q.q2*q.q3);
    double cosy_cosp = 1 - 2*(q.q3*q.q3 + q.q4*q.q4);

    angles.yaw = atan2(siny_cosp, cosy_cosp);

    return angles;
}
// Prediction

// Update
