#include "alphasparse/kernel.h"
#include "alphasparse/util.h"
#include "alphasparse/opt.h"
#include <memory.h>

alphasparse_status_t
ONAME(const ALPHA_Number alpha, const ALPHA_SPMAT_COO *mat, const ALPHA_Number *x, const ALPHA_INT columns, const ALPHA_INT ldx, const ALPHA_Number beta, ALPHA_Number *y, const ALPHA_INT ldy)
{
    ALPHA_INT m = mat->rows;
    ALPHA_INT n = columns;
    ALPHA_INT num_threads = alpha_get_thread_num();

#ifdef _OPENMP
#pragma omp parallel for num_threads(num_threads)
#endif
    for (ALPHA_INT i = 0; i < m * n; i++)
        alpha_mul(y[i], y[i], beta);

#ifdef _OPENMP
#pragma omp parallel num_threads(num_threads)
#endif
    {
        ALPHA_INT tid = alpha_get_thread_id();
        for (ALPHA_INT ai = 0; ai < mat->nnz; ++ai)
        {
            ALPHA_INT cr = mat->row_indx[ai];
            if (cr % num_threads != tid)
                continue;

            ALPHA_Number *Y = &y[index2(cr, 0, ldy)];

            if (mat->col_indx[ai] <= cr)
            {
                ALPHA_Number val;
                alpha_mul(val, alpha, mat->values[ai]);
                const ALPHA_Number *X = &x[index2(mat->col_indx[ai], 0, ldx)];
                for (ALPHA_INT c = 0; c < n; ++c)
                    alpha_madde(Y[c], val, X[c]);
            }
        }
    }
    return ALPHA_SPARSE_STATUS_SUCCESS;
}
