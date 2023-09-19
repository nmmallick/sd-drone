#ifndef BNO_055_H
#define BNO_055_H

#include "stm32f4xlx_hal.h"
#include <math.h>

typedef struct IMU_TypeDef
{
    float acc_data[3];
    float gyro_data[3];
    float mag_data[3];

    float dt;

    I2C_HandleTypdef *i2d;
} IMU_TypeDef;


#define OP_MODE_REG 0x3d
typedef enum
{
    OP_MODE_CONFIG = 0x00,
    OP_MODE_ACCONLY = 0x01,
    OP_MODE_MAGONLY = 0x02,
    OP_MODE_GYROONLY = 0x03,
    OP_MODE_ACCMAG = 0x04,
    OP_MODE_ACCGYRO = 0x05,
    OP_MODE_MAGGYRO = 0x06,
    OP_MODE_AMG = 0x07,
    OP_MODE_IMU = 0x08,
    OP_MODE_COMPASS = 0x09,
    OP_MODE_M4G = 0x0a,
    OP_MODE_NDOF_FMC_OFF = 0x0b,
    OP_MODE_NDOF = 0x0c
} bno_op_mode;

/**
 * @brief Initialize IMU
 */
void BNO_055_init(bno_op_mode mode);

#endif
