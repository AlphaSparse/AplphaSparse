#include <alphasparse.h>
#include <stdio.h>
#include <mkl.h>

static void mkl_gthrz(const MKL_INT n, float * x, const MKL_INT* indx, float *y, int thread_num)
{
    mkl_set_num_threads(thread_num);
    alpha_timer_t timer;
    alpha_timing_start(&timer);

    cblas_sgthrz(n, y, x, indx);

    alpha_timing_end(&timer);
    alpha_timing_elaped_time_print(&timer, "cblas_sgthrz");
}

static void alpha_gthrz(const ALPHA_INT n, float * x, const ALPHA_INT *indx, float *y, int thread_num)
{
    alpha_set_thread_num(thread_num);
    alpha_timer_t timer;
    alpha_timing_start(&timer);

    alpha_call_exit(alphasparse_s_gthrz_plain(n, y, x, indx), "alpha_s_gthrz");

    alpha_timing_end(&timer);
    alpha_timing_elaped_time_print(&timer, "alpha_s_gthrz");
}

int main(int argc,const char *argv[])
{
    // args
    args_help(argc, argv);
    int thread_num = args_get_thread_num(argc, argv);
    bool check = args_get_if_check(argc, argv);

    const float alpha = 3.;

    // return
    MKL_INT mkl_n = 50;
    float a = 2.0f;
    ALPHA_INT alpha_n = 50;

    ALPHA_INT *alpha_incx = alpha_memalign(sizeof(ALPHA_INT) * alpha_n, DEFAULT_ALIGNMENT);
    MKL_INT *mkl_incx = alpha_memalign(sizeof(MKL_INT) * mkl_n, DEFAULT_ALIGNMENT);

    float *alpha_x = alpha_memalign(sizeof(float) * alpha_n, DEFAULT_ALIGNMENT);
    float *mkl_x = alpha_memalign(sizeof(float) * mkl_n, DEFAULT_ALIGNMENT);
    // alpha_fill_random_s(x, 1, alpha_n);

    for(ALPHA_INT i = 0; i < alpha_n; i++)
    {
        mkl_incx[i] = i * 20;
        alpha_incx[i] = i * 20;
    }

    float *alpha_y = alpha_memalign(sizeof(float) * alpha_n * 20, DEFAULT_ALIGNMENT);
    float *mkl_y = alpha_memalign(sizeof(float) * mkl_n * 20, DEFAULT_ALIGNMENT);

    alpha_fill_random_s(alpha_y, 1, alpha_n * 20);
    alpha_fill_random_s(mkl_y, 1, mkl_n * 20);
    
    alpha_gthrz(alpha_n, alpha_x, alpha_incx, alpha_y, thread_num);
    int status = 0;
    if (check)
    {
        mkl_gthrz(mkl_n, mkl_x, mkl_incx, mkl_y, thread_num);
        status = check_s(mkl_x, mkl_n, alpha_x, alpha_n);
    }

    return status;
}