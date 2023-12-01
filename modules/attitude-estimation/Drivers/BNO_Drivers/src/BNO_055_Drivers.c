/**
 * BNO_055_Driver.c
 */

#include "BNO_055_Drivers.h"

IMU_TypeDef *imu_interface;
char debug[256] = {'\0'};

// IMU Initialization
void bno_055_init(IMU_TypeDef *imu_dtype)
{

    imu_interface = imu_dtype;
    set_mode(OP_MODE_CONFIG);

    HAL_StatusTypeDef ret;

    // Read chip id register
    uint8_t id[2] = { 0x00, 0x00};
    uint8_t buf[2];
    uint8_t op_mode;
    memset(&buf, 0x00, sizeof(buf));



    id[0] = read_reg(BNO_055_CHIP_ID_ADDR);
    if (id[0] != BNO_055_CHIP_ID)
    {
	sprintf((char*) debug, "chip id does not match %d\n\r", id[0]);
	HAL_UART_Transmit(imu_dtype->huart, debug, sizeof(debug), HAL_MAX_DELAY);
	return;
    }

    // Write power mode
    buf[0] = PWR_MODE_NORM;
    ret = HAL_I2C_Mem_Write(imu_dtype->i2c, BNO_055_I2C_ADDR << 1, PWR_MODE_REG, I2C_MEMADD_SIZE_8BIT, buf, 1, HAL_MAX_DELAY);
    while (ret != HAL_OK)
    {
	ret = HAL_I2C_Mem_Write(imu_dtype->i2c, BNO_055_I2C_ADDR << 1, PWR_MODE_REG, I2C_MEMADD_SIZE_8BIT, buf, 1, HAL_MAX_DELAY);
	sprintf((char *)debug, "i2c mem write [power mode] returned with code %d\n\r");
	HAL_UART_Transmit(imu_dtype->huart, debug, sizeof(debug), HAL_MAX_DELAY);
    }
    HAL_Delay(50);

    // Remapping axis to (x, z, y

    // 0x 00 01 10 00
    ret = write_byte(AXIS_MAP_CONFIG, 0x09);
    if (ret != HAL_OK)
    {
    	sprintf((char *)debug, "failed to remap axes\n\r");
    	HAL_UART_Transmit(imu_dtype->huart, debug, sizeof(debug), HAL_MAX_DELAY);
    	exit(1);
    }

    // Negate x,y,z
    ret = write_byte(AXIS_MAP_SIGN, 0x05);
    if (ret != HAL_OK)
    {
    	sprintf((char *)debug, "failed to remap axes sign\n\r");
    	HAL_UART_Transmit(imu_dtype->huart, debug, sizeof(debug), HAL_MAX_DELAY);
    	exit(1);
    }

    // Write operating mode configuration
    set_mode(OP_MODE_NDOF);
    op_mode = read_reg(OP_MODE_REG);
    HAL_Delay(50);

    if (op_mode != OP_MODE_NDOF)
    {
	sprintf((char *)debug, "operating mode does not match %d\n\r", op_mode);
	HAL_UART_Transmit(imu_dtype->huart, debug, sizeof(debug), HAL_MAX_DELAY);
	exit(1);
    }
}

void set_mode(uint8_t mode)
{
    int ret = write_byte(OP_MODE_REG, mode);
    while (ret != HAL_OK)
    {
	ret = write_byte(OP_MODE_REG, mode);
	sprintf((char *)debug, "i2c mem write [operating mode] returned with code %d\n\r", ret);
	HAL_UART_Transmit(imu_interface->huart, debug, sizeof(debug), HAL_MAX_DELAY);
    }
     HAL_Delay(50);
}

uint8_t read_reg(uint8_t reg_addr)
{
    uint8_t buf;
    int ret = HAL_I2C_Mem_Read(imu_interface->i2c, BNO_055_I2C_ADDR << 1, reg_addr, I2C_MEMADD_SIZE_8BIT, &buf, 1, HAL_MAX_DELAY);
    if (ret != HAL_OK)
    {
	sprintf((char *)debug, "read operating mode reg returned with code %d\n\r", ret);
	HAL_UART_Transmit(imu_interface->huart, debug, sizeof(debug), HAL_MAX_DELAY);
    }

    return buf;
}

HAL_StatusTypeDef write_byte(uint8_t reg_addr, uint8_t data)
{
    return HAL_I2C_Mem_Write(imu_interface->i2c, BNO_055_I2C_ADDR << 1, reg_addr, I2C_MEMADD_SIZE_8BIT, &data, 1, HAL_MAX_DELAY);
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
	imu_dtype->mag_data[0] = convert(&buf, LSB_TO_MICRO_T);

    // y-axis
    ret = HAL_I2C_Mem_Read(imu_dtype->i2c, BNO_055_I2C_ADDR << 1, MAG_REG_Y_LSB, I2C_MEMADD_SIZE_8BIT, buf, 2, HAL_MAX_DELAY);
    if (ret == HAL_OK)
	imu_dtype->mag_data[1] = convert(&buf, LSB_TO_MICRO_T);

    //z-axis
    ret = HAL_I2C_Mem_Read(imu_dtype->i2c, BNO_055_I2C_ADDR << 1, MAG_REG_Z_LSB, I2C_MEMADD_SIZE_8BIT, buf, 2, HAL_MAX_DELAY);
    if (ret == HAL_OK)
	imu_dtype->mag_data[2] = convert(&buf, LSB_TO_MICRO_T);
}

void read_quat(IMU_TypeDef *imu_dtype)
{
    HAL_StatusTypeDef ret;

    uint8_t buf[2];

    ret = HAL_I2C_Mem_Read(imu_dtype->i2c, BNO_055_I2C_ADDR << 1, QUAT_REG_W_LSB, I2C_MEMADD_SIZE_8BIT, buf, 2, HAL_MAX_DELAY);
    if (ret == HAL_OK)
	imu_dtype->q[0] = convert(&buf, LSB_TO_QUAT);

    ret = HAL_I2C_Mem_Read(imu_dtype->i2c, BNO_055_I2C_ADDR << 1, QUAT_REG_X_LSB, I2C_MEMADD_SIZE_8BIT, buf, 2, HAL_MAX_DELAY);
    if (ret == HAL_OK)
	imu_dtype->q[1] = convert(&buf, LSB_TO_QUAT);

    ret = HAL_I2C_Mem_Read(imu_dtype->i2c, BNO_055_I2C_ADDR << 1, QUAT_REG_Y_LSB, I2C_MEMADD_SIZE_8BIT, buf, 2, HAL_MAX_DELAY);
    if (ret == HAL_OK)
	imu_dtype->q[2] = convert(&buf, LSB_TO_QUAT);

    ret = HAL_I2C_Mem_Read(imu_dtype->i2c, BNO_055_I2C_ADDR << 1, QUAT_REG_Z_LSB, I2C_MEMADD_SIZE_8BIT, buf, 2, HAL_MAX_DELAY);
    if (ret == HAL_OK)
	imu_dtype->q[3] = convert(&buf, LSB_TO_QUAT);
}

void read_euler(IMU_TypeDef *imu_dtype)
{
    HAL_StatusTypeDef ret;

    uint8_t buf[2];

    ret = HAL_I2C_Mem_Read(imu_dtype->i2c, BNO_055_I2C_ADDR << 1, EUL_REG_X_LSB, I2C_MEMADD_SIZE_8BIT, buf, 2, HAL_MAX_DELAY);
    if (ret == HAL_OK)
	imu_dtype->euler[0] = convert(&buf, LSB_TO_DEG);

    ret = HAL_I2C_Mem_Read(imu_dtype->i2c, BNO_055_I2C_ADDR << 1, EUL_REG_Y_LSB, I2C_MEMADD_SIZE_8BIT, buf, 2, HAL_MAX_DELAY);
    if (ret == HAL_OK)
	imu_dtype->euler[1] = convert(&buf, LSB_TO_DEG);

    ret = HAL_I2C_Mem_Read(imu_dtype->i2c, BNO_055_I2C_ADDR << 1, EUL_REG_Z_LSB, I2C_MEMADD_SIZE_8BIT, buf, 2, HAL_MAX_DELAY);
    if (ret == HAL_OK)
	imu_dtype->euler[2] = convert(&buf, LSB_TO_DEG);
}

void read_imu(IMU_TypeDef *imu_dtype)
{
    read_gyro(imu_dtype);
    read_acc(imu_dtype);
    read_mag(imu_dtype);
    read_quat(imu_dtype);
    read_euler(imu_dtype);
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
