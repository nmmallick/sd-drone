#include "ekf.h"

static char debug[256] = {'\0'};
static const UART *huart;
static size_t iter = 0;
void send_debug()
{
    sprintf((char*) debug, "debugging - 1\n\r");
    HAL_UART_Transmit(huart, debug, sizeof(debug), HAL_MAX_DELAY);
}

// Set a matrix to be identity
void identity(f32 *mat, size_t rows, size_t cols)
{
    for (size_t i = 0; i < rows; i++)
	for (size_t j = 0; j < cols; j++)
	    if (i == j)
		*(mat + i*cols + j) = 1.0;
	    else
		*(mat + i*cols + j) = 0.0;
}

void scale(f32 *mat, f32 scale, size_t rows, size_t cols)
{
    for (size_t i = 0; i < rows; i++)
	for (size_t j = 0; j < cols; j++)
	    *(mat + i*cols + j) *= scale;
}

void normalize(f32 *vec, size_t size)
{
    f32 mag = 0;

    for (size_t i = 0; i < size; i++)
	mag += pow(vec[i], 2);

    mag = sqrt(mag);

    for (size_t i = 0; i < size; i++)
	vec[i] /= mag;
}

void print_mat(f32 *mat, size_t rows, size_t cols)
{
    char buf[512] = {'\0'};
    size_t offset = 0;
    offset += sprintf((char*)(buf + offset), "---------------\n\r");
    for (size_t i = 0; i < rows; i++)
    {
	for (size_t j = 0; j < cols; j++)
	{
	    offset += sprintf((char *)(buf + offset), "%.4f ", *(mat + i*cols + j));
	}

	offset += sprintf((char *)(buf + offset), "\n\r");
    }
    offset += sprintf((char*)(buf + offset), "---------------\n\r");
    HAL_UART_Transmit(huart, buf, sizeof(buf), HAL_MAX_DELAY);
}

void init_filter(FilterCtx_TypeDef *ctx)
{
    arm_status status;
    huart = ctx->uart;

    // Initialize error matrix P
    arm_mat_init_f32(&P_mat, 7, 7, P);
    identity(&P, 7, 7);
    scale(&P, 0.01, 7, 7);

    // Initialize covariance matrix Q
    arm_mat_init_f32(&Q_mat, 7, 7, Q);
    identity(&Q, 7, 7);
    scale(&Q, 0.001, 7, 7);

    // Initialize R
    arm_mat_init_f32(&R_mat, 6, 6, R);
    identity(&R, 6, 6);
    scale(&R, 0.1, 6, 6);

    // Initialize Quaternion and Biases
    ctx->x.quat.q0 = 1.0;
    ctx->x.quat.q1 = 0.0;
    ctx->x.quat.q2 = 0.0;
    ctx->x.quat.q3 = 0.0;

    ctx->x.bias.roll = 0.0;
    ctx->x.bias.pitch = 0.0;
    ctx->x.bias.yaw = 0.0;
}

// Read IMU
void filter_runOnce(FilterCtx_TypeDef *ctx, float dt)
{
    uint8_t buf[16] = {'\0'};

    read_imu(ctx->imu);

    ctx->x.quat.q0 = ctx->imu->q[0];
    ctx->x.quat.q1 = ctx->imu->q[1];
    ctx->x.quat.q2 = ctx->imu->q[2];
    ctx->x.quat.q3 = ctx->imu->q[3];

    EulerAngles att = to_euler(ctx->x.quat);

    /* sprintf((char *)debug, "%f %f %f %f\n\n\r", */
    /* 	    ctx->x.quat.q0, ctx->x.quat.q1, ctx->x.quat.q2, ctx->x.quat.q3); */

    memcpy((void *)&buf, (void *)&(ctx->x.quat), 16);

    /* sprintf((char *)debug, "%.4f %.4f %.4f\n\n\r", */
    /* 	    att.roll*(180.0/M_PI), att.pitch*(180.0/M_PI), att.yaw*(180.0/M_PI)); */
    /* HAL_UART_Transmit(huart, debug, sizeof(debug), HAL_MAX_DELAY); */
    HAL_UART_Transmit(huart, &buf, sizeof(buf), HAL_MAX_DELAY);
}

Prediction predict(FilterCtx_TypeDef *ctx, f32 dt)
{
    // Return values
    Prediction p;
    arm_status status;

    State *x_prev = &ctx->x;
    Quaternion *q = &x_prev->quat;
    IMU *imu = ctx->imu;
    f32 t_2 = dt/2.0;
    // (1k) //
    // Create A and Sq matrix
    f32 sq[4][3] = { { -(q->q1), -(q->q2), -(q->q3) },
		     { (q->q0), -(q->q3), -(q->q2) },
		     { (q->q3), (q->q0), -(q->q1) },
		     { -(q->q2), (q->q1), (q->q0) } };

    f32 A[7][7] = {{ 1, 0, 0, 0, -t_2*sq[0][0], -t_2*sq[0][1], -t_2*sq[0][2] },
		   { 0, 1, 0, 0, -t_2*sq[1][0], -t_2*sq[1][1], -t_2*sq[1][2] },
		   { 0, 0, 1, 0, -t_2*sq[2][0], -t_2*sq[2][1], -t_2*sq[2][2] },
		   { 0, 0, 0, 1, -t_2*sq[3][0], -t_2*sq[3][1], -t_2*sq[3][2] },
		   { 0, 0, 0, 0, 1, 0, 0 },
		   { 0, 0, 0, 0, 0, 1, 0 },
		   { 0, 0, 0, 0, 0, 0, 1 }};

    f32 x[7] = { q->q0, q->q1, q->q2, q->q3,
		 x_prev->bias.roll, x_prev->bias.pitch, x_prev->bias.yaw };

    f32 B[7][3] = { {t_2*sq[0][0], t_2*sq[0][1], t_2*sq[0][2] },
		    {t_2*sq[1][0], t_2*sq[1][1], t_2*sq[1][2] },
		    {t_2*sq[2][0], t_2*sq[2][1], t_2*sq[2][2] },
		    {t_2*sq[3][0], t_2*sq[3][1], t_2*sq[3][2] },
		    { 0, 0, 0 },
		    { 0, 0, 0 },
		    { 0, 0, 0 } };

    f32 u[3] = { imu->gyro_data[0]*M_PI/180.0, imu->gyro_data[1]*M_PI/180.0, imu->gyro_data[2]*M_PI/180.0 };

    // Intermediate computations
    f32 Ax[7];
    f32 Bu[7];

    // Result
    Matf32 x_priori_mat, p_priori_mat;

    // Parameters
    Matf32 A_mat,
	B_mat,
	x_vec,
	u_vec;

    // Intermediate calculations
    Matf32 Ax_mat,
	Bu_mat;

    // Initialize for matrix math
    arm_mat_init_f32(&A_mat, 7, 7, A);
    arm_mat_init_f32(&x_vec, 7, 1, x);

    arm_mat_init_f32(&B_mat, 7, 3, B);
    arm_mat_init_f32(&u_vec, 3, 1, u);

    arm_mat_init_f32(&Ax_mat, 7, 1, Ax);
    arm_mat_init_f32(&Bu_mat, 7, 1, Bu);

    arm_mat_init_f32(&x_priori_mat, 7, 1, p.x_priori);
    arm_mat_init_f32(&p_priori_mat, 7, 1, p.p_priori);

    // TODO: Do something with these status returns
    status = arm_mat_mult_f32(&A_mat, &x_vec, &Ax_mat);
    status = arm_mat_mult_f32(&B_mat, &u_vec, &Bu_mat);
    status = arm_mat_add_f32(&Ax_mat, &Bu_mat, &x_priori_mat);

    // Normalize the first four elements of predicted state (the quaternion)
    normalize(&p.x_priori, 4);

    // (2k) //
    f32 AT[7][7];
    f32 AP[7][7];
    f32 APAT[7][7];

    Matf32 APAT_mat, AT_mat, AP_mat;

    arm_mat_init_f32(&APAT_mat, 7, 7, APAT);
    arm_mat_init_f32(&AT_mat, 7, 7, AT);
    arm_mat_init_f32(&AP_mat, 7, 7, AP);

    status = arm_mat_trans_f32(&A_mat, &AT_mat);
    status = arm_mat_mult_f32(&A_mat, &P_mat, &AP_mat);
    status = arm_mat_mult_f32(&AP_mat, &AT_mat, &APAT_mat);
    status = arm_mat_add_f32(&APAT_mat, &Q_mat, &p_priori_mat);

    return p;
}

void update(FilterCtx_TypeDef *ctx, Prediction *p)
{

    arm_status status;
    Matf32 P_PRIORI_mat, X_PRIORI_mat;
    arm_mat_init_f32(&P_PRIORI_mat, 7, 7, p->p_priori);

    Quaternion *q = &ctx->x.quat;
    IMU *imu = ctx->imu;
    // Construct the C matrix
    f32 Ca[3][4] = { { -q->q2, q->q3, -q->q0, q->q1 },
		     { q->q1, q->q0, q->q3, q->q2 },
		     { q->q0, -q->q1, -q->q2, q->q3} };

    f32 Cb[3][4] = { { q->q3, q->q2, q->q1, q->q0 },
		     { q->q0, -q->q1, -q->q2, q->q3 },
		     { -q->q1, -q->q0, q->q3, q->q2 } };

    f32 C[6][7] = { { -2*Ca[0][0], -2*Ca[0][1], -2*Ca[0][2], -2*Ca[0][3], 0, 0, 0},
		    { -2*Ca[1][0], -2*Ca[1][1], -2*Ca[1][2], -2*Ca[1][3], 0, 0, 0},
		    { -2*Ca[2][0], -2*Ca[2][1], -2*Ca[2][2], -2*Ca[2][3], 0, 0, 0},
		    { -2*Cb[0][0], -2*Cb[0][1], -2*Cb[0][2], -2*Cb[0][3], 0, 0, 0},
		    { -2*Cb[1][0], -2*Cb[1][1], -2*Cb[1][2], -2*Cb[1][3], 0, 0, 0},
		    { -2*Cb[2][0], -2*Cb[2][1], -2*Cb[2][2], -2*Cb[2][3], 0, 0, 0} };

    f32 CT[7][6];
    f32 PCT[7][6];
    f32 CPCT[6][6];
    f32 CPCTR[6][6];
    f32 CPCTR_INV[6][6];
    f32 K[7][6];

    Matf32 C_mat, CT_mat,
	PCT_mat, CPCT_mat, CPCTR_mat,
	CPCTR_INV_mat, K_mat;

    arm_mat_init_f32(&C_mat, 6, 7, C);
    arm_mat_init_f32(&CT_mat, 7, 6, CT);
    arm_mat_init_f32(&PCT_mat, 7, 6, PCT);
    arm_mat_init_f32(&CPCT_mat, 6, 6, CPCT);
    arm_mat_init_f32(&CPCTR_mat, 6, 6, CPCTR);
    arm_mat_init_f32(&CPCTR_INV_mat, 6, 6, CPCTR_INV);
    arm_mat_init_f32(&K_mat, 7, 6, K);

    // create C transpose
    status = arm_mat_trans_f32(&C_mat, &CT_mat);

    // compute PCT
    status = arm_mat_mult_f32(&P_PRIORI_mat, &CT_mat, &PCT_mat);

    // compute CPCT
    status = arm_mat_mult_f32(&C_mat, &PCT_mat, &CPCT_mat);

    // compute CPCTR
    status = arm_mat_add_f32(&CPCT_mat, &R_mat, &CPCTR_mat);

    // compute CPCTR^-1
    status = arm_mat_inverse_f32(&CPCTR_mat, &CPCTR_INV_mat);
    // TODO: check for singularity

    // compute K = (PCT * CPCTR^-1)
    status = arm_mat_mult_f32(&PCT_mat, &CPCTR_INV_mat, &K_mat);

    f32 y_priori[6];
    f32 k_act_y[6];

    f32 R[9];
    f32 RT[9];
    Matf32 R_mat, RT_mat;

    arm_mat_init_f32(&R_mat, 3, 3, R);
    arm_mat_init_f32(&RT_mat, 3, 3, RT);

    arm_quaternion2rotation_f32((f32*)q, &R, 1);
    arm_mat_trans_f32(&R_mat, &RT_mat);

    f32 y[6] = { imu->acc_data[0], imu->acc_data[1], imu->acc_data[2],
		 imu->mag_data[0], imu->mag_data[1], imu->mag_data[2] };

    normalize(&y, 3);
    mag_vector(&ctx->x.quat, &y[3]);

    //arm_mat_vec_mult_f32(&C_mat, &p->x_priori, &y_priori);

    // Subtract y_priori from y
    for (size_t i = 0; i < 6; i++)
	y[i] -= y_priori[i];

    arm_mat_vec_mult_f32(&K_mat, &y, &k_act_y);

    f32 x[7];
    for (size_t i = 0; i < 7; i++)
	x[i] = p->x_priori[i] + k_act_y[i];

    normalize(&x, 4);
    memcpy(&ctx->x, &x, sizeof(x));

    // Update error
    f32 I[7][7];
    Matf32 I_mat;
    identity(&I, 7, 7);

    f32 KC[7][7];
    Matf32 KC_mat;

    arm_mat_init_f32(&I_mat, 7, 7, I);
    arm_mat_init_f32(&KC_mat, 7, 7, KC);

    status = arm_mat_mult_f32(&K_mat, &C_mat, &KC_mat);

    // set P
    status = arm_mat_sub_f32(&I_mat, &KC_mat, &P_mat);
}

void mag_vector(Quaternion *q, f32 *mag)
{
    f32 R[9];
    f32 RT[9];
    f32 mag_n[3];
    Matf32 R_mat, RT_mat;

    arm_mat_init_f32(&R_mat, 3, 3, R);
    arm_mat_init_f32(&RT_mat, 3, 3, RT);

    arm_quaternion2rotation_f32((f32 *)q, &R, 1);
    arm_mat_trans_f32(&R_mat, &RT_mat);
    arm_mat_vec_mult_f32(&R_mat, mag, &mag_n);

    mag_n[2] = 0.0;
    f32 scale = sqrt(pow(mag_n[0], 2) + pow(mag_n[1], 2));
    for (size_t i = 0; i < 2; i++)
	mag_n[i] /= scale;

    arm_mat_vec_mult_f32(&RT_mat, &mag_n, mag);
}

Quaternion to_quaternion(EulerAngles att)
{
    Quaternion q;

    double cr = cos(att.roll * 0.5);
    double sr = sin(att.roll * 0.5);
    double cp = cos(att.pitch * 0.5);
    double sp = sin(att.pitch * 0.5);
    double cy = cos(att.yaw * 0.5);
    double sy = sin(att.yaw * 0.5);

    q.q0 = cr * cp * cy + sr * sp * sy;
    q.q1 = sr * cp * cy - cr * sp * sy;
    q.q2 = cr * sp * cy + sr * cp * sy;
    q.q3 = cr * cp * sy - sr * sp * cy;

    return q;
}

// Convert a quaternion to z-y-x euler angles (yaw, pitch, roll)
EulerAngles to_euler(Quaternion q)
{
    EulerAngles angles;

    // Roll
    f32 sinr_cosp = 2*((q.q0*q.q1) + (q.q2*q.q3));
    f32 cosr_cosp = 1 - 2*((q.q1*q.q1) + (q.q2*q.q2));

    angles.roll = atan2(sinr_cosp, cosr_cosp);

    // Pitch
    f32 sinp = sqrt(1 + 2*(q.q0*q.q2 - q.q1*q.q3));
    f32 cosp = sqrt(1 - 2*(q.q0*q.q2 - q.q1*q.q3));

    angles.pitch = 2*atan2(sinp, cosp) - M_PI_2;

    // yaw
    f32 siny_cosp = 2*(q.q0*q.q3 + q.q1*q.q2);
    f32 cosy_cosp = 1 - 2*(q.q2*q.q2 + q.q3*q.q3);

    angles.yaw = atan2(siny_cosp, cosy_cosp);

    return angles;
}
