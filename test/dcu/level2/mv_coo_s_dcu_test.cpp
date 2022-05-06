/**
 * @brief ict dcu mv hyb test
 * @author HPCRC, ICT
 */

#include <hip/hip_runtime_api.h>
#include <rocsparse.h>
#include <stdio.h>
#include <stdlib.h>

#include <iomanip>
#include <iostream>
#include <vector>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <alphasparse_dcu.h>

const char *file;
int thread_num;
bool check;
int iter;

alphasparse_operation_t transA;
alphasparse_layout_t layout;
struct alpha_matrix_descr descr;

// coo format
ALPHA_INT rm, rk, rnnz;
ALPHA_INT *row_index, *col_index;
float *values;

// parms for kernel
float *x;
float *icty;
float *rocy;
ALPHA_INT sizex, sizey;
const float alpha = 2.f;
const float beta  = 3.f;

const ALPHA_INT warm_up = 5;
const ALPHA_INT trials  = 10;
const int batch_size    = 1;

static void roc_mv_dcu()
{
    // rocSPARSE handle
    rocsparse_handle handle;
    rocsparse_create_handle(&handle);

    hipDeviceProp_t devProp;
    int device_id = 0;

    hipGetDevice(&device_id);
    hipGetDeviceProperties(&devProp, device_id);
    // std::cout << "Device: " << devProp.name << std::endl;

    rocsparse_int m   = rm;
    rocsparse_int n   = rk;
    rocsparse_int nnz = rnnz;

    // Generate problem
    std::vector<rocsparse_int> hArow(nnz);
    std::vector<rocsparse_int> hAcol(nnz);
    std::vector<float> hAval(nnz);
    for (int i = 0; i < nnz; i++) {
        hArow[i] = row_index[i];
        hAcol[i] = col_index[i];
        hAval[i] = values[i];
    }

    // Offload data to device
    rocsparse_int *dArow = NULL;
    rocsparse_int *dAcol = NULL;
    float *dAval         = NULL;
    float *dx            = NULL;
    float *dy            = NULL;

    hipMalloc((void **)&dArow, sizeof(rocsparse_int) * nnz);
    hipMalloc((void **)&dAcol, sizeof(rocsparse_int) * nnz);
    hipMalloc((void **)&dAval, sizeof(float) * nnz);
    hipMalloc((void **)&dx, sizeof(float) * sizex);
    hipMalloc((void **)&dy, sizeof(float) * sizey);

    hipMemcpy(dArow, hArow.data(), sizeof(rocsparse_int) * nnz, hipMemcpyHostToDevice);
    hipMemcpy(dAcol, hAcol.data(), sizeof(rocsparse_int) * nnz, hipMemcpyHostToDevice);
    hipMemcpy(dAval, hAval.data(), sizeof(float) * nnz, hipMemcpyHostToDevice);
    hipMemcpy(dx, x, sizeof(float) * sizex, hipMemcpyHostToDevice);
    hipMemcpy(dy, rocy, sizeof(float) * sizey, hipMemcpyHostToDevice);

    float halpha = alpha;
    float hbeta  = beta;

    // Matrix descriptor
    rocsparse_mat_descr descrA;
    rocsparse_create_mat_descr(&descrA);

    // Warm up
    for (int i = 0; i < warm_up; ++i) {
        // Call rocsparse hybmv
        roc_call_exit(
            rocsparse_scoomv(handle, rocsparse_operation_none, m, n, nnz, &halpha, descrA, dAval, dArow, dAcol, dx, &hbeta, dy),
            "rocsparse_scoomv");
    }

    // Device synchronization
    hipDeviceSynchronize();

    // Start time measurement
    double time = get_time_us();

    // CSR matrix vector multiplication
    for (int i = 0; i < trials; ++i) {
        for (int i = 0; i < batch_size; ++i) {
            // Call rocsparse hybmv
            roc_call_exit(
                rocsparse_scoomv(handle, rocsparse_operation_none, m, n, nnz, &halpha, descrA, dAval, dArow, dAcol, dx, &hbeta, dy),
                "rocsparse_scoomv");
        }

        // Device synchronization
        hipDeviceSynchronize();
    }

    time = (get_time_us() - time) / (trials * batch_size * 1e3);
    std::cout << time << std::endl;

    hipMemcpy(rocy, dy, sizeof(float) * sizey, hipMemcpyDeviceToHost);

    // Clear up on device
    hipFree(dArow);
    hipFree(dAcol);
    hipFree(dAval);
    hipFree(dx);
    hipFree(dy);

    rocsparse_destroy_mat_descr(descrA);
    rocsparse_destroy_handle(handle);
}

static void alpha_mv_dcu()
{
    // rocSPARSE handle
    alphasparse_dcu_handle_t handle;
    init_handle(&handle);
    alphasparse_dcu_get_handle(&handle);

    hipDeviceProp_t devProp;
    int device_id = 0;

    hipGetDevice(&device_id);
    hipGetDeviceProperties(&devProp, device_id);
    // std::cout << "Device: " << devProp.name << std::endl;

    // Generate problem
    ALPHA_INT m   = rm;
    ALPHA_INT n   = rk;
    ALPHA_INT nnz = rnnz;

    ALPHA_INT *hArow = (ALPHA_INT *)alpha_malloc(sizeof(ALPHA_INT) * nnz);
    ALPHA_INT *hAcol = (ALPHA_INT *)alpha_malloc(sizeof(ALPHA_INT) * nnz);
    float *hAval     = (float *)alpha_malloc(sizeof(float) * nnz);

    for (int i = 0; i < nnz; i++) {
        hAcol[i] = col_index[i];
        hArow[i] = row_index[i];
        hAval[i] = values[i];
    }

    // Offload data to device
    ALPHA_INT *dArow = NULL;
    ALPHA_INT *dAcol = NULL;
    float *dAval     = NULL;
    float *dx        = NULL;
    float *dy        = NULL;

    PRINT_IF_HIP_ERROR(hipMalloc((void **)&dArow, sizeof(ALPHA_INT) * nnz));
    PRINT_IF_HIP_ERROR(hipMalloc((void **)&dAcol, sizeof(ALPHA_INT) * nnz));
    PRINT_IF_HIP_ERROR(hipMalloc((void **)&dAval, sizeof(float) * nnz));
    PRINT_IF_HIP_ERROR(hipMalloc((void **)&dx, sizeof(float) * n));
    PRINT_IF_HIP_ERROR(hipMalloc((void **)&dy, sizeof(float) * m));

    PRINT_IF_HIP_ERROR(
        hipMemcpy(dArow, hArow, sizeof(ALPHA_INT) * nnz, hipMemcpyHostToDevice));
    PRINT_IF_HIP_ERROR(
        hipMemcpy(dAcol, hAcol, sizeof(ALPHA_INT) * nnz, hipMemcpyHostToDevice));
    PRINT_IF_HIP_ERROR(
        hipMemcpy(dAval, hAval, sizeof(float) * nnz, hipMemcpyHostToDevice));
    PRINT_IF_HIP_ERROR(
        hipMemcpy(dx, x, sizeof(float) * sizex, hipMemcpyHostToDevice));
    PRINT_IF_HIP_ERROR(
        hipMemcpy(dy, icty, sizeof(float) * sizey, hipMemcpyHostToDevice));

    float halpha = alpha;
    float hbeta  = beta;

    // Matrix descriptor
    alpha_dcu_matrix_descr_t descrA;
    alphasparse_dcu_create_mat_descr(&descrA);

    // Warm up
    for (int i = 0; i < warm_up; ++i) {
        // Call alphasparse_dcu coomv
        alpha_call_exit(alphasparse_dcu_s_coomv(
                            handle, ALPHA_SPARSE_OPERATION_NON_TRANSPOSE, m, n, nnz, &halpha, descrA, dAval, dArow, dAcol, dx, &hbeta, dy),
                        "alphasparse_dcu_s_coomv");
    }

    // Device synchronization
    hipDeviceSynchronize();

    // Start time measurement
    double time = get_time_us();

    // COO matrix vector multiplication
    for (int i = 0; i < trials; ++i) {
        for (int i = 0; i < batch_size; ++i) {
            // Call alphasparse_dcu coomv
            alpha_call_exit(alphasparse_dcu_s_coomv(
                                handle, ALPHA_SPARSE_OPERATION_NON_TRANSPOSE, m, n, nnz, &halpha, descrA, dAval, dArow, dAcol, dx, &hbeta, dy),
                            "alphasparse_dcu_s_coomv");
        }
        // Device synchronization
        hipDeviceSynchronize();
    }

    time = (get_time_us() - time) / (trials * batch_size * 1e3);
    std::cout << time << ",";

    hipMemcpy(icty, dy, sizeof(float) * sizey, hipMemcpyDeviceToHost);

    // Clear up on device
    hipFree(dArow);
    hipFree(dAcol);
    hipFree(dAval);
    hipFree(dx);
    hipFree(dy);

    alphasparse_dcu_destroy_mat_descr(descrA);
    alphasparse_dcu_destory_handle(handle);
}

int main(int argc, const char *argv[])
{
    // args
    args_help(argc, argv);
    file       = args_get_data_file(argc, argv);
    thread_num = args_get_thread_num(argc, argv);
    check      = args_get_if_check(argc, argv);
    transA     = alpha_args_get_transA(argc, argv);
    descr      = alpha_args_get_matrix_descrA(argc, argv);

    // read coo
    alpha_read_coo(file, &rm, &rk, &rnnz, &row_index, &col_index, &values);

    sizex = rk, sizey = rm;
    if (transA == ALPHA_SPARSE_OPERATION_TRANSPOSE ||
        transA == ALPHA_SPARSE_OPERATION_CONJUGATE_TRANSPOSE) {
        sizex = rm;
        sizey = rk;
    }
    // init x y
    x    = (float *)alpha_malloc(sizex * sizeof(float));
    icty = (float *)alpha_malloc(sizey * sizeof(float));
    rocy = (float *)alpha_malloc(sizey * sizeof(float));

    alpha_fill_random_s(x, 0, sizex);
    alpha_fill_random_s(icty, 1, sizey);
    alpha_fill_random_s(rocy, 1, sizey);

    alpha_mv_dcu();

    if (check) {
        roc_mv_dcu();
        check_s((float *)rocy, sizey, (float *)icty, sizey);
    }
    printf("\n");
    alpha_free(x);
    alpha_free(icty);
    alpha_free(rocy);

    alpha_free(row_index);
    alpha_free(col_index);
    alpha_free(values);
    return 0;
}

#ifdef __cplusplus
}
#endif /*__cplusplus */
