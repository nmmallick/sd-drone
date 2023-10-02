 #ifndef BNO_055_H
#define BNO_055_H

#include "stm32f1xx_hal.h"
#include <math.h>

typedef struct IMU_TypeDef
{
    float acc_data[3];
    float gyro_data[3];
    float mag_data[3];

    I2C_HandleTypeDef *i2c;
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

#define GYRO_REG_X_LSB 0x14
#define GYRO_REG_X_MSB 0x15
#define GYRO_REG_Y_LSB 0x16
#define GYRO_REG_Y_MSB 0x17
#define GYRO_REG_Z_LSB 0x18
#define GYRO_REG_Z_MSB 0x19

#define ACC_REG_X_LSB 0x08
#define ACC_REG_X_MSB 0x09
#define ACC_REG_Y_LSB 0x0a
#define ACC_REG_Y_MSB 0x0b
#define ACC_REG_Z_LSB 0x0c
#define ACC_REG_Z_MSB 0x0d

#define MAG_REG_X_LSB 0x0e
#define MAG_REG_X_MSB 0x0f
#define MAG_REG_Y_LSB 0x10
#define MAG_REG_Y_MSB 0x11
#define MAG_REG_Z_LSB 0x12
#define MAG_REG_Z_MSB 0x13

#define BNO_055_I2C_ADDR 0x29

// Conversion for gyroscope
#define LSB_TO_DPS 16.0
#define LSB_TO_RPS 900.0

// Conversions for accelerometer
#define LSB_TO_M_PER_S 100.0
#define LSB_TO_G 1/1000.0

// Conversions for magnometer
#define LSB_TO_MICRO_T 16.0

/**
 * @brief Initialize IMU
 */
void bno_055_init(IMU_TypeDef *imu_dtype);

void read_gyro(IMU_TypeDef *imu_dtype);
void read_acc(IMU_TypeDef *imu_dtype);
void read_mag(IMU_TypeDef *imu_dtype);

void read_imu(IMU_TypeDef *imu_dtype);

#endif
