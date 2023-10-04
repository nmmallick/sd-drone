/**
 * BNO_055_Driver.c
 */

#include "BNO_055_Drivers.h"


// IMU Initialization
void bno_055_init(IMU_TypeDef *imu_dtype)
{
    char debug[256] = {'\0'};
    HAL_StatusTypeDef ret;

    // Read chip id register
    uint8_t id[2] = { 0x00, 0x00};
    uint8_t buf[2];
    uint8_t opMode;

    ret = HAL_I2C_Mem_Read(imu_dtype->i2c, BNO_055_I2C_ADDR << 1, BNO_055_CHIP_ID_ADDR, I2C_MEMADD_SIZE_8BIT, id, 1, HAL_MAX_DELAY);
    if (ret != HAL_OK)
    {
	sprintf((char *)debug, "i2c mem read returned with code %d\n\r", ret);
	HAL_UART_Transmit(imu_dtype->huart, debug, sizeof(debug), HAL_MAX_DELAY);
    }

    if (id[0] != BNO_055_CHIP_ID)
    {
	sprintf((char*) debug, "chip id does not match %d\n\r", id[0]);
	HAL_UART_Transmit(imu_dtype->huart, debug, sizeof(debug), HAL_MAX_DELAY);
	return;
    }

    // Write power mode
    buf[0] = PWR_MODE_NORM;
    ret = HAL_I2C_Mem_Write(imu_dtype->i2c, BNO_055_I2C_ADDR << 1, PWR_MODE_REG, I2C_MEMADD_SIZE_8BIT, buf, 1, HAL_MAX_DELAY);
    if (ret != HAL_OK)
    {
	sprintf((char *)debug, "i2c mem write [power mode] returned with code %d\n\r");
	HAL_UART_Transmit(imu_dtype->huart, debug, sizeof(debug), HAL_MAX_DELAY);
    }
    HAL_Delay(50);

    // Write operating mode configuration
    buf[0] = OP_MODE_AMG;
    ret = HAL_I2C_Mem_Write(imu_dtype->i2c, BNO_055_I2C_ADDR << 1, OP_MODE_REG, I2C_MEMADD_SIZE_8BIT, buf, 1, HAL_MAX_DELAY);
    if (ret != HAL_OK)
    {
	sprintf((char *)debug, "i2c mem write [operating mode] returned with code %d\n\r", ret);
	HAL_UART_Transmit(imu_dtype->huart, debug, sizeof(debug), HAL_MAX_DELAY);
    }
    HAL_Delay(50);

    ret = HAL_I2C_Mem_Read(imu_dtype->i2c, BNO_055_I2C_ADDR << 1, OP_MODE_REG, I2C_MEMADD_SIZE_8BIT, &opMode, 1, HAL_MAX_DELAY);
    if (ret != HAL_OK)
    {
	sprintf((char *)debug, "read operating mode reg returned with code %d\n\r", ret);
	HAL_UART_Transmit(imu_dtype->huart, debug, sizeof(debug), HAL_MAX_DELAY);
    }
    HAL_Delay(50);

    if (opMode != OP_MODE_AMG)
    {
	sprintf((char *)debug, "operating mode does not match %d\n\r", opMode);
	HAL_UART_Transmit(imu_dtype->huart, debug, sizeof(debug), HAL_MAX_DELAY);
	exit(1);
    }
}

// Read gyro raw
void read_gyro(IMU_TypeDef *imu_dtype)
{
    HAL_StatusTypeDef ret;
    uint8_t buf[2];

    // x-axis
    ret = HAL_I2C_Mem_Read(imu_dtype->i2c, BNO_055_I2C_ADDR << 1, GYRO_REG_X_LSB, I2C_MEMADD_SIZE_8BIT, buf, 2, HAL_MAX_DELAY);
    if (ret == HAL_OK)
	imu_dtype->gyro_data[0] = convert(&buf, LSB_TO_DPS);

    // y-axis
    ret = HAL_I2C_Mem_Read(imu_dtype->i2c, BNO_055_I2C_ADDR << 1, GYRO_REG_Y_LSB, I2C_MEMADD_SIZE_8BIT, buf, 2, HAL_MAX_DELAY);
    if (ret == HAL_OK)
	imu_dtype->gyro_data[1] = convert(&buf, LSB_TO_DPS);

    // z-axis
    ret = HAL_I2C_Mem_Read(imu_dtype->i2c, BNO_055_I2C_ADDR << 1, GYRO_REG_Z_LSB, I2C_MEMADD_SIZE_8BIT, buf, 2, HAL_MAX_DELAY);
    if (ret == HAL_OK)
	imu_dtype->gyro_data[2] = convert(&buf, LSB_TO_DPS);
}

// Read accel raw
void read_acc(IMU_TypeDef *imu_dtype)
{
    HAL_StatusTypeDef ret;
    uint8_t buf[2];

    // x-axis
    ret = HAL_I2C_Mem_Read(imu_dtype->i2c, BNO_055_I2C_ADDR << 1, ACC_REG_X_LSB, I2C_MEMADD_SIZE_8BIT, buf, 2, HAL_MAX_DELAY);
    if (ret == HAL_OK)
	imu_dtype->acc_data[0] = convert(&buf, LSB_TO_G);

    // y-axis
    ret = HAL_I2C_Mem_Read(imu_dtype->i2c, BNO_055_I2C_ADDR << 1, ACC_REG_Y_LSB, I2C_MEMADD_SIZE_8BIT, buf, 2, HAL_MAX_DELAY);
    if (ret == HAL_OK)
	imu_dtype->acc_data[1] = convert(&buf, LSB_TO_G);

    // z-axis
    ret = HAL_I2C_Mem_Read(imu_dtype->i2c, BNO_055_I2C_ADDR << 1, ACC_REG_Z_LSB, I2C_MEMADD_SIZE_8BIT, buf, 2, HAL_MAX_DELAY);
    if (ret == HAL_OK)
	imu_dtype->acc_data[2] = convert(&buf, LSB_TO_G);

}

// Read mag raw
void read_mag(IMU_TypeDef *imu_dtype)
{
    HAL_StatusTypeDef ret;
    char buf[2];

    // x-axis
    ret = HAL_I2C_Mem_Read(imu_dtype->i2c, BNO_055_I2C_ADDR << 1, MAG_REG_X_LSB, I2C_MEMADD_SIZE_8BIT, buf, 2, HAL_MAX_DELAY);
    if (ret == HAL_OK)
    {
	char send[256] = {'\0'};
	int32_t data = (buf[1] << 8) | buf[2];

	sprintf((char*)send, "%d\n\r", data);

        HAL_UART_Transmit(imu_dtype->huart, send, sizeof(send), HAL_MAX_DELAY);
	imu_dtype->mag_data[0] = convert(&buf, LSB_TO_MICRO_T);
    }

    // y-axis
    ret = HAL_I2C_Mem_Read(imu_dtype->i2c, BNO_055_I2C_ADDR << 1, MAG_REG_Y_LSB, I2C_MEMADD_SIZE_8BIT, buf, 2, HAL_MAX_DELAY);
    if (ret == HAL_OK)
	imu_dtype->mag_data[1] = convert(&buf, LSB_TO_MICRO_T);

    //z-axis
    ret = HAL_I2C_Mem_Read(imu_dtype->i2c, BNO_055_I2C_ADDR << 1, MAG_REG_Z_LSB, I2C_MEMADD_SIZE_8BIT, buf, 2, HAL_MAX_DELAY);
    if (ret == HAL_OK)
	imu_dtype->mag_data[2] = convert(&buf, LSB_TO_MICRO_T);
}

void read_imu(IMU_TypeDef *imu_dtype)
{
    read_gyro(imu_dtype);
    read_acc(imu_dtype);
    read_mag(imu_dtype);
}

float convert(uint8_t *buf, float conv)
{
    int16_t val;
    float acc;

    val = (buf[1] << 8) | buf[0];

    if ((val & 0x80) == 0)
    {
	val = (~(val - 0x01));
	acc = val * -conv;
    } else
    {
	acc = val*conv;
    }

    return acc;
}
