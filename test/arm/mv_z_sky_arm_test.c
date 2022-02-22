/**
 * @brief openspblas mv sky test
 * @author Zhuoqiang Guo <gzq9425@qq.com>
 */

#include <alphasparse.h>
#include <stdio.h>

static void alpha_mv(const int argc, const char *argv[],  ALPHA_INT m,  ALPHA_INT n,  ALPHA_INT nnz,  ALPHA_INT *row_index,  ALPHA_INT *col_index,  ALPHA_Complex16 *values,  ALPHA_Complex16 *x,  ALPHA_Complex16 alpha,  ALPHA_Complex16 beta, ALPHA_Complex16 *y, ALPHA_INT thread_num)
{
    alpha_set_thread_num(thread_num);

    alphasparse_operation_t transA = alpha_args_get_transA(argc, argv);
    struct alpha_matrix_descr descr = alpha_args_get_matrix_descrA(argc, argv);

    alphasparse_matrix_t cooA, skyA;
    alpha_call_exit(alphasparse_z_create_coo(&cooA, ALPHA_SPARSE_INDEX_BASE_ZERO, m, n, nnz, row_index, col_index, values), "alphasparse_z_create_coo");
    alpha_call_exit(alphasparse_convert_sky(cooA, ALPHA_SPARSE_OPERATION_NON_TRANSPOSE, descr.mode, &skyA), "alphasparse_convert_sky");

    alpha_timer_t timer;
    alpha_timing_start(&timer);

    alpha_call_exit(alphasparse_z_mv(transA, alpha, skyA, descr, x, beta, y), "alphasparse_z_mv");

    alpha_timing_end(&timer);
    alpha_timing_elaped_time_print(&timer, "openspblas mv _z_ sky time");
    alphasparse_destroy(cooA);
    alphasparse_destroy(skyA);
}

static void alpha_mv_plain(const int argc, const char *argv[],  ALPHA_INT m,  ALPHA_INT n,  ALPHA_INT nnz,  ALPHA_INT *row_index,  ALPHA_INT *col_index,  ALPHA_Complex16 *values,  ALPHA_Complex16 *x,  ALPHA_Complex16 alpha,  ALPHA_Complex16 beta, ALPHA_Complex16 *y, ALPHA_INT thread_num)
{
    alpha_set_thread_num(thread_num);

    alphasparse_operation_t transA = alpha_args_get_transA(argc, argv);
    struct alpha_matrix_descr descr = alpha_args_get_matrix_descrA(argc, argv);

    alphasparse_matrix_t cooA, skyA;
    alpha_call_exit(alphasparse_z_create_coo(&cooA, ALPHA_SPARSE_INDEX_BASE_ZERO, m, n, nnz, row_index, col_index, values), "alphasparse_z_create_coo");
    alpha_call_exit(alphasparse_convert_sky(cooA, ALPHA_SPARSE_OPERATION_NON_TRANSPOSE, descr.mode, &skyA), "alphasparse_convert_sky");

    alpha_timer_t timer;
    alpha_timing_start(&timer);

    alpha_call_exit(alphasparse_z_mv_plain(transA, alpha, skyA, descr, x, beta, y), "alphasparse_z_mv_plain");

    alpha_timing_end(&timer);

    alpha_timing_elaped_time_print(&timer, "alphasparse_z_mv_plain");

    alphasparse_destroy(cooA);
    alphasparse_destroy(skyA);
}

int main(int argc,const char *argv[])
{
    // args
    args_help(argc, argv);
    const char *file = args_get_data_file(argc, argv);
    int thread_num = args_get_thread_num(argc, argv);
    bool check = args_get_if_check(argc, argv);

    ALPHA_INT m, k, nnz;
    ALPHA_INT *row_index, *col_index;
    ALPHA_Complex16 *values;
    const ALPHA_Complex16 alpha = {.real = 3., .imag = 3.};
    const ALPHA_Complex16 beta = {.real = 2., .imag = 2.};
    // read coo
    alpha_read_coo_z(file, &m, &k, &nnz, &row_index, &col_index, &values);

    alphasparse_operation_t transA = alpha_args_get_transA(argc, argv);
    ALPHA_INT sizex = k,sizey = m;
    if(transA == ALPHA_SPARSE_OPERATION_TRANSPOSE || transA == ALPHA_SPARSE_OPERATION_CONJUGATE_TRANSPOSE){
        sizex = m;
        sizey = k;
    }
    // init x y
    ALPHA_Complex16 *x = alpha_malloc(sizex * sizeof(ALPHA_Complex16));
    ALPHA_Complex16 *alpha_y = alpha_malloc(sizey * sizeof(ALPHA_Complex16));
    ALPHA_Complex16 *alpha_y_plain = alpha_malloc(sizey * sizeof(ALPHA_Complex16));

    alpha_fill_random_z(x, 0, sizex);
    alpha_fill_random_z(alpha_y, 1, sizey);

    printf("thread_num : %d\n", thread_num);

    alpha_mv(argc, argv, m, k, nnz, row_index, col_index, values, x, alpha, beta, alpha_y, thread_num);

    int status = 0;
    if (check)
    {
        alpha_fill_random_z(alpha_y_plain, 1, sizey);
        alpha_mv_plain(argc, argv, m, k, nnz, row_index, col_index, values, x, alpha, beta, alpha_y_plain, thread_num);
        status = check_z(alpha_y, sizey, alpha_y_plain, sizey);
    }

    alpha_free(x);
    alpha_free(alpha_y);
    alpha_free(alpha_y_plain);

    alpha_free(row_index);
    alpha_free(col_index);
    alpha_free(values);
    return status;
}