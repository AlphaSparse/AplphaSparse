#include "alphasparse/kernel.h"
#include "alphasparse/util.h"
#include <alphasparse.h>
#define CACHELINE 64

alphasparse_status_t
ONAME(const ALPHA_Number alpha, const ALPHA_SPMAT_COO *mat, const ALPHA_Number *x, const ALPHA_INT columns, const ALPHA_INT ldx, const ALPHA_Number beta, ALPHA_Number *y, const ALPHA_INT ldy)
{

    ALPHA_INT m = mat->rows;
    ALPHA_INT n = columns;
    ALPHA_INT num_threads = alpha_get_thread_num();

#ifdef _OPENMP
#pragma omp parallel for num_threads(num_threads)
#endif
    for (ALPHA_INT i = 0; i < mat->rows; i++)
        for (ALPHA_INT j = 0; j < columns; j++)
        {
            alpha_mul(y[i * ldy + j], y[i * ldy + j], beta);
        }
    ALPHA_INT block_size = CACHELINE / sizeof(ALPHA_Number);
    ALPHA_INT block_num = (columns + block_size - 1) / block_size;
    if (num_threads > block_num)
        num_threads = block_num;

#ifdef _OPENMP
#pragma omp parallel num_threads(num_threads)
#endif
    {
        ALPHA_INT tid = alpha_get_thread_id();
        ALPHA_INT bcl = cross_block_low(tid, num_threads, block_num) * block_size;
        ALPHA_INT bch = cross_block_high(tid, num_threads, block_num) * block_size;
        if (bch > columns)
            bch = columns;

        for (ALPHA_INT ai = 0; ai < mat->nnz; ai++)
        {
            ALPHA_INT ac = mat->col_indx[ai];
            ALPHA_INT r = mat->row_indx[ai];
            if (ac > r)
            {
                ALPHA_Number val;
                alpha_mul_3c(val, alpha, mat->values[ai]);
                for (ALPHA_INT c = bcl; c < bch; ++c)
                    alpha_madde(y[index2(r, c, ldy)], val, x[index2(ac, c, ldx)]);
                for (ALPHA_INT c = bcl; c < bch; ++c)
                    alpha_madde(y[index2(ac, c, ldy)], val, x[index2(r, c, ldx)]);
            }
            else if (ac == r)
            {
                ALPHA_Number val;
                alpha_mul_3c(val, alpha, mat->values[ai]);

                for (ALPHA_INT c = bcl; c < bch; ++c)
                {
                    alpha_madde(y[index2(r, c, ldy)], val, x[index2(ac, c, ldx)]);
                }
            }
        }
    }
    return ALPHA_SPARSE_STATUS_SUCCESS;
}
