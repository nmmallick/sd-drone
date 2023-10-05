#ifndef EKF_H_
#define EKF_H_

#include "stm32f1xx_hal.h"
#include "BNO_055_Drivers.h"

typedef struct FilterCtx_TypeDef
{
    IMU_TypeDef *imu;

    // Uart handler for debugging/serial communication
    UART_HandleTypeDef *huart;

    // TODO: Add kalman filter context
} FilterCtx_TypeDef;

typedef struct Quaternion
{
    float q1, q2, q3, q4;
} Quaternion;

typedef struct EulerAngles
{
    float roll, pitch, yaw;
} EulerAngles;

// Initialize
void init_filter(FilterCtx_TypeDef *);

// Read IMU
void filter_run(FilterCtx_TypeDef *);

Quaternion to_quaternion(EulerAngles angles);

EulerAngles to_euler(Quaternion quat);

#endif
