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
}

// Read IMU
void filter_runOnce(FilterCtx_TypeDef *ctx)
{
    char buf[256];

    read_imu(ctx->imu);
    /* sprintf((char *)buf, "ACC: %.2f %.2f %.2f\n\r", */
    /* 	    ctx->imu->acc_data[0], */
    /* 	    ctx->imu->acc_data[1], */
    /* 	    ctx->imu->acc_data[2]); */
    /* HAL_UART_Transmit(ctx->huart, buf, strlen((char *)buf), HAL_MAX_DELAY); */
    /* sprintf((char *)buf, "GYRO: %.2f %.2f %.2f\n\n\r", */
    /* 	    ctx->imu->gyro_data[0], */
    /* 	    ctx->imu->gyro_data[1], */
    /* 	    ctx->imu->gyro_data[2]); */
    /* HAL_UART_Transmit(ctx->huart, buf, strlen((char *)buf), HAL_MAX_DELAY); */
    sprintf((char *)buf, "MAG: %.5f %.5f %.5f\n\n\r",
    	    ctx->imu->mag_data[0],
    	    ctx->imu->mag_data[1],
    	    ctx->imu->mag_data[2]);
    HAL_UART_Transmit(ctx->huart, buf, strlen((char *)buf), HAL_MAX_DELAY);
}

