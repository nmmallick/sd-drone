#ifndef EKF_H_
#define EKF_H_

#include "stm32l4xx_hal.h"
#include "BNO_055_Drivers.h"
#include "arm_math.h"

typedef IMU_TypeDef IMU;
typedef UART_HandleTypeDef UART;

typedef arm_matrix_instance_f32 Matf32;
typedef float32_t f32;

typedef struct Quaternion
{
    f32  q0, q1, q2, q3;
} Quaternion;

typedef struct EulerAngles
{
    f32 roll, pitch, yaw;
} EulerAngles;

typedef struct State
{
    Quaternion quat;
    EulerAngles bias;
} State;

typedef struct Prediction
{
    f32 x_priori[7];
    f32 p_priori[7][7];
} Prediction;

typedef struct FilterCtx_TypeDef
{
    IMU *imu;

    // Uart handler for debugging/serial communication
    UART *uart;

    State x;
} FilterCtx_TypeDef;

static f32 P[7][7];
Matf32 P_mat;

static f32 Q[7][7];
Matf32 Q_mat;

static f32 R[6][6];
Matf32 R_mat;

static f32 accRef[3] = { 0.0, 0.0, -1.0 };
static f32 magRef[3] = { 0.0, -1.0, 0.0 };

// Initialize
void init_filter(FilterCtx_TypeDef *);

// Read IMU
void filter_runOnce(FilterCtx_TypeDef *, float dt);


Prediction predict(FilterCtx_TypeDef *ctx, float dt);
void update(FilterCtx_TypeDef *ctx, Prediction *p);

void mag_vector(Quaternion *q, f32 *mag);
void normalize(f32 *vec, size_t size);
void identity(f32 *mat, size_t rows, size_t cols);
void scale(f32 *mat, f32 scale, size_t rows, size_t cols);

void send_debug();
void print_mat(f32 *mat, size_t rows, size_t cols);

Quaternion to_quaternion(EulerAngles angles);
EulerAngles to_euler(Quaternion q);

#endif
