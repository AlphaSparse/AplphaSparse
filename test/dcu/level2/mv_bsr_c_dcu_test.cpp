/**
 * @brief ict dcu mv bsr test
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

#include <alpha_spblas_dcu.h>

const char *file;
int thread_num;
bool check;
int iter;

alphasparse_operation_t transA;
alphasparse_layout_t layout;
struct alpha_matrix_descr descr;

// bsr format
ALPHA_INT rm, rk, nnzb, bs = 2;
ALPHA_INT *bsr_row_ptr, *bsr_row_ptr_end, *bsr_col_index;
ALPHA_Complex8 *bsr_values;
rocsparse_direction layout_roc;

// parms for kernel
ALPHA_Complex8 *x;
ALPHA_Complex8 *icty;
ALPHA_Complex8 *rocy;
ALPHA_INT sizex, sizey;
const ALPHA_Complex8 alpha = {2.f, 2.f};
const ALPHA_Complex8 beta  = {3.f, 3.f};

ALPHA_INT lo, diag, hi;
ALPHA_INT64 ops;

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
    rocsparse_int nnz = nnzb * bs * bs;
    rocsparse_int nnb = nnzb;

    // Generate problem
    std::vector<rocsparse_int> hAptr(m + 1);
    std::vector<rocsparse_int> hAcol(nnb);
    std::vector<ALPHA_Complex8> hAval(nnz);

    for (int i = 0; i < m; i++)
        hAptr[i] = bsr_row_ptr[i];

    hAptr[m] = bsr_row_ptr_end[m - 1];

    for (int i = 0; i < nnb; i++) {
        hAcol[i] = bsr_col_index[i];
    }
    for (int i = 0; i < nnz; i++) {
        hAval[i] = bsr_values[i];
    }

    // Offload data to device
    rocsparse_int *dAptr           = NULL;
    rocsparse_int *dAcol           = NULL;
    rocsparse_float_complex *dAval = NULL;
    rocsparse_float_complex *dx    = NULL;
    rocsparse_float_complex *dy    = NULL;

    hipMalloc((void **)&dAptr, sizeof(rocsparse_int) * (m + 1));
    hipMalloc((void **)&dAcol, sizeof(rocsparse_int) * nnb);
    hipMalloc((void **)&dAval, sizeof(rocsparse_float_complex) * nnz);
    hipMalloc((void **)&dx, sizeof(rocsparse_float_complex) * sizex);
    hipMalloc((void **)&dy, sizeof(rocsparse_float_complex) * sizey);

    hipMemcpy(dAptr, hAptr.data(), sizeof(rocsparse_int) * (m + 1), hipMemcpyHostToDevice);
    hipMemcpy(dAcol, hAcol.data(), sizeof(rocsparse_int) * nnb, hipMemcpyHostToDevice);
    hipMemcpy(dAval, hAval.data(), sizeof(rocsparse_float_complex) * nnz, hipMemcpyHostToDevice);
    hipMemcpy(dx, x, sizeof(rocsparse_float_complex) * sizex, hipMemcpyHostToDevice);
    hipMemcpy(dy, rocy, sizeof(rocsparse_float_complex) * sizey, hipMemcpyHostToDevice);

    rocsparse_float_complex halpha, hbeta;
    halpha.x = alpha.real;
    halpha.y = alpha.imag;
    hbeta.x  = beta.real;
    hbeta.y  = beta.imag;

    // Matrix descriptor
    rocsparse_mat_descr descrA;
    rocsparse_create_mat_descr(&descrA);

    // Warm up
    for (int i = 0; i < warm_up; ++i) {
        // Call rocsparse bsrmv
        rocsparse_cbsrmv(handle, layout_roc, rocsparse_operation_none, m, n, nnzb, &halpha, descrA, dAval, dAptr, dAcol, bs, dx, &hbeta, dy);
    }

    // Device synchronization
    hipDeviceSynchronize();

    // Start time measurement
    double time = get_time_us();

    // bsr matrix vector multiplication
    for (int i = 0; i < trials; ++i) {
        for (int i = 0; i < batch_size; ++i) {
            // Call rocsparse bsrmv
            roc_call_exit(
                rocsparse_cbsrmv(handle, layout_roc, rocsparse_operation_none, m, n, nnzb, &halpha, descrA, dAval, dAptr, dAcol, bs, dx, &hbeta, dy),
                "rocsparse_cbsrmv");
        }

        // Device synchronization
        hipDeviceSynchronize();
    }

    time = (get_time_us() - time) / (trials * batch_size * 1e3);
    std::cout << time << std::endl;

    hipMemcpy(rocy, dy, sizeof(ALPHA_Complex8) * sizey, hipMemcpyDeviceToHost);

    // Clear up on device
    hipFree(dAptr);
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
    ALPHA_INT nnz = nnzb * bs * bs;

    ALPHA_INT *hAptr      = (ALPHA_INT *)alpha_malloc(sizeof(ALPHA_INT) * (m + 1));
    ALPHA_INT *hAcol      = (ALPHA_INT *)alpha_malloc(sizeof(ALPHA_INT) * nnzb);
    ALPHA_Complex8 *hAval = (ALPHA_Complex8 *)alpha_malloc(sizeof(ALPHA_Complex8) * nnz);

    for (int i = 0; i < m; i++)
        hAptr[i] = bsr_row_ptr[i];

    hAptr[m] = bsr_row_ptr_end[m - 1];

    for (int i = 0; i < nnzb; i++) {
        hAcol[i] = bsr_col_index[i];
    }
    for (int i = 0; i < nnz; i++) {
        hAval[i] = bsr_values[i];
    }

    // Offload data to device
    ALPHA_INT *dAptr      = NULL;
    ALPHA_INT *dAcol      = NULL;
    ALPHA_Complex8 *dAval = NULL;
    ALPHA_Complex8 *dx    = NULL;
    ALPHA_Complex8 *dy    = NULL;

    PRINT_IF_HIP_ERROR(hipMalloc((void **)&dAptr, sizeof(ALPHA_INT) * (m + 1)));
    PRINT_IF_HIP_ERROR(hipMalloc((void **)&dAcol, sizeof(ALPHA_INT) * nnzb));
    PRINT_IF_HIP_ERROR(hipMalloc((void **)&dAval, sizeof(ALPHA_Complex8) * nnz));
    PRINT_IF_HIP_ERROR(hipMalloc((void **)&dx, sizeof(ALPHA_Complex8) * sizex));
    PRINT_IF_HIP_ERROR(hipMalloc((void **)&dy, sizeof(ALPHA_Complex8) * sizey));

    PRINT_IF_HIP_ERROR(hipMemcpy(dAptr, hAptr, sizeof(ALPHA_INT) * (m + 1), hipMemcpyHostToDevice));
    PRINT_IF_HIP_ERROR(
        hipMemcpy(dAcol, hAcol, sizeof(ALPHA_INT) * nnzb, hipMemcpyHostToDevice));
    PRINT_IF_HIP_ERROR(hipMemcpy(dAval, hAval, sizeof(ALPHA_Complex8) * nnz, hipMemcpyHostToDevice));
    PRINT_IF_HIP_ERROR(
        hipMemcpy(dx, x, sizeof(ALPHA_Complex8) * sizex, hipMemcpyHostToDevice));
    PRINT_IF_HIP_ERROR(
        hipMemcpy(dy, icty, sizeof(ALPHA_Complex8) * sizey, hipMemcpyHostToDevice));

    ALPHA_Complex8 halpha = alpha;
    ALPHA_Complex8 hbeta  = beta;

    // Matrix descriptor
    alpha_dcu_matrix_descr_t descrA;
    alphasparse_dcu_create_mat_descr(&descrA);

    // Warm up
    for (int i = 0; i < warm_up; ++i) {
        // Call alphasparse_dcu bsrmv
        alphasparse_dcu_c_bsrmv(handle, layout, ALPHA_SPARSE_OPERATION_NON_TRANSPOSE, m, n, nnzb, &halpha, descrA, dAval, dAptr, dAcol, bs, dx, &hbeta, dy);
    }

    // Device synchronization
    hipDeviceSynchronize();

    // Start time measurement
    double time = get_time_us();

    // bsr matrix vector multiplication
    for (int i = 0; i < trials; ++i) {
        for (int i = 0; i < batch_size; ++i) {
            // Call alphasparse_dcu bsrmv
            alphasparse_dcu_c_bsrmv(handle, layout, ALPHA_SPARSE_OPERATION_NON_TRANSPOSE, m, n, nnzb, &halpha, descrA, dAval, dAptr, dAcol, bs, dx, &hbeta, dy);
        }
        // Device synchronization
        hipDeviceSynchronize();
    }

    time = (get_time_us() - time) / (trials * batch_size * 1e3);
    std::cout << time << ",";

    hipMemcpy(icty, dy, sizeof(ALPHA_Complex8) * sizey, hipMemcpyDeviceToHost);

    // Clear up on device
    hipFree(dAptr);
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
    layout     = alpha_args_get_layout(argc, argv);

    alphasparse_index_base_t bsr_index;

    alphasparse_matrix_t coo, bsr;
    ALPHA_INT *coo_row_index, *coo_col_index;
    ALPHA_Complex8 *coo_values;
    // read coo
    alpha_read_coo_c(file, &rm, &rk, &nnzb, &coo_row_index, &coo_col_index, &coo_values);

    // 创建coo格式稀疏矩阵
    alpha_call_exit(
        alphasparse_c_create_coo(&coo, ALPHA_SPARSE_INDEX_BASE_ZERO, rm, rk, nnzb, coo_row_index, coo_col_index, coo_values),
        "alphasparse_c_create_coo");
    // 将稀疏矩阵从coo格式转换成bsr格式
    alpha_call_exit(alphasparse_convert_bsr(
                        coo, bs, layout, ALPHA_SPARSE_OPERATION_NON_TRANSPOSE, &bsr),
                    "alphasparse_convert_bsr");
    // 获取bsr格式里的数据
    alpha_call_exit(alphasparse_c_export_bsr(bsr, &bsr_index, &layout, &rm, &rk, &bs, &bsr_row_ptr, &bsr_row_ptr_end, &bsr_col_index, &bsr_values),
                    "alphasparse_c_export_bsr");

    nnzb  = bsr_row_ptr_end[rm - 1];
    sizex = rk * bs, sizey = rm * bs;
    // std::cout << nnzb * bs * bs << std::endl;
    if (transA == ALPHA_SPARSE_OPERATION_TRANSPOSE ||
        transA == ALPHA_SPARSE_OPERATION_CONJUGATE_TRANSPOSE) {
        sizex = rm * bs;
        sizey = rk * bs;
    }
    if (layout == ALPHA_SPARSE_LAYOUT_ROW_MAJOR) {
        layout_roc = rocsparse_direction_row;
    } else {
        layout_roc = rocsparse_direction_column;
    }

    // init x y
    x    = (ALPHA_Complex8 *)alpha_malloc(sizex * sizeof(ALPHA_Complex8));
    icty = (ALPHA_Complex8 *)alpha_malloc(sizey * sizeof(ALPHA_Complex8));
    rocy = (ALPHA_Complex8 *)alpha_malloc(sizey * sizeof(ALPHA_Complex8));

    alpha_fill_random_c(x, 0, sizex);
    alpha_fill_random_c(icty, 1, sizey);
    alpha_fill_random_c(rocy, 1, sizey);
    alpha_mv_dcu();

    if (check) {
        roc_mv_dcu();
        check_c((ALPHA_Complex8 *)rocy, sizey, (ALPHA_Complex8 *)icty, sizey);
    }
    printf("\n");

    alpha_free(x);
    alpha_free(icty);
    alpha_free(rocy);
    // alpha_free(bsr_row_ptr);
    // alpha_free(bsr_row_ptr_end);
    // alpha_free(bsr_col_index);
    alpha_free(coo_row_index);
    alpha_free(coo_col_index);
    alpha_free(coo_values);
    return 0;
}

#ifdef __cplusplus
}
#endif /*__cplusplus */
