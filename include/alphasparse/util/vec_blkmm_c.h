#pragma once

#include "../types.h"
#define BLOCK_DGEMM_RR_SQUARE BLOCK_C_DGEMM_RR_SQUARE
#define BLOCK_DGEMM_RR_GENERAL BLOCK_C_DGEMM_RR_GENERAL
#define BLOCK_DGEMM_CR_GENERAL BLOCK_C_DGEMM_CR_GENERAL
#define BLOCK_DGEMM_RR_SQUARE_CONJ2 BLOCK_C_DGEMM_RR_SQUARE_CONJ2
#define BLOCK_DGEMM_RR_GENERAL_CONJ2 BLOCK_C_DGEMM_RR_GENERAL_CONJ2
#define BLOCK_DGEMM_CR_GENERAL_CONJ2 BLOCK_C_DGEMM_CR_GENERAL_CONJ2
#define iszero(num) ((num).imag * (num).imag + (num).real * (num).real < 1e-20)
//gemm square
#ifdef CMLA
#define BLOCK_C_DGEMM_RR_SQUARE(C, A, B, bs, ldc, lda, ldb)                          \
    {                                                                                \
        float32x4_t z_v_00 = vdupq_n_f32(0);                                         \
        float32x4_t z_v_01 = vdupq_n_f32(0);                                         \
        float32x4_t z_v_10 = vdupq_n_f32(0);                                         \
        float32x4_t z_v_11 = vdupq_n_f32(0);                                         \
        float32x4_t y_v_00, y_v_01;                                                  \
        float32x4_t y_v_10, y_v_11;                                                  \
        float32x4_t x_v_00, x_v_01, x_v_10, x_v_11;                                  \
        for (ALPHA_INT __i = 0; __i < (bs); __i += 2)                                  \
        {                                                                            \
            for (ALPHA_INT j = 0; j < (bs); j += 4)                                    \
            {                                                                        \
                z_v_00 = vld1q_f32((void *)((C) + __i * (ldc) + j));                 \
                z_v_01 = vld1q_f32((void *)((C) + __i * (ldc) + j + 2));             \
                z_v_10 = vld1q_f32((void *)((C) + (__i + 1) * (ldc) + j));           \
                z_v_11 = vld1q_f32((void *)((C) + (__i + 1) * (ldc) + j + 2));       \
                for (ALPHA_INT k = 0; k < (bs); k += 2)                                \
                {                                                                    \
                    const ALPHA_Complex8 *X = (A) + __i * (lda) + k;                   \
                    const ALPHA_Complex8 *X1 = (A) + (__i + 1) * (lda) + k;            \
                    const ALPHA_Complex8 *Y00 = (B) + k * (ldb) + j;                   \
                    const ALPHA_Complex8 *Y10 = (B) + (k + 1) * (ldb) + j;             \
                    const ALPHA_Complex8 *Y01 = (B) + k * (ldb) + j + 2;               \
                    const ALPHA_Complex8 *Y11 = (B) + (k + 1) * (ldb) + j + 2;         \
                    x_v_00 = vreinterpretq_f32_f64(vld1q_dup_f64((void *)X));        \
                    x_v_01 = vreinterpretq_f32_f64(vld1q_dup_f64((void *)(X + 1)));  \
                    x_v_10 = vreinterpretq_f32_f64(vld1q_dup_f64((void *)(X1)));     \
                    x_v_11 = vreinterpretq_f32_f64(vld1q_dup_f64((void *)(X1 + 1))); \
                    y_v_00 = vld1q_f32((void *)Y00);                                 \
                    y_v_01 = vld1q_f32((void *)Y01);                                 \
                    y_v_10 = vld1q_f32((void *)Y10);                                 \
                    y_v_11 = vld1q_f32((void *)Y11);                                 \
                    z_v_00 = vcmlaq_f32(z_v_00, x_v_00, y_v_00);                     \
                    z_v_01 = vcmlaq_f32(z_v_01, x_v_00, y_v_01);                     \
                    z_v_10 = vcmlaq_f32(z_v_10, x_v_10, y_v_00);                     \
                    z_v_11 = vcmlaq_f32(z_v_11, x_v_10, y_v_01);                     \
                    z_v_00 = vcmlaq_rot90_f32(z_v_00, x_v_00, y_v_00);               \
                    z_v_01 = vcmlaq_rot90_f32(z_v_01, x_v_00, y_v_01);               \
                    z_v_10 = vcmlaq_rot90_f32(z_v_10, x_v_10, y_v_00);               \
                    z_v_11 = vcmlaq_rot90_f32(z_v_11, x_v_10, y_v_01);               \
                    z_v_00 = vcmlaq_f32(z_v_00, x_v_01, y_v_10);                     \
                    z_v_01 = vcmlaq_f32(z_v_01, x_v_01, y_v_11);                     \
                    z_v_10 = vcmlaq_f32(z_v_10, x_v_11, y_v_10);                     \
                    z_v_11 = vcmlaq_f32(z_v_11, x_v_11, y_v_11);                     \
                    z_v_00 = vcmlaq_rot90_f32(z_v_00, x_v_01, y_v_10);               \
                    z_v_01 = vcmlaq_rot90_f32(z_v_01, x_v_01, y_v_11);               \
                    z_v_10 = vcmlaq_rot90_f32(z_v_10, x_v_11, y_v_10);               \
                    z_v_11 = vcmlaq_rot90_f32(z_v_11, x_v_11, y_v_11);               \
                }                                                                    \
                vst1q_f32((void *)((C) + __i * (ldc) + j), z_v_00);                  \
                vst1q_f32((void *)((C) + __i * (ldc) + j + 2), z_v_01);              \
                vst1q_f32((void *)((C) + (__i + 1) * (ldc) + j), z_v_10);            \
                vst1q_f32((void *)((C) + (__i + 1) * (ldc) + j + 2), z_v_11);        \
            }                                                                        \
        }                                                                            \
    }
#elif defined(__aarch64__)
#define BLOCK_C_DGEMM_RR_SQUARE(C, A, B, bs, ldc, lda, ldb)                                   \
    for (ALPHA_INT __i = 0; __i < (bs); __i++)                                                  \
    {                                                                                         \
        for (ALPHA_INT j = 0; j < (bs); j += 4)                                                 \
        {                                                                                     \
            float32x4x2_t z_v_0 = vld2q_f32((float *)((C) + __i * (ldc) + j));                \
            float32x4x2_t y_v_0, y_v_1;                                                       \
            float32x4_t x_r_0, x_r_1, x_i_0, x_i_1, xr_yr, xr_yi, xi_yr, xi_yi, res_r, res_i; \
            for (ALPHA_INT k = 0; k < (bs); k += 2)                                             \
            {                                                                                 \
                const ALPHA_Complex8 *_A = (A) + __i * (lda) + k;                               \
                y_v_0 = vld2q_f32((float *)((B) + k * (ldb) + j));                            \
                x_r_0 = vdupq_n_f32(_A[0].real);                                              \
                x_i_0 = vdupq_n_f32(_A[0].imag);                                              \
                xr_yr = vmulq_f32(x_r_0, y_v_0.val[0]);                                       \
                xr_yi = vmulq_f32(x_r_0, y_v_0.val[1]);                                       \
                res_r = vfmsq_f32(xr_yr, x_i_0, y_v_0.val[1]);                                \
                res_i = vfmaq_f32(xr_yi, x_i_0, y_v_0.val[0]);                                \
                z_v_0.val[0] = vaddq_f32(z_v_0.val[0], res_r);                                \
                z_v_0.val[1] = vaddq_f32(z_v_0.val[1], res_i);                                \
                y_v_1 = vld2q_f32((float *)((B) + (k + 1) * (ldb) + j));                      \
                x_r_1 = vdupq_n_f32((_A[1].real));                                            \
                x_i_1 = vdupq_n_f32((_A[1].imag));                                            \
                xr_yr = vmulq_f32(x_r_1, y_v_1.val[0]);                                       \
                xr_yi = vmulq_f32(x_r_1, y_v_1.val[1]);                                       \
                res_r = vfmsq_f32(xr_yr, x_i_1, y_v_1.val[1]);                                \
                res_i = vfmaq_f32(xr_yi, x_i_1, y_v_1.val[0]);                                \
                z_v_0.val[0] = vaddq_f32(z_v_0.val[0], res_r);                                \
                z_v_0.val[1] = vaddq_f32(z_v_0.val[1], res_i);                                \
            }                                                                                 \
            vst2q_f32((float *)((C) + __i * (ldc) + j), z_v_0);                               \
        }                                                                                     \
    }
#else
#define BLOCK_C_DGEMM_RR_SQUARE(C, A, B, bs, ldc, lda, ldb)                                                                                         \
    for (ALPHA_INT __i = 0; __i < bs; __i++)                                                                                                          \
    {                                                                                                                                               \
        for (ALPHA_INT j = 0; j < bs; j++)                                                                                                            \
        {                                                                                                                                           \
            for (ALPHA_INT m = 0; m < bs - 3; m += 4)                                                                                                 \
            {                                                                                                                                       \
                ALPHA_Complex8 *tmp = C + __i * ldc + j;                                                                                              \
                tmp->real += (A)[__i * lda + m + 0].real * (B)[(m + 0) * ldb + j].real - (A)[__i * lda + m + 0].imag * (B)[(m + 0) * ldb + j].imag; \
                tmp->imag += (A)[__i * lda + m + 0].imag * (B)[(m + 0) * ldb + j].real + (A)[__i * lda + m + 0].real * (B)[(m + 0) * ldb + j].imag; \
                tmp->real += (A)[__i * lda + m + 1].real * (B)[(m + 1) * ldb + j].real - (A)[__i * lda + m + 1].imag * (B)[(m + 1) * ldb + j].imag; \
                tmp->imag += (A)[__i * lda + m + 1].imag * (B)[(m + 1) * ldb + j].real + (A)[__i * lda + m + 1].real * (B)[(m + 1) * ldb + j].imag; \
                tmp->real += (A)[__i * lda + m + 2].real * (B)[(m + 2) * ldb + j].real - (A)[__i * lda + m + 2].imag * (B)[(m + 2) * ldb + j].imag; \
                tmp->imag += (A)[__i * lda + m + 2].imag * (B)[(m + 2) * ldb + j].real + (A)[__i * lda + m + 2].real * (B)[(m + 2) * ldb + j].imag; \
                tmp->real += (A)[__i * lda + m + 3].real * (B)[(m + 3) * ldb + j].real - (A)[__i * lda + m + 3].imag * (B)[(m + 3) * ldb + j].imag; \
                tmp->imag += (A)[__i * lda + m + 3].imag * (B)[(m + 3) * ldb + j].real + (A)[__i * lda + m + 3].real * (B)[(m + 3) * ldb + j].imag; \
            }                                                                                                                                       \
        }                                                                                                                                           \
    }
#endif
//gemm rr
#ifdef CMLA
#define BLOCK_C_DGEMM_RR_GENERAL(C, A, B, bs, ldc, lda, N, ldb)                                                                                         \
    {                                                                                                                                                   \
        float32x4_t z_v_0, z_v_1, z_v_2, z_v_3;                                                                                                         \
        float32x4_t x_v_0;                                                                                                                              \
        float32x4_t y_v_0, y_v_1, y_v_2, y_v_3;                                                                                                         \
        for (ALPHA_INT __i = 0; __i < (bs); __i++)                                                                                                        \
        {                                                                                                                                               \
            for (ALPHA_INT k = 0; k < (bs); k++)                                                                                                          \
            {                                                                                                                                           \
                const ALPHA_Complex8 *X = (A) + __i * (lda) + k;                                                                                          \
                x_v_0 = vreinterpretq_f32_f64(vld1q_dup_f64((void *)X));                                                                                \
                if (iszero(X[0]))                                                                                                                       \
                    continue;                                                                                                                           \
                ALPHA_INT j = 0;                                                                                                                          \
                for (; j < (N)-7; j += 8)                                                                                                               \
                {                                                                                                                                       \
                    ALPHA_Complex8 *__C = (C) + __i * (ldc) + j;                                                                                          \
                    const ALPHA_Complex8 *__B = (B) + k * (ldb) + j;                                                                                      \
                    z_v_0 = vld1q_f32((void *)(__C + 0));                                                                                               \
                    z_v_1 = vld1q_f32((void *)(__C + 2));                                                                                               \
                    z_v_2 = vld1q_f32((void *)(__C + 4));                                                                                               \
                    z_v_3 = vld1q_f32((void *)(__C + 6));                                                                                               \
                    y_v_0 = vld1q_f32((void *)(__B + 0));                                                                                               \
                    y_v_1 = vld1q_f32((void *)(__B + 2));                                                                                               \
                    y_v_2 = vld1q_f32((void *)(__B + 4));                                                                                               \
                    y_v_3 = vld1q_f32((void *)(__B + 6));                                                                                               \
                    z_v_0 = vcmlaq_f32(z_v_0, y_v_0, x_v_0);                                                                                            \
                    z_v_1 = vcmlaq_f32(z_v_1, y_v_1, x_v_0);                                                                                            \
                    z_v_2 = vcmlaq_f32(z_v_2, y_v_2, x_v_0);                                                                                            \
                    z_v_3 = vcmlaq_f32(z_v_3, y_v_3, x_v_0);                                                                                            \
                    z_v_0 = vcmlaq_rot90_f32(z_v_0, y_v_0, x_v_0);                                                                                      \
                    z_v_1 = vcmlaq_rot90_f32(z_v_1, y_v_1, x_v_0);                                                                                      \
                    z_v_2 = vcmlaq_rot90_f32(z_v_2, y_v_2, x_v_0);                                                                                      \
                    z_v_3 = vcmlaq_rot90_f32(z_v_3, y_v_3, x_v_0);                                                                                      \
                    vst1q_f32((void *)(__C + 0), z_v_0);                                                                                                \
                    vst1q_f32((void *)(__C + 2), z_v_1);                                                                                                \
                    vst1q_f32((void *)(__C + 4), z_v_2);                                                                                                \
                    vst1q_f32((void *)(__C + 6), z_v_3);                                                                                                \
                }                                                                                                                                       \
                for (; j < (N)-1; j += 2)                                                                                                               \
                {                                                                                                                                       \
                    ALPHA_Complex8 *__C = (C) + __i * (ldc) + j;                                                                                          \
                    const ALPHA_Complex8 *__B = (B) + k * (ldb) + j;                                                                                      \
                    z_v_0 = vld1q_f32((void *)__C);                                                                                                     \
                    y_v_0 = vld1q_f32((void *)__B);                                                                                                     \
                    z_v_0 = vcmlaq_f32(z_v_0, y_v_0, x_v_0);                                                                                            \
                    z_v_0 = vcmlaq_rot90_f32(z_v_0, y_v_0, x_v_0);                                                                                      \
                    vst1q_f32((void *)__C, z_v_0);                                                                                                      \
                }                                                                                                                                       \
                for (; j < (N); j++)                                                                                                                    \
                {                                                                                                                                       \
                    ALPHA_Complex8 *tmp = C + __i * ldc + j;                                                                                              \
                    tmp->real += (A)[__i * lda + k + 0].real * (B)[(k + 0) * ldb + j].real - (A)[__i * lda + k + 0].imag * (B)[(k + 0) * ldb + j].imag; \
                    tmp->imag += (A)[__i * lda + k + 0].imag * (B)[(k + 0) * ldb + j].real + (A)[__i * lda + k + 0].real * (B)[(k + 0) * ldb + j].imag; \
                }                                                                                                                                       \
            }                                                                                                                                           \
        }                                                                                                                                               \
    }
#elif defined(__aarch64__)
#define BLOCK_C_DGEMM_RR_GENERAL(C, A, B, bs, ldc, lda, N, ldb)                                                                                     \
    for (ALPHA_INT __i = 0; __i < (bs); __i++)                                                                                                        \
    {                                                                                                                                               \
        for (ALPHA_INT k = 0; k < (bs); k++)                                                                                                          \
        {                                                                                                                                           \
            float32x4x2_t z_v_0, z_v_1;                                                                                                             \
            float32x4x2_t y_v_0, y_v_1;                                                                                                             \
            const ALPHA_Complex8 *__A = (A) + __i * (lda) + k;                                                                                        \
            float32x4_t x_r_0 = vdupq_n_f32(__A[0].real);                                                                                           \
            float32x4_t x_i_0 = vdupq_n_f32(__A[0].imag);                                                                                           \
            float32x4_t xr_yr0, xr_yi0, xi_yr0, xi_yi0, res_r0, res_i0,                                                                             \
                xr_yr1, xr_yi1, xi_yr1, xi_yi1, res_r1, res_i1;                                                                                     \
            ALPHA_INT j = 0;                                                                                                                          \
            if (iszero(__A[0]))                                                                                                                     \
                continue;                                                                                                                           \
            for (; j < (N)-7; j += 8)                                                                                                               \
            {                                                                                                                                       \
                ALPHA_Complex8 *__C = (C) + __i * (ldc) + j;                                                                                          \
                const ALPHA_Complex8 *__B = (B) + k * (ldb) + j;                                                                                      \
                z_v_0 = vld2q_f32((float *)((__C) + 0));                                                                                            \
                z_v_1 = vld2q_f32((float *)((__C) + 4));                                                                                            \
                y_v_0 = vld2q_f32((float *)((__B) + 0));                                                                                            \
                y_v_1 = vld2q_f32((float *)((__B) + 4));                                                                                            \
                xr_yr0 = vmulq_f32(x_r_0, y_v_0.val[0]);                                                                                            \
                xr_yi0 = vmulq_f32(x_r_0, y_v_0.val[1]);                                                                                            \
                xr_yr1 = vmulq_f32(x_r_0, y_v_1.val[0]);                                                                                            \
                xr_yi1 = vmulq_f32(x_r_0, y_v_1.val[1]);                                                                                            \
                res_r0 = vfmsq_f32(xr_yr0, x_i_0, y_v_0.val[1]);                                                                                    \
                res_i0 = vfmaq_f32(xr_yi0, x_i_0, y_v_0.val[0]);                                                                                    \
                res_r1 = vfmsq_f32(xr_yr1, x_i_0, y_v_1.val[1]);                                                                                    \
                res_i1 = vfmaq_f32(xr_yi1, x_i_0, y_v_1.val[0]);                                                                                    \
                z_v_0.val[0] = vaddq_f32(z_v_0.val[0], res_r0);                                                                                     \
                z_v_0.val[1] = vaddq_f32(z_v_0.val[1], res_i0);                                                                                     \
                z_v_1.val[0] = vaddq_f32(z_v_1.val[0], res_r1);                                                                                     \
                z_v_1.val[1] = vaddq_f32(z_v_1.val[1], res_i1);                                                                                     \
                vst2q_f32((float *)((__C) + 0), z_v_0);                                                                                             \
                vst2q_f32((float *)((__C) + 4), z_v_1);                                                                                             \
            }                                                                                                                                       \
            for (; j < (N)-3; j += 4)                                                                                                               \
            {                                                                                                                                       \
                ALPHA_Complex8 *__C = (C) + __i * (ldc) + j;                                                                                          \
                const ALPHA_Complex8 *__B = (B) + k * (ldb) + j;                                                                                      \
                z_v_0 = vld2q_f32((float *)((__C) + 0));                                                                                            \
                y_v_0 = vld2q_f32((float *)((__B) + 0));                                                                                            \
                xr_yr0 = vmulq_f32(x_r_0, y_v_0.val[0]);                                                                                            \
                xr_yi0 = vmulq_f32(x_r_0, y_v_0.val[1]);                                                                                            \
                res_r0 = vfmsq_f32(xr_yr0, x_i_0, y_v_0.val[1]);                                                                                    \
                res_i0 = vfmaq_f32(xr_yi0, x_i_0, y_v_0.val[0]);                                                                                    \
                z_v_0.val[0] = vaddq_f32(z_v_0.val[0], res_r0);                                                                                     \
                z_v_0.val[1] = vaddq_f32(z_v_0.val[1], res_i0);                                                                                     \
                vst2q_f32((float *)((__C) + 0), z_v_0);                                                                                             \
            }                                                                                                                                       \
            for (; j < (N); j++)                                                                                                                    \
            {                                                                                                                                       \
                ALPHA_Complex8 *tmp = C + __i * ldc + j;                                                                                              \
                tmp->real += (A)[__i * lda + k + 0].real * (B)[(k + 0) * ldb + j].real - (A)[__i * lda + k + 0].imag * (B)[(k + 0) * ldb + j].imag; \
                tmp->imag += (A)[__i * lda + k + 0].imag * (B)[(k + 0) * ldb + j].real + (A)[__i * lda + k + 0].real * (B)[(k + 0) * ldb + j].imag; \
            }                                                                                                                                       \
        }                                                                                                                                           \
    }
#else
#define BLOCK_C_DGEMM_RR_GENERAL(C, A, B, bs, ldc, lda, N, ldb)                                                                                     \
    for (ALPHA_INT __i = 0; __i < bs; __i++)                                                                                                          \
    {                                                                                                                                               \
        for (ALPHA_INT j = 0; j < N; j++)                                                                                                             \
        {                                                                                                                                           \
            for (ALPHA_INT m = 0; m < bs; m++)                                                                                                        \
            {                                                                                                                                       \
                ALPHA_Complex8 *tmp = C + __i * ldc + j;                                                                                              \
                tmp->real += (A)[__i * lda + m + 0].real * (B)[(m + 0) * ldb + j].real - (A)[__i * lda + m + 0].imag * (B)[(m + 0) * ldb + j].imag; \
                tmp->imag += (A)[__i * lda + m + 0].imag * (B)[(m + 0) * ldb + j].real + (A)[__i * lda + m + 0].real * (B)[(m + 0) * ldb + j].imag; \
            }                                                                                                                                       \
        }                                                                                                                                           \
    }
#endif
//gemm cr
#ifdef CMLA
#define BLOCK_C_DGEMM_CR_GENERAL(C, A, B, bs, ldc, lda, N, ldb)                                                                                         \
    {                                                                                                                                                   \
        float32x4_t z_v_0, z_v_1, z_v_2, z_v_3;                                                                                                         \
        float32x4_t x_v_0;                                                                                                                              \
        float32x4_t y_v_0, y_v_1, y_v_2, y_v_3;                                                                                                         \
        for (ALPHA_INT __i = 0; __i < (bs); __i++)                                                                                                        \
        {                                                                                                                                               \
            for (ALPHA_INT k = 0; k < (bs); k++)                                                                                                          \
            {                                                                                                                                           \
                const ALPHA_Complex8 *X = (A) + k * (lda) + __i;                                                                                          \
                x_v_0 = vreinterpretq_f32_f64(vld1q_dup_f64((void *)X));                                                                                \
                ALPHA_INT j = 0;                                                                                                                          \
                if (iszero(X[0]))                                                                                                                       \
                    continue;                                                                                                                           \
                for (; j < (N)-7; j += 8)                                                                                                               \
                {                                                                                                                                       \
                    ALPHA_Complex8 *__C = (C) + __i * (ldc) + j;                                                                                          \
                    const ALPHA_Complex8 *__B = (B) + k * (ldb) + j;                                                                                      \
                    z_v_0 = vld1q_f32((void *)(__C + 0));                                                                                               \
                    z_v_1 = vld1q_f32((void *)(__C + 2));                                                                                               \
                    z_v_2 = vld1q_f32((void *)(__C + 4));                                                                                               \
                    z_v_3 = vld1q_f32((void *)(__C + 6));                                                                                               \
                    y_v_0 = vld1q_f32((void *)(__B + 0));                                                                                               \
                    y_v_1 = vld1q_f32((void *)(__B + 2));                                                                                               \
                    y_v_2 = vld1q_f32((void *)(__B + 4));                                                                                               \
                    y_v_3 = vld1q_f32((void *)(__B + 6));                                                                                               \
                    z_v_0 = vcmlaq_f32(z_v_0, y_v_0, x_v_0);                                                                                            \
                    z_v_1 = vcmlaq_f32(z_v_1, y_v_1, x_v_0);                                                                                            \
                    z_v_2 = vcmlaq_f32(z_v_2, y_v_2, x_v_0);                                                                                            \
                    z_v_3 = vcmlaq_f32(z_v_3, y_v_3, x_v_0);                                                                                            \
                    z_v_0 = vcmlaq_rot90_f32(z_v_0, y_v_0, x_v_0);                                                                                      \
                    z_v_1 = vcmlaq_rot90_f32(z_v_1, y_v_1, x_v_0);                                                                                      \
                    z_v_2 = vcmlaq_rot90_f32(z_v_2, y_v_2, x_v_0);                                                                                      \
                    z_v_3 = vcmlaq_rot90_f32(z_v_3, y_v_3, x_v_0);                                                                                      \
                    vst1q_f32((void *)(__C + 0), z_v_0);                                                                                                \
                    vst1q_f32((void *)(__C + 2), z_v_1);                                                                                                \
                    vst1q_f32((void *)(__C + 4), z_v_2);                                                                                                \
                    vst1q_f32((void *)(__C + 6), z_v_3);                                                                                                \
                }                                                                                                                                       \
                for (; j < (N)-1; j += 2)                                                                                                               \
                {                                                                                                                                       \
                    ALPHA_Complex8 *__C = (C) + __i * (ldc) + j;                                                                                          \
                    const ALPHA_Complex8 *__B = (B) + k * (ldb) + j;                                                                                      \
                    z_v_0 = vld1q_f32((void *)__C);                                                                                                     \
                    y_v_0 = vld1q_f32((void *)__B);                                                                                                     \
                    z_v_0 = vcmlaq_f32(z_v_0, y_v_0, x_v_0);                                                                                            \
                    z_v_0 = vcmlaq_rot90_f32(z_v_0, y_v_0, x_v_0);                                                                                      \
                    vst1q_f32((void *)__C, z_v_0);                                                                                                      \
                }                                                                                                                                       \
                for (; j < (N); j++)                                                                                                                    \
                {                                                                                                                                       \
                    ALPHA_Complex8 *tmp = C + __i * ldc + j;                                                                                              \
                    tmp->real += (A)[k * lda + __i + 0].real * (B)[(k + 0) * ldb + j].real - (A)[k * lda + __i + 0].imag * (B)[(k + 0) * ldb + j].imag; \
                    tmp->imag += (A)[k * lda + __i + 0].imag * (B)[(k + 0) * ldb + j].real + (A)[k * lda + __i + 0].real * (B)[(k + 0) * ldb + j].imag; \
                }                                                                                                                                       \
            }                                                                                                                                           \
        }                                                                                                                                               \
    }
#elif defined(__aarch64__)
#define BLOCK_C_DGEMM_CR_GENERAL(C, A, B, bs, ldc, lda, N, ldb)                                                                                     \
    for (ALPHA_INT __i = 0; __i < (bs); __i++)                                                                                                        \
    {                                                                                                                                               \
        for (ALPHA_INT k = 0; k < (bs); k++)                                                                                                          \
        {                                                                                                                                           \
            float32x4x2_t z_v_0, z_v_1;                                                                                                             \
            float32x4x2_t y_v_0, y_v_1;                                                                                                             \
            const ALPHA_Complex8 *__A = (A) + k * (lda) + __i;                                                                                        \
            float32x4_t x_r_0 = vdupq_n_f32(__A[0].real);                                                                                           \
            float32x4_t x_i_0 = vdupq_n_f32(__A[0].imag);                                                                                           \
            float32x4_t xr_yr0, xr_yi0, xi_yr0, xi_yi0, res_r0, res_i0,                                                                             \
                xr_yr1, xr_yi1, xi_yr1, xi_yi1, res_r1, res_i1;                                                                                     \
            ALPHA_INT j = 0;                                                                                                                          \
            if (iszero(__A[0]))                                                                                                                     \
                continue;                                                                                                                           \
            for (; j < (N)-7; j += 8)                                                                                                               \
            {                                                                                                                                       \
                ALPHA_Complex8 *__C = (C) + __i * (ldc) + j;                                                                                          \
                const ALPHA_Complex8 *__B = (B) + k * (ldb) + j;                                                                                      \
                z_v_0 = vld2q_f32((float *)((__C) + 0));                                                                                            \
                z_v_1 = vld2q_f32((float *)((__C) + 4));                                                                                            \
                y_v_0 = vld2q_f32((float *)((__B) + 0));                                                                                            \
                y_v_1 = vld2q_f32((float *)((__B) + 4));                                                                                            \
                xr_yr0 = vmulq_f32(x_r_0, y_v_0.val[0]);                                                                                            \
                xr_yi0 = vmulq_f32(x_r_0, y_v_0.val[1]);                                                                                            \
                xr_yr1 = vmulq_f32(x_r_0, y_v_1.val[0]);                                                                                            \
                xr_yi1 = vmulq_f32(x_r_0, y_v_1.val[1]);                                                                                            \
                res_r0 = vfmsq_f32(xr_yr0, x_i_0, y_v_0.val[1]);                                                                                    \
                res_i0 = vfmaq_f32(xr_yi0, x_i_0, y_v_0.val[0]);                                                                                    \
                res_r1 = vfmsq_f32(xr_yr1, x_i_0, y_v_1.val[1]);                                                                                    \
                res_i1 = vfmaq_f32(xr_yi1, x_i_0, y_v_1.val[0]);                                                                                    \
                z_v_0.val[0] = vaddq_f32(z_v_0.val[0], res_r0);                                                                                     \
                z_v_0.val[1] = vaddq_f32(z_v_0.val[1], res_i0);                                                                                     \
                z_v_1.val[0] = vaddq_f32(z_v_1.val[0], res_r1);                                                                                     \
                z_v_1.val[1] = vaddq_f32(z_v_1.val[1], res_i1);                                                                                     \
                vst2q_f32((float *)((__C) + 0), z_v_0);                                                                                             \
                vst2q_f32((float *)((__C) + 4), z_v_1);                                                                                             \
            }                                                                                                                                       \
            for (; j < (N)-3; j += 4)                                                                                                               \
            {                                                                                                                                       \
                ALPHA_Complex8 *__C = (C) + __i * (ldc) + j;                                                                                          \
                const ALPHA_Complex8 *__B = (B) + k * (ldb) + j;                                                                                      \
                z_v_0 = vld2q_f32((float *)((__C) + 0));                                                                                            \
                y_v_0 = vld2q_f32((float *)((__B) + 0));                                                                                            \
                xr_yr0 = vmulq_f32(x_r_0, y_v_0.val[0]);                                                                                            \
                xr_yi0 = vmulq_f32(x_r_0, y_v_0.val[1]);                                                                                            \
                res_r0 = vfmsq_f32(xr_yr0, x_i_0, y_v_0.val[1]);                                                                                    \
                res_i0 = vfmaq_f32(xr_yi0, x_i_0, y_v_0.val[0]);                                                                                    \
                z_v_0.val[0] = vaddq_f32(z_v_0.val[0], res_r0);                                                                                     \
                z_v_0.val[1] = vaddq_f32(z_v_0.val[1], res_i0);                                                                                     \
                vst2q_f32((float *)((__C) + 0), z_v_0);                                                                                             \
            }                                                                                                                                       \
            for (; j < (N); j++)                                                                                                                    \
            {                                                                                                                                       \
                ALPHA_Complex8 *tmp = C + __i * ldc + j;                                                                                              \
                tmp->real += (A)[k * lda + __i + 0].real * (B)[(k + 0) * ldb + j].real - (A)[k * lda + __i + 0].imag * (B)[(k + 0) * ldb + j].imag; \
                tmp->imag += (A)[k * lda + __i + 0].imag * (B)[(k + 0) * ldb + j].real + (A)[k * lda + __i + 0].real * (B)[(k + 0) * ldb + j].imag; \
            }                                                                                                                                       \
        }                                                                                                                                           \
    }
#else
#define BLOCK_C_DGEMM_CR_GENERAL(C, A, B, bs, ldc, lda, N, ldb)                                                                                     \
    for (ALPHA_INT __i = 0; __i < bs; __i++)                                                                                                          \
    {                                                                                                                                               \
        for (ALPHA_INT m = 0; m < bs; m++)                                                                                                            \
        {                                                                                                                                           \
            for (ALPHA_INT j = 0; j < N; j++)                                                                                                         \
            {                                                                                                                                       \
                ALPHA_Complex8 *tmp = C + __i * ldc + j;                                                                                              \
                tmp->real += (A)[m * lda + __i + 0].real * (B)[(m + 0) * ldb + j].real - (A)[m * lda + __i + 0].imag * (B)[(m + 0) * ldb + j].imag; \
                tmp->imag += (A)[m * lda + __i + 0].imag * (B)[(m + 0) * ldb + j].real + (A)[m * lda + __i + 0].real * (B)[(m + 0) * ldb + j].imag; \
            }                                                                                                                                       \
        }                                                                                                                                           \
    }
#endif
#ifdef CMLA
#define BLOCK_C_DGEMM_RR_SQUARE_CONJ2(C, A, B, bs, ldc, lda, ldb)                    \
    {                                                                                \
        float32x4_t z_v_00 = vdupq_n_f32(0);                                         \
        float32x4_t z_v_01 = vdupq_n_f32(0);                                         \
        float32x4_t z_v_10 = vdupq_n_f32(0);                                         \
        float32x4_t z_v_11 = vdupq_n_f32(0);                                         \
        float32x4_t y_v_00, y_v_01;                                                  \
        float32x4_t y_v_10, y_v_11;                                                  \
        float32x4_t x_v_00, x_v_01, x_v_10, x_v_11;                                  \
        for (ALPHA_INT __i = 0; __i < (bs); __i += 2)                                  \
        {                                                                            \
            for (ALPHA_INT j = 0; j < (bs); j += 4)                                    \
            {                                                                        \
                z_v_00 = vld1q_f32((void *)((C) + __i * (ldc) + j));                 \
                z_v_01 = vld1q_f32((void *)((C) + __i * (ldc) + j + 2));             \
                z_v_10 = vld1q_f32((void *)((C) + (__i + 1) * (ldc) + j));           \
                z_v_11 = vld1q_f32((void *)((C) + (__i + 1) * (ldc) + j + 2));       \
                for (ALPHA_INT k = 0; k < (bs); k += 2)                                \
                {                                                                    \
                    const ALPHA_Complex8 *X = (A) + __i * (lda) + k;                   \
                    const ALPHA_Complex8 *X1 = (A) + (__i + 1) * (lda) + k;            \
                    const ALPHA_Complex8 *Y00 = (B) + k * (ldb) + j;                   \
                    const ALPHA_Complex8 *Y10 = (B) + (k + 1) * (ldb) + j;             \
                    const ALPHA_Complex8 *Y01 = (B) + k * (ldb) + j + 2;               \
                    const ALPHA_Complex8 *Y11 = (B) + (k + 1) * (ldb) + j + 2;         \
                    x_v_00 = vreinterpretq_f32_f64(vld1q_dup_f64((void *)X));        \
                    x_v_01 = vreinterpretq_f32_f64(vld1q_dup_f64((void *)(X + 1)));  \
                    x_v_10 = vreinterpretq_f32_f64(vld1q_dup_f64((void *)(X1)));     \
                    x_v_11 = vreinterpretq_f32_f64(vld1q_dup_f64((void *)(X1 + 1))); \
                    y_v_00 = vld1q_f32((void *)Y00);                                 \
                    y_v_01 = vld1q_f32((void *)Y01);                                 \
                    y_v_10 = vld1q_f32((void *)Y10);                                 \
                    y_v_11 = vld1q_f32((void *)Y11);                                 \
                    z_v_00 = vcmlaq_f32(z_v_00, x_v_00, y_v_00);                     \
                    z_v_01 = vcmlaq_f32(z_v_01, x_v_00, y_v_01);                     \
                    z_v_10 = vcmlaq_f32(z_v_10, x_v_10, y_v_00);                     \
                    z_v_11 = vcmlaq_f32(z_v_11, x_v_10, y_v_01);                     \
                    z_v_00 = vcmlaq_rot270_f32(z_v_00, x_v_00, y_v_00);              \
                    z_v_01 = vcmlaq_rot270_f32(z_v_01, x_v_00, y_v_01);              \
                    z_v_10 = vcmlaq_rot270_f32(z_v_10, x_v_10, y_v_00);              \
                    z_v_11 = vcmlaq_rot270_f32(z_v_11, x_v_10, y_v_01);              \
                    z_v_00 = vcmlaq_f32(z_v_00, x_v_01, y_v_10);                     \
                    z_v_01 = vcmlaq_f32(z_v_01, x_v_01, y_v_11);                     \
                    z_v_10 = vcmlaq_f32(z_v_10, x_v_11, y_v_10);                     \
                    z_v_11 = vcmlaq_f32(z_v_11, x_v_11, y_v_11);                     \
                    z_v_00 = vcmlaq_rot270_f32(z_v_00, x_v_01, y_v_10);              \
                    z_v_01 = vcmlaq_rot270_f32(z_v_01, x_v_01, y_v_11);              \
                    z_v_10 = vcmlaq_rot270_f32(z_v_10, x_v_11, y_v_10);              \
                    z_v_11 = vcmlaq_rot270_f32(z_v_11, x_v_11, y_v_11);              \
                }                                                                    \
                vst1q_f32((void *)((C) + __i * (ldc) + j), z_v_00);                  \
                vst1q_f32((void *)((C) + __i * (ldc) + j + 2), z_v_01);              \
                vst1q_f32((void *)((C) + (__i + 1) * (ldc) + j), z_v_10);            \
                vst1q_f32((void *)((C) + (__i + 1) * (ldc) + j + 2), z_v_11);        \
            }                                                                        \
        }                                                                            \
    }
#elif defined(__aarch64__)
#define BLOCK_C_DGEMM_RR_SQUARE_CONJ2(C, A, B, bs, ldc, lda, ldb)                             \
    for (ALPHA_INT __i = 0; __i < (bs); __i++)                                                  \
    {                                                                                         \
        for (ALPHA_INT j = 0; j < (bs); j += 4)                                                 \
        {                                                                                     \
            float32x4x2_t z_v_0 = vld2q_f32((float *)((C) + __i * (ldc) + j));                \
            float32x4x2_t y_v_0, y_v_1;                                                       \
            float32x4_t x_r_0, x_r_1, x_i_0, x_i_1, xr_yr, xr_yi, xi_yr, xi_yi, res_r, res_i; \
            for (ALPHA_INT k = 0; k < (bs); k += 2)                                             \
            {                                                                                 \
                const ALPHA_Complex8 *_A = (A) + __i * (lda) + k;                               \
                y_v_0 = vld2q_f32((float *)((B) + k * (ldb) + j));                            \
                x_r_0 = vdupq_n_f32(_A[0].real);                                              \
                x_i_0 = vdupq_n_f32(-_A[0].imag);                                             \
                xr_yr = vmulq_f32(x_r_0, y_v_0.val[0]);                                       \
                xr_yi = vmulq_f32(x_r_0, y_v_0.val[1]);                                       \
                res_r = vfmsq_f32(xr_yr, x_i_0, y_v_0.val[1]);                                \
                res_i = vfmaq_f32(xr_yi, x_i_0, y_v_0.val[0]);                                \
                z_v_0.val[0] = vaddq_f32(z_v_0.val[0], res_r);                                \
                z_v_0.val[1] = vaddq_f32(z_v_0.val[1], res_i);                                \
                y_v_1 = vld2q_f32((float *)((B) + (k + 1) * (ldb) + j));                      \
                x_r_1 = vdupq_n_f32((_A[1].real));                                            \
                x_i_1 = vdupq_n_f32(-(_A[1].imag));                                           \
                xr_yr = vmulq_f32(x_r_1, y_v_1.val[0]);                                       \
                xr_yi = vmulq_f32(x_r_1, y_v_1.val[1]);                                       \
                res_r = vfmsq_f32(xr_yr, x_i_1, y_v_1.val[1]);                                \
                res_i = vfmaq_f32(xr_yi, x_i_1, y_v_1.val[0]);                                \
                z_v_0.val[0] = vaddq_f32(z_v_0.val[0], res_r);                                \
                z_v_0.val[1] = vaddq_f32(z_v_0.val[1], res_i);                                \
            }                                                                                 \
            vst2q_f32((float *)((C) + __i * (ldc) + j), z_v_0);                               \
        }                                                                                     \
    }
#else
#define BLOCK_C_DGEMM_RR_SQUARE_CONJ2(C, A, B, bs, ldc, lda, ldb)                                                                                    \
    for (ALPHA_INT __i = 0; __i < bs; __i++)                                                                                                           \
    {                                                                                                                                                \
        for (ALPHA_INT j = 0; j < bs; j++)                                                                                                             \
        {                                                                                                                                            \
            for (ALPHA_INT m = 0; m < bs - 3; m += 4)                                                                                                  \
            {                                                                                                                                        \
                ALPHA_Complex8 *tmp = C + __i * ldc + j;                                                                                               \
                tmp->real += (A)[__i * lda + m + 0].real * (B)[(m + 0) * ldb + j].real + (A)[__i * lda + m + 0].imag * (B)[(m + 0) * ldb + j].imag;  \
                tmp->imag += -(A)[__i * lda + m + 0].imag * (B)[(m + 0) * ldb + j].real + (A)[__i * lda + m + 0].real * (B)[(m + 0) * ldb + j].imag; \
                tmp->real += (A)[__i * lda + m + 1].real * (B)[(m + 1) * ldb + j].real + (A)[__i * lda + m + 1].imag * (B)[(m + 1) * ldb + j].imag;  \
                tmp->imag += -(A)[__i * lda + m + 1].imag * (B)[(m + 1) * ldb + j].real + (A)[__i * lda + m + 1].real * (B)[(m + 1) * ldb + j].imag; \
                tmp->real += (A)[__i * lda + m + 2].real * (B)[(m + 2) * ldb + j].real + (A)[__i * lda + m + 2].imag * (B)[(m + 2) * ldb + j].imag;  \
                tmp->imag += -(A)[__i * lda + m + 2].imag * (B)[(m + 2) * ldb + j].real + (A)[__i * lda + m + 2].real * (B)[(m + 2) * ldb + j].imag; \
                tmp->real += (A)[__i * lda + m + 3].real * (B)[(m + 3) * ldb + j].real + (A)[__i * lda + m + 3].imag * (B)[(m + 3) * ldb + j].imag;  \
                tmp->imag += -(A)[__i * lda + m + 3].imag * (B)[(m + 3) * ldb + j].real + (A)[__i * lda + m + 3].real * (B)[(m + 3) * ldb + j].imag; \
            }                                                                                                                                        \
        }                                                                                                                                            \
    }
#endif
#ifdef CMLA
#define BLOCK_C_DGEMM_RR_GENERAL_CONJ2(C, A, B, bs, ldc, lda, N, ldb)                                                                                    \
    {                                                                                                                                                    \
        float32x4_t z_v_0, z_v_1, z_v_2, z_v_3;                                                                                                          \
        float32x4_t x_v_0;                                                                                                                               \
        float32x4_t y_v_0, y_v_1, y_v_2, y_v_3;                                                                                                          \
        for (ALPHA_INT __i = 0; __i < (bs); __i++)                                                                                                         \
        {                                                                                                                                                \
            for (ALPHA_INT k = 0; k < (bs); k++)                                                                                                           \
            {                                                                                                                                            \
                const ALPHA_Complex8 *X = (A) + __i * (lda) + k;                                                                                           \
                x_v_0 = vreinterpretq_f32_f64(vld1q_dup_f64((void *)X));                                                                                 \
                ALPHA_INT j = 0;                                                                                                                           \
                if (iszero(X[0]))                                                                                                                        \
                    continue;                                                                                                                            \
                for (; j < (N)-7; j += 8)                                                                                                                \
                {                                                                                                                                        \
                    ALPHA_Complex8 *__C = (C) + __i * (ldc) + j;                                                                                           \
                    const ALPHA_Complex8 *__B = (B) + k * (ldb) + j;                                                                                       \
                    z_v_0 = vld1q_f32((void *)(__C + 0));                                                                                                \
                    z_v_1 = vld1q_f32((void *)(__C + 2));                                                                                                \
                    z_v_2 = vld1q_f32((void *)(__C + 4));                                                                                                \
                    z_v_3 = vld1q_f32((void *)(__C + 6));                                                                                                \
                    y_v_0 = vld1q_f32((void *)(__B + 0));                                                                                                \
                    y_v_1 = vld1q_f32((void *)(__B + 2));                                                                                                \
                    y_v_2 = vld1q_f32((void *)(__B + 4));                                                                                                \
                    y_v_3 = vld1q_f32((void *)(__B + 6));                                                                                                \
                    z_v_0 = vcmlaq_f32(z_v_0, x_v_0, y_v_0);                                                                                             \
                    z_v_1 = vcmlaq_f32(z_v_1, x_v_0, y_v_1);                                                                                             \
                    z_v_2 = vcmlaq_f32(z_v_2, x_v_0, y_v_2);                                                                                             \
                    z_v_3 = vcmlaq_f32(z_v_3, x_v_0, y_v_3);                                                                                             \
                    z_v_0 = vcmlaq_rot270_f32(z_v_0, x_v_0, y_v_0);                                                                                      \
                    z_v_1 = vcmlaq_rot270_f32(z_v_1, x_v_0, y_v_1);                                                                                      \
                    z_v_2 = vcmlaq_rot270_f32(z_v_2, x_v_0, y_v_2);                                                                                      \
                    z_v_3 = vcmlaq_rot270_f32(z_v_3, x_v_0, y_v_3);                                                                                      \
                    vst1q_f32((void *)(__C + 0), z_v_0);                                                                                                 \
                    vst1q_f32((void *)(__C + 2), z_v_1);                                                                                                 \
                    vst1q_f32((void *)(__C + 4), z_v_2);                                                                                                 \
                    vst1q_f32((void *)(__C + 6), z_v_3);                                                                                                 \
                }                                                                                                                                        \
                for (; j < (N)-1; j += 2)                                                                                                                \
                {                                                                                                                                        \
                    ALPHA_Complex8 *__C = (C) + __i * (ldc) + j;                                                                                           \
                    const ALPHA_Complex8 *__B = (B) + k * (ldb) + j;                                                                                       \
                    z_v_0 = vld1q_f32((void *)__C);                                                                                                      \
                    y_v_0 = vld1q_f32((void *)__B);                                                                                                      \
                    z_v_0 = vcmlaq_f32(z_v_0, x_v_0, y_v_0);                                                                                             \
                    z_v_0 = vcmlaq_rot270_f32(z_v_0, x_v_0, y_v_0);                                                                                      \
                    vst1q_f32((void *)__C, z_v_0);                                                                                                       \
                }                                                                                                                                        \
                for (; j < (N); j++)                                                                                                                     \
                {                                                                                                                                        \
                    ALPHA_Complex8 *tmp = C + __i * ldc + j;                                                                                               \
                    tmp->real += (A)[__i * lda + k + 0].real * (B)[(k + 0) * ldb + j].real + (A)[__i * lda + k + 0].imag * (B)[(k + 0) * ldb + j].imag;  \
                    tmp->imag += -(A)[__i * lda + k + 0].imag * (B)[(k + 0) * ldb + j].real + (A)[__i * lda + k + 0].real * (B)[(k + 0) * ldb + j].imag; \
                }                                                                                                                                        \
            }                                                                                                                                            \
        }                                                                                                                                                \
    }
#elif defined(__aarch64__)
#define BLOCK_C_DGEMM_RR_GENERAL_CONJ2(C, A, B, bs, ldc, lda, N, ldb)                                                                                \
    for (ALPHA_INT __i = 0; __i < (bs); __i++)                                                                                                         \
    {                                                                                                                                                \
        for (ALPHA_INT k = 0; k < (bs); k++)                                                                                                           \
        {                                                                                                                                            \
            float32x4x2_t z_v_0, z_v_1;                                                                                                              \
            float32x4x2_t y_v_0, y_v_1;                                                                                                              \
            const ALPHA_Complex8 *__A = (A) + __i * (lda) + k;                                                                                         \
            float32x4_t x_r_0 = vdupq_n_f32(__A[0].real);                                                                                            \
            float32x4_t x_i_0 = vdupq_n_f32(-__A[0].imag);                                                                                           \
            float32x4_t xr_yr0, xr_yi0, xi_yr0, xi_yi0, res_r0, res_i0,                                                                              \
                xr_yr1, xr_yi1, xi_yr1, xi_yi1, res_r1, res_i1;                                                                                      \
            ALPHA_INT j = 0;                                                                                                                           \
            if (iszero(__A[0]))                                                                                                                      \
                continue;                                                                                                                            \
            for (; j < (N)-7; j += 8)                                                                                                                \
            {                                                                                                                                        \
                ALPHA_Complex8 *__C = (C) + __i * (ldc) + j;                                                                                           \
                const ALPHA_Complex8 *__B = (B) + k * (ldb) + j;                                                                                       \
                z_v_0 = vld2q_f32((float *)((__C) + 0));                                                                                             \
                z_v_1 = vld2q_f32((float *)((__C) + 4));                                                                                             \
                y_v_0 = vld2q_f32((float *)((__B) + 0));                                                                                             \
                y_v_1 = vld2q_f32((float *)((__B) + 4));                                                                                             \
                xr_yr0 = vmulq_f32(x_r_0, y_v_0.val[0]);                                                                                             \
                xr_yi0 = vmulq_f32(x_r_0, y_v_0.val[1]);                                                                                             \
                xr_yr1 = vmulq_f32(x_r_0, y_v_1.val[0]);                                                                                             \
                xr_yi1 = vmulq_f32(x_r_0, y_v_1.val[1]);                                                                                             \
                res_r0 = vfmsq_f32(xr_yr0, x_i_0, y_v_0.val[1]);                                                                                     \
                res_i0 = vfmaq_f32(xr_yi0, x_i_0, y_v_0.val[0]);                                                                                     \
                res_r1 = vfmsq_f32(xr_yr1, x_i_0, y_v_1.val[1]);                                                                                     \
                res_i1 = vfmaq_f32(xr_yi1, x_i_0, y_v_1.val[0]);                                                                                     \
                z_v_0.val[0] = vaddq_f32(z_v_0.val[0], res_r0);                                                                                      \
                z_v_0.val[1] = vaddq_f32(z_v_0.val[1], res_i0);                                                                                      \
                z_v_1.val[0] = vaddq_f32(z_v_1.val[0], res_r1);                                                                                      \
                z_v_1.val[1] = vaddq_f32(z_v_1.val[1], res_i1);                                                                                      \
                vst2q_f32((float *)((__C) + 0), z_v_0);                                                                                              \
                vst2q_f32((float *)((__C) + 4), z_v_1);                                                                                              \
            }                                                                                                                                        \
            for (; j < (N)-3; j += 4)                                                                                                                \
            {                                                                                                                                        \
                ALPHA_Complex8 *__C = (C) + __i * (ldc) + j;                                                                                           \
                const ALPHA_Complex8 *__B = (B) + k * (ldb) + j;                                                                                       \
                z_v_0 = vld2q_f32((float *)((__C) + 0));                                                                                             \
                y_v_0 = vld2q_f32((float *)((__B) + 0));                                                                                             \
                xr_yr0 = vmulq_f32(x_r_0, y_v_0.val[0]);                                                                                             \
                xr_yi0 = vmulq_f32(x_r_0, y_v_0.val[1]);                                                                                             \
                res_r0 = vfmsq_f32(xr_yr0, x_i_0, y_v_0.val[1]);                                                                                     \
                res_i0 = vfmaq_f32(xr_yi0, x_i_0, y_v_0.val[0]);                                                                                     \
                z_v_0.val[0] = vaddq_f32(z_v_0.val[0], res_r0);                                                                                      \
                z_v_0.val[1] = vaddq_f32(z_v_0.val[1], res_i0);                                                                                      \
                vst2q_f32((float *)((__C) + 0), z_v_0);                                                                                              \
            }                                                                                                                                        \
            for (; j < (N); j++)                                                                                                                     \
            {                                                                                                                                        \
                ALPHA_Complex8 *tmp = C + __i * ldc + j;                                                                                               \
                tmp->real += (A)[__i * lda + k + 0].real * (B)[(k + 0) * ldb + j].real + (A)[__i * lda + k + 0].imag * (B)[(k + 0) * ldb + j].imag;  \
                tmp->imag += -(A)[__i * lda + k + 0].imag * (B)[(k + 0) * ldb + j].real + (A)[__i * lda + k + 0].real * (B)[(k + 0) * ldb + j].imag; \
            }                                                                                                                                        \
        }                                                                                                                                            \
    }
#else
#define BLOCK_C_DGEMM_RR_GENERAL_CONJ2(C, A, B, bs, ldc, lda, N, ldb)                                                                                \
    for (ALPHA_INT __i = 0; __i < bs; __i++)                                                                                                           \
    {                                                                                                                                                \
        for (ALPHA_INT j = 0; j < N; j++)                                                                                                              \
        {                                                                                                                                            \
            for (ALPHA_INT m = 0; m < bs; m++)                                                                                                         \
            {                                                                                                                                        \
                ALPHA_Complex8 *tmp = C + __i * ldc + j;                                                                                               \
                tmp->real += (A)[__i * lda + m + 0].real * (B)[(m + 0) * ldb + j].real + (A)[__i * lda + m + 0].imag * (B)[(m + 0) * ldb + j].imag;  \
                tmp->imag += -(A)[__i * lda + m + 0].imag * (B)[(m + 0) * ldb + j].real + (A)[__i * lda + m + 0].real * (B)[(m + 0) * ldb + j].imag; \
            }                                                                                                                                        \
        }                                                                                                                                            \
    }
#endif
#ifdef CMLA
#define BLOCK_C_DGEMM_CR_GENERAL_CONJ2(C, A, B, bs, ldc, lda, N, ldb)                                                                                    \
    {                                                                                                                                                    \
        float32x4_t z_v_0, z_v_1, z_v_2, z_v_3;                                                                                                          \
        float32x4_t x_v_0;                                                                                                                               \
        float32x4_t y_v_0, y_v_1, y_v_2, y_v_3;                                                                                                          \
        for (ALPHA_INT __i = 0; __i < (bs); __i++)                                                                                                         \
        {                                                                                                                                                \
            for (ALPHA_INT k = 0; k < (bs); k++)                                                                                                           \
            {                                                                                                                                            \
                const ALPHA_Complex8 *X = (A) + k * (lda) + __i;                                                                                           \
                x_v_0 = vreinterpretq_f32_f64(vld1q_dup_f64((void *)X));                                                                                 \
                ALPHA_INT j = 0;                                                                                                                           \
                if (iszero(X[0]))                                                                                                                        \
                    continue;                                                                                                                            \
                for (; j < (N)-7; j += 8)                                                                                                                \
                {                                                                                                                                        \
                    ALPHA_Complex8 *__C = (C) + __i * (ldc) + j;                                                                                           \
                    const ALPHA_Complex8 *__B = (B) + k * (ldb) + j;                                                                                       \
                    z_v_0 = vld1q_f32((void *)(__C + 0));                                                                                                \
                    z_v_1 = vld1q_f32((void *)(__C + 2));                                                                                                \
                    z_v_2 = vld1q_f32((void *)(__C + 4));                                                                                                \
                    z_v_3 = vld1q_f32((void *)(__C + 6));                                                                                                \
                    y_v_0 = vld1q_f32((void *)(__B + 0));                                                                                                \
                    y_v_1 = vld1q_f32((void *)(__B + 2));                                                                                                \
                    y_v_2 = vld1q_f32((void *)(__B + 4));                                                                                                \
                    y_v_3 = vld1q_f32((void *)(__B + 6));                                                                                                \
                    z_v_0 = vcmlaq_f32(z_v_0, x_v_0, y_v_0);                                                                                             \
                    z_v_1 = vcmlaq_f32(z_v_1, x_v_0, y_v_1);                                                                                             \
                    z_v_2 = vcmlaq_f32(z_v_2, x_v_0, y_v_2);                                                                                             \
                    z_v_3 = vcmlaq_f32(z_v_3, x_v_0, y_v_3);                                                                                             \
                    z_v_0 = vcmlaq_rot270_f32(z_v_0, x_v_0, y_v_0);                                                                                      \
                    z_v_1 = vcmlaq_rot270_f32(z_v_1, x_v_0, y_v_1);                                                                                      \
                    z_v_2 = vcmlaq_rot270_f32(z_v_2, x_v_0, y_v_2);                                                                                      \
                    z_v_3 = vcmlaq_rot270_f32(z_v_3, x_v_0, y_v_3);                                                                                      \
                    vst1q_f32((void *)(__C + 0), z_v_0);                                                                                                 \
                    vst1q_f32((void *)(__C + 2), z_v_1);                                                                                                 \
                    vst1q_f32((void *)(__C + 4), z_v_2);                                                                                                 \
                    vst1q_f32((void *)(__C + 6), z_v_3);                                                                                                 \
                }                                                                                                                                        \
                for (; j < (N)-1; j += 2)                                                                                                                \
                {                                                                                                                                        \
                    ALPHA_Complex8 *__C = (C) + __i * (ldc) + j;                                                                                           \
                    const ALPHA_Complex8 *__B = (B) + k * (ldb) + j;                                                                                       \
                    z_v_0 = vld1q_f32((void *)__C);                                                                                                      \
                    y_v_0 = vld1q_f32((void *)__B);                                                                                                      \
                    z_v_0 = vcmlaq_f32(z_v_0, x_v_0, y_v_0);                                                                                             \
                    z_v_0 = vcmlaq_rot270_f32(z_v_0, x_v_0, y_v_0);                                                                                      \
                    vst1q_f32((void *)__C, z_v_0);                                                                                                       \
                }                                                                                                                                        \
                for (; j < (N); j++)                                                                                                                     \
                {                                                                                                                                        \
                    ALPHA_Complex8 *tmp = C + __i * ldc + j;                                                                                               \
                    tmp->real += (A)[k * lda + __i + 0].real * (B)[(k + 0) * ldb + j].real + (A)[k * lda + __i + 0].imag * (B)[(k + 0) * ldb + j].imag;  \
                    tmp->imag += -(A)[k * lda + __i + 0].imag * (B)[(k + 0) * ldb + j].real + (A)[k * lda + __i + 0].real * (B)[(k + 0) * ldb + j].imag; \
                }                                                                                                                                        \
            }                                                                                                                                            \
        }                                                                                                                                                \
    }
#elif defined(__aarch64__)
#define BLOCK_C_DGEMM_CR_GENERAL_CONJ2(C, A, B, bs, ldc, lda, N, ldb)                                                                                \
    for (ALPHA_INT __i = 0; __i < (bs); __i++)                                                                                                         \
    {                                                                                                                                                \
        for (ALPHA_INT k = 0; k < (bs); k++)                                                                                                           \
        {                                                                                                                                            \
            float32x4x2_t z_v_0, z_v_1;                                                                                                              \
            float32x4x2_t y_v_0, y_v_1;                                                                                                              \
            const ALPHA_Complex8 *__A = (A) + k * (lda) + __i;                                                                                         \
            float32x4_t x_r_0 = vdupq_n_f32(__A[0].real);                                                                                            \
            float32x4_t x_i_0 = vdupq_n_f32(-__A[0].imag);                                                                                           \
            float32x4_t xr_yr0, xr_yi0, xi_yr0, xi_yi0, res_r0, res_i0,                                                                              \
                xr_yr1, xr_yi1, xi_yr1, xi_yi1, res_r1, res_i1;                                                                                      \
            ALPHA_INT j = 0;                                                                                                                           \
            if (iszero(__A[0]))                                                                                                                      \
                continue;                                                                                                                            \
            for (; j < (N)-7; j += 8)                                                                                                                \
            {                                                                                                                                        \
                ALPHA_Complex8 *__C = (C) + __i * (ldc) + j;                                                                                           \
                const ALPHA_Complex8 *__B = (B) + k * (ldb) + j;                                                                                       \
                z_v_0 = vld2q_f32((float *)((__C) + 0));                                                                                             \
                z_v_1 = vld2q_f32((float *)((__C) + 4));                                                                                             \
                y_v_0 = vld2q_f32((float *)((__B) + 0));                                                                                             \
                y_v_1 = vld2q_f32((float *)((__B) + 4));                                                                                             \
                xr_yr0 = vmulq_f32(x_r_0, y_v_0.val[0]);                                                                                             \
                xr_yi0 = vmulq_f32(x_r_0, y_v_0.val[1]);                                                                                             \
                xr_yr1 = vmulq_f32(x_r_0, y_v_1.val[0]);                                                                                             \
                xr_yi1 = vmulq_f32(x_r_0, y_v_1.val[1]);                                                                                             \
                res_r0 = vfmsq_f32(xr_yr0, x_i_0, y_v_0.val[1]);                                                                                     \
                res_i0 = vfmaq_f32(xr_yi0, x_i_0, y_v_0.val[0]);                                                                                     \
                res_r1 = vfmsq_f32(xr_yr1, x_i_0, y_v_1.val[1]);                                                                                     \
                res_i1 = vfmaq_f32(xr_yi1, x_i_0, y_v_1.val[0]);                                                                                     \
                z_v_0.val[0] = vaddq_f32(z_v_0.val[0], res_r0);                                                                                      \
                z_v_0.val[1] = vaddq_f32(z_v_0.val[1], res_i0);                                                                                      \
                z_v_1.val[0] = vaddq_f32(z_v_1.val[0], res_r1);                                                                                      \
                z_v_1.val[1] = vaddq_f32(z_v_1.val[1], res_i1);                                                                                      \
                vst2q_f32((float *)((__C) + 0), z_v_0);                                                                                              \
                vst2q_f32((float *)((__C) + 4), z_v_1);                                                                                              \
            }                                                                                                                                        \
            for (; j < (N)-3; j += 4)                                                                                                                \
            {                                                                                                                                        \
                ALPHA_Complex8 *__C = (C) + __i * (ldc) + j;                                                                                           \
                const ALPHA_Complex8 *__B = (B) + k * (ldb) + j;                                                                                       \
                z_v_0 = vld2q_f32((float *)((__C) + 0));                                                                                             \
                y_v_0 = vld2q_f32((float *)((__B) + 0));                                                                                             \
                xr_yr0 = vmulq_f32(x_r_0, y_v_0.val[0]);                                                                                             \
                xr_yi0 = vmulq_f32(x_r_0, y_v_0.val[1]);                                                                                             \
                res_r0 = vfmsq_f32(xr_yr0, x_i_0, y_v_0.val[1]);                                                                                     \
                res_i0 = vfmaq_f32(xr_yi0, x_i_0, y_v_0.val[0]);                                                                                     \
                z_v_0.val[0] = vaddq_f32(z_v_0.val[0], res_r0);                                                                                      \
                z_v_0.val[1] = vaddq_f32(z_v_0.val[1], res_i0);                                                                                      \
                vst2q_f32((float *)((__C) + 0), z_v_0);                                                                                              \
            }                                                                                                                                        \
            for (; j < (N); j++)                                                                                                                     \
            {                                                                                                                                        \
                ALPHA_Complex8 *tmp = C + __i * ldc + j;                                                                                               \
                tmp->real += (A)[k * lda + __i + 0].real * (B)[(k + 0) * ldb + j].real + (A)[k * lda + __i + 0].imag * (B)[(k + 0) * ldb + j].imag;  \
                tmp->imag += -(A)[k * lda + __i + 0].imag * (B)[(k + 0) * ldb + j].real + (A)[k * lda + __i + 0].real * (B)[(k + 0) * ldb + j].imag; \
            }                                                                                                                                        \
        }                                                                                                                                            \
    }
#else
#define BLOCK_C_DGEMM_CR_GENERAL_CONJ2(C, A, B, bs, ldc, lda, N, ldb)                                                                                \
    for (ALPHA_INT __i = 0; __i < bs; __i++)                                                                                                           \
    {                                                                                                                                                \
        for (ALPHA_INT m = 0; m < bs; m++)                                                                                                             \
        {                                                                                                                                            \
            for (ALPHA_INT j = 0; j < N; j++)                                                                                                          \
            {                                                                                                                                        \
                ALPHA_Complex8 *tmp = C + __i * ldc + j;                                                                                               \
                tmp->real += (A)[m * lda + __i + 0].real * (B)[(m + 0) * ldb + j].real + (A)[m * lda + __i + 0].imag * (B)[(m + 0) * ldb + j].imag;  \
                tmp->imag += -(A)[m * lda + __i + 0].imag * (B)[(m + 0) * ldb + j].real + (A)[m * lda + __i + 0].real * (B)[(m + 0) * ldb + j].imag; \
            }                                                                                                                                        \
        }                                                                                                                                            \
    }
#endif
