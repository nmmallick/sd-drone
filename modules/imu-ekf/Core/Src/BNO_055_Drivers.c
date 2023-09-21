/**
 * BNO_055_Driver.c
 */

#include "BNO_055_Drivers.h"


// IMU Initialization
void BNO_055_init()
{
    // Write configuration
}

// Read gyro raw
void read_gyro(IMU_TypeDef *imu_dtype)
{
    // HAL return typedef
    HAL_StatusTypeDef ret;
    uint8_t buf[2];

    // x-axis
    ret = HAL_I2C_Mem_Read(imu_dtype->i2c, BNO_055_I2C_ADDR, GYRO_REG_X_LSB, I2C_MEMADD_SIZE_8BIT, buf, 2, HAL_MAX_DELAY);
    if (ret == HAL_OK)
	imu_dtype->gyro_data[0] = ((buf[1] << 8) |  buf[0])*LSB_TO_DPS;

    // y-axis
    ret = HAL_I2C_Mem_Read(imu_dtype->i2c, BNO_055_I2C_ADDR, GYRO_REG_Y_LSB, I2C_MEMADD_SIZE_8BIT, buf, 2, HAL_MAX_DELAY);
    if (ret == HAL_OK)
	imu_dtype->gyro_data[1] = ((buf[1] << 8) | buf[0])*LSB_TO_DPS;

    // z-axis
    ret = HAL_I2C_Mem_Read(imu_dtype->i2c, BNO_055_I2C_ADDR, GYRO_REG_Z_LSB, I2C_MEMADD_SIZE_8BIT, buf, 2, HAL_MAX_DELAY);
    if (ret == HAL_OK)
	imu_dtype->gyro_data[2] = ((buf[1] << 8) | buf[0])*LSB_TO_DPS;
}

float convert_buf(char *buf, float conv)
{

}
// Read accel raw

// Read mag raw
