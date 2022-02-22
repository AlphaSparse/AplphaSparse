/**
 * @brief openspblas trsv csr test
 * @author Zhuoqiang Guo <gzq9425@qq.com>
 */

#include <alphasparse.h>
#include <stdio.h>
#include <mkl.h>

static void mkl_trsv(const int argc, const char *argv[], const char *file, int thread_num, const double alpha, double **ret, size_t *size)
{
    MKL_INT m, k, nnz;
    MKL_INT *row_index, *col_index;
    double *values;
    mkl_read_coo_d(file, &m, &k, &nnz, &row_index, &col_index, &values);
    if (m != k)
    {
        printf("sparse matrix must be Square matrix but (%d,%d)\n", (int)m, (int)k);
        exit(-1);
    }
    size_t size_x = k;
    size_t size_y = m;
    double *x = alpha_memalign(sizeof(double) * size_x, DEFAULT_ALIGNMENT);
    double *y = alpha_memalign(sizeof(double) * size_y, DEFAULT_ALIGNMENT);
    alpha_fill_random_d(x, 1, size_x);
    for(int i = 0; i < nnz; i++)
    {
        if(row_index[i] == col_index[i])
        {
            values[i] += 1.0;
        }
    }

    mkl_set_num_threads(thread_num);

    sparse_operation_t transA = mkl_args_get_transA(argc, argv);
    struct matrix_descr descr = mkl_args_get_matrix_descrA(argc, argv);

    sparse_matrix_t cooA, csrA;
    mkl_sparse_d_create_coo(&cooA, SPARSE_INDEX_BASE_ZERO, m, k, nnz, row_index, col_index, values);
    mkl_sparse_convert_csr(cooA, SPARSE_OPERATION_NON_TRANSPOSE, &csrA);
    alpha_timer_t timer;
    alpha_timing_start(&timer);
    mkl_call_exit(mkl_sparse_d_trsv(transA, alpha, csrA, descr, x, y), "mkl_sparse_d_trsv");
    alpha_timing_end(&timer);
    alpha_timing_elaped_time_print(&timer, "mkl_sparse_d_trsv");
    mkl_sparse_destroy(cooA);
    mkl_sparse_destroy(csrA);

    *ret = y;
    *size = size_y;
    alpha_free(row_index);
    alpha_free(col_index);
    alpha_free(values);
}

static void alpha_trsv(const int argc, const char *argv[], const char *file, int thread_num, const double alpha, double **ret, size_t *size)
{
    ALPHA_INT m, k, nnz;
    ALPHA_INT *row_index, *col_index;
    double *values;
    alpha_read_coo_d(file, &m, &k, &nnz, &row_index, &col_index, &values);
    if (m != k)
    {
        printf("sparse matrix must be Square matrix but (%d,%d)\n", (int)m, (int)k);
        exit(-1);
    }
    if (m != k)
    {
        printf("sparse matrix must be Square matrix but (%d,%d)\n", (int)m, (int)k);
        exit(-1);
    }
    size_t size_x = k;
    size_t size_y = m;
    double *x = alpha_memalign(sizeof(double) * size_x, DEFAULT_ALIGNMENT);
    double *y = alpha_memalign(sizeof(double) * size_y, DEFAULT_ALIGNMENT);
    alpha_fill_random_d(x, 1, size_x);
    for(int i = 0; i < nnz; i++)
    {
        if(row_index[i] == col_index[i])
        {
            values[i] += 1.0;
        }
    }

    alpha_set_thread_num(thread_num);
    alphasparse_operation_t transA = alpha_args_get_transA(argc, argv);
    struct alpha_matrix_descr descr = alpha_args_get_matrix_descrA(argc, argv);

    alphasparse_matrix_t cooA, csrA;
    alpha_call_exit(alphasparse_d_create_coo(&cooA, ALPHA_SPARSE_INDEX_BASE_ZERO, m, k, nnz, row_index, col_index, values), "alphasparse_d_create_coo");
    alpha_call_exit(alphasparse_convert_dia(cooA, ALPHA_SPARSE_OPERATION_NON_TRANSPOSE, &csrA), "alphasparse_convert_dia");

    alpha_timer_t timer;
    alpha_timing_start(&timer);

    alpha_call_exit(alphasparse_d_trsv_plain(transA, alpha, csrA, descr, x, y), "alphasparse_d_trsv_plain");

    alpha_timing_end(&timer);
    alpha_timing_elaped_time_print(&timer, "alphasparse_d_trsv_plain");
    alphasparse_destroy(cooA);
    alphasparse_destroy(csrA);

    *ret = y;
    *size = size_y;
    alpha_free(row_index);
    alpha_free(col_index);
    alpha_free(values);
}

int main(int argc, const char *argv[])
{
    // args
    args_help(argc, argv);
    const char *file = args_get_data_file(argc, argv);
    int thread_num = args_get_thread_num(argc, argv);
    bool check = args_get_if_check(argc, argv);

    const double alpha = 2;

    printf("thread_num : %d\n", thread_num);

    double *alpha_y, *mkl_y;
    size_t size_alpha_y, size_mkl_y;

    alpha_trsv(argc, argv, file, thread_num, alpha, &alpha_y, &size_alpha_y);
    int status = 0;
    if (check)
    {
        mkl_trsv(argc, argv, file, thread_num, alpha, &mkl_y, &size_mkl_y);
        status = check_d(mkl_y, size_mkl_y, alpha_y, size_alpha_y);
        alpha_free(mkl_y);
    }

    alpha_free(alpha_y);

    return status;
}