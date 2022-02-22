/**
 * @brief ictt mv csr test
 * @author Zhuoqiang Guo <gzq9425@qq.com>
 */

#include <alphasparse.h>
#include <stdio.h>

const char *file;
int thread_num;
bool check;
int iter;

alphasparse_operation_t transA;
alphasparse_layout_t layout;
struct alpha_matrix_descr descr;

ALPHA_INT m, k, nnz;
ALPHA_INT *row_index, *col_index;
float *values;
const float alpha = 2.f;
const float beta = 2.f;

ALPHA_INT ldx, ldy;

float *x;
float *icty;
float *icty_plain;

ALPHA_INT lo, diag, hi;
ALPHA_INT64 ops;

static void alpha_mv()
{

    alphasparse_matrix_t cooA, csrA;
    alpha_call_exit(alphasparse_s_create_coo(&cooA, ALPHA_SPARSE_INDEX_BASE_ZERO, m, k, nnz, row_index, col_index, values), "alphasparse_s_create_coo");
    alpha_call_exit(alphasparse_convert_csr(cooA, ALPHA_SPARSE_OPERATION_NON_TRANSPOSE, &csrA), "alphasparse_convert_csr");

    alpha_timer_t timer;
    double total_time = 0.;

    // alpha_clear_cache();
    for (int i = 0; i < iter; i++)
    {
        // alpha_clear_cache();
        alpha_timing_start(&timer);
        alpha_call_exit(alphasparse_s_mv(transA, alpha, csrA, descr, x, beta, icty), "alphasparse_s_mv");
        alpha_timing_end(&timer);
        total_time += alpha_timing_elapsed_time(&timer);
    }

    printf("%s time : %lf[sec]\n","alphasparse_s_mv",(total_time/iter));

    alphasparse_destroy(cooA);
    alphasparse_destroy(csrA);
}

static void alpha_mv_plain()
{
    alpha_set_thread_num(thread_num);

    alphasparse_matrix_t cooA, csrA;
    alpha_call_exit(alphasparse_s_create_coo(&cooA, ALPHA_SPARSE_INDEX_BASE_ZERO, m, k, nnz, row_index, col_index, values), "alphasparse_s_create_coo");
    alpha_call_exit(alphasparse_convert_csr(cooA, ALPHA_SPARSE_OPERATION_NON_TRANSPOSE, &csrA), "alphasparse_convert_csr");

    // alpha_clear_cache();
    alpha_timer_t timer;
    double total_time = 0.;
    for (int i = 0; i < iter; i++)
    {
        // alpha_clear_cache();
        alpha_timing_start(&timer);
        alpha_call_exit(alphasparse_s_mv_plain(transA, alpha, csrA, descr, x, beta, icty_plain), "alphasparse_s_mv_plain");
        alpha_timing_end(&timer);
        total_time += alpha_timing_elapsed_time(&timer);
    }
    printf("%s time : %lf[sec]\n","alphasparse_s_mv_plain",(total_time/iter));

    alphasparse_destroy(cooA);
    alphasparse_destroy(csrA);
}

int main(int argc, const char *argv[])
{
    // args
    args_help(argc, argv);
    file = args_get_data_file(argc, argv);
    thread_num = args_get_thread_num(argc, argv);
    check = args_get_if_check(argc, argv);
    iter = args_get_iter(argc, argv);
    transA = alpha_args_get_transA(argc, argv);
    descr = alpha_args_get_matrix_descrA(argc, argv);
    alpha_set_thread_num(thread_num);
    printf("%d\n", thread_num);

    // read coo
    alpha_read_coo(file, &m, &k, &nnz, &row_index, &col_index, &values);

    alphasparse_nnz_counter_coo(row_index, col_index, nnz, &lo, &diag, &hi);
    ops = alphasparse_operations_mv(m, k, lo, diag, hi, transA, descr, ALPHA_SPARSE_DATATYPE_FLOAT_COMPLEX);

    ALPHA_INT sizex = k,sizey=m;
    if(transA == ALPHA_SPARSE_OPERATION_TRANSPOSE || transA == ALPHA_SPARSE_OPERATION_CONJUGATE_TRANSPOSE){
        sizex = m;
        sizey = k;
    }
    // init x y
    x = alpha_malloc(sizex * sizeof(float));
    icty = alpha_malloc(sizey * sizeof(float));
    icty_plain = alpha_malloc(sizey * sizeof(float));

    alpha_fill_random_s(x, 0, sizex);
    alpha_fill_random_s(icty, 1, sizey);

    alpha_mv();

    int status = 0;
    if (check)
    {
        alpha_fill_random_s(icty_plain, 1, sizey);
        alpha_mv_plain();
        status = check_s_l2((float*)icty, sizey, icty_plain, sizey, x, NULL, alpha, beta, argc, argv);
        // status = check_s(icty, sizey, icty_plain, sizey);
    }
    printf("\n");
    alpha_free(x);
    alpha_free(icty);
    alpha_free(icty_plain);

    alpha_free(row_index);
    alpha_free(col_index);
    alpha_free(values);
    return status;
}
