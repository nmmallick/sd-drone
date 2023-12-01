
#ifndef BNO_055_H_
#define BNO_055_H_

#include "stm32l4xx_hal.h"
#include <math.h>

typedef float f32;

typedef struct IMU_TypeDef
{
    f32 acc_data[3];
    f32 gyro_data[3];
    f32 mag_data[3];

    f32 q[4];

    f32 euler[3];

    I2C_HandleTypeDef *i2c;
    UART_HandleTypeDef *huart;
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

#define PWR_MODE_REG 0x3e
typedef enum
{
    PWR_MODE_NORM = 0x00,
    PWR_MODE_LOW = 0x01,
    PWR_MODE_SUS = 0x02,
    PWR_MODE_INVALID = 0x03
} bno_pwr_mode;

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

#define QUAT_REG_W_LSB 0x20
#define QUAT_REG_W_MSB 0x21
#define QUAT_REG_X_LSB 0x22
#define QUAT_REG_X_MSB 0x23
#define QUAT_REG_Y_LSB 0x24
#define QUAT_REG_Y_MSB 0x25
#define QUAT_REG_Z_LSB 0x26
#define QUAT_REG_Z_MSB 0x27

#define EUL_REG_X_LSB 0x1a
#define EUL_REG_X_MSB 0x1b
#define EUL_REG_Y_LSB 0x1c
#define EUL_REG_Y_MSB 0x1d
#define EUL_REG_Z_LSB 0x1e
#define EUL_REG_Z_MSB 0x1f

#define BNO_055_I2C_ADDR 0x28
#define BNO_055_CHIP_ID 0xa0
#define BNO_055_CHIP_ID_ADDR 0x00

#define AXIS_MAP_CONFIG 0x41
#define AXIS_MAP_SIGN 0x42

#define X_AXIS 0x00
#define Y_AXIS 0x01
#define Z_AXIS 0x02

// Conversion for gyroscope
#define LSB_TO_DPS 16.0
#define LSB_TO_RPS 900.0

// Conversions for accelerometer
#define LSB_TO_M_PER_S 100.0
#define LSB_TO_G 1/1000.0

// Conversions for magnometer
#define LSB_TO_MICRO_T 16.0

// Conversion for euler angles [RADIANS]
#define LSB_TO_DEG 1/16.0
#define LSB_TO_RAD 1/900.0

// Conversion for quaternions
#define LSB_TO_QUAT 1.0/(1 << 14)

/**
 * @brief Initialize IMU
 */
void bno_055_init(IMU_TypeDef *imu_dtype);

void read_gyro(IMU_TypeDef *imu_dtype);
void read_acc(IMU_TypeDef *imu_dtype);
void read_mag(IMU_TypeDef *imu_dtype);
void read_quat(IMU_TypeDef *imu_dtype);
void read_euler(IMU_TypeDef *imu_dtype);

void read_imu(IMU_TypeDef *imu_dtype);

float convert(uint8_t *, float);

// Helpers
void set_mode(uint8_t mode);
uint8_t read_reg(uint8_t reg_addr);
HAL_StatusTypeDef write_byte(uint8_t reg_addr, uint8_t data);

#endif
