#include "ekf.h"

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

    bno_055_init(ctx->imu);

    __filter__ = ctx;
}

// Read IMU
void filter_runOnce()
{
    char buf[256];

    // Read IMU data
    read_imu(__filter__->imu);
    /* sprintf((char *)buf, "acc: %f\t%f\t%f\n", */
    /* 	    __filter__->imu->acc_data[0], */
    /* 	    __filter__->imu->acc_data[1], */
    /* 	    __filter__->imu->acc_data[2]); */

    HAL_UART_Transmit(__filter__->huart, buf, strlen((char *)buf), HAL_MAX_DELAY);
}
