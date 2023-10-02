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

FilterCtx_TypeDef *__filter__;

// Initialize
void init_filter(FilterCtx_TypeDef *dtype);

// Read IMU
void filter_runOnce();

#endif
