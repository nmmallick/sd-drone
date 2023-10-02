/**
 * BNO_055_Driver.c
 */

#include "BNO_055_Drivers.h"


// IMU Initialization
void bno_055_init(IMU_TypeDef *imu_dtype)
{
    // Write configuration

    HAL_StatusTypeDef ret;
    ret = HAL_I2C_Mem_Write(imu_dtype->i2c, BNO_055_I2C_ADDR, OP_MODE_REG, I2C_MEMADD_SIZE_8BIT, OP_MODE_AMG, 1, HAL_MAX_DELAY);
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

// Read accel raw
void read_acc(IMU_TypeDef *imu_dtype)
{
    HAL_StatusTypeDef ret;

    char buf[2];

    // x-axis
    ret = HAL_I2C_Mem_Read(imu_dtype->i2c, BNO_055_I2C_ADDR, ACC_REG_X_LSB, I2C_MEMADD_SIZE_8BIT, buf, 2, HAL_MAX_DELAY);
    if (ret == HAL_OK)
	imu_dtype->acc_data[0] = ((buf[1] << 8) | buf[0])*LSB_TO_M_PER_S;

    // y-axis
    ret = HAL_I2C_Mem_Read(imu_dtype->i2c, BNO_055_I2C_ADDR, ACC_REG_Y_LSB, I2C_MEMADD_SIZE_8BIT, buf, 2, HAL_MAX_DELAY);
    if (ret == HAL_OK)
	imu_dtype->acc_data[1] = ((buf[1] << 8) | buf[0])*LSB_TO_M_PER_S;

    // z-axis
    ret = HAL_I2C_Mem_Read(imu_dtype->i2c, BNO_055_I2C_ADDR, ACC_REG_Z_LSB, I2C_MEMADD_SIZE_8BIT, buf, 2, HAL_MAX_DELAY);
    if (ret == HAL_OK)
	imu_dtype->acc_data[2] = ((buf[1] << 8) | buf[0])*LSB_TO_M_PER_S;
}

// Read mag raw
void read_mag(IMU_TypeDef *imu_dtype)
{
    HAL_StatusTypeDef ret;
    char buf[2];

    // x-axis
    ret = HAL_I2C_Mem_Read(imu_dtype->i2c, BNO_055_I2C_ADDR, MAG_REG_X_LSB, I2C_MEMADD_SIZE_8BIT, buf, 2, HAL_MAX_DELAY);
    if (ret == HAL_OK)
	imu_dtype->mag_data[0] = ((buf[1] << 8) | buf[0])*LSB_TO_MICRO_T;

    // y-axis
    ret = HAL_I2C_Mem_Read(imu_dtype->i2c, BNO_055_I2C_ADDR, MAG_REG_Y_LSB, I2C_MEMADD_SIZE_8BIT, buf, 2, HAL_MAX_DELAY);
    if (ret == HAL_OK)
	imu_dtype->mag_data[1] = ((buf[1] << 8) | buf[0])*LSB_TO_MICRO_T;

    //z-axis
    ret = HAL_I2C_Mem_Read(imu_dtype->i2c, BNO_055_I2C_ADDR, MAG_REG_Z_LSB, I2C_MEMADD_SIZE_8BIT, buf, 2, HAL_MAX_DELAY);
    if (ret == HAL_OK)
	imu_dtype->mag_data[2] = ((buf[1] << 8) | buf[0])*LSB_TO_MICRO_T;
}

void read_imu(IMU_TypeDef *imu_dtype)
{
    read_gyro(imu_dtype);
    read_acc(imu_dtype);
    read_mag(imu_dtype);
}
