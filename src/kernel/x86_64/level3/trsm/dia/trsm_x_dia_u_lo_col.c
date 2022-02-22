#include "alphasparse/kernel.h"
#include "alphasparse/util.h"
#include "alphasparse/opt.h"
#ifdef _OPENMP
#include <omp.h>
#endif

alphasparse_status_t ONAME(const ALPHA_Number alpha, const ALPHA_SPMAT_DIA *A, const ALPHA_Number *x, const ALPHA_INT columns, const ALPHA_INT ldx, ALPHA_Number *y, const ALPHA_INT ldy)
{
    ALPHA_INT m = A->rows;
    ALPHA_INT main_diag_pos = 0;
    int num_thread = alpha_get_thread_num(); 

#ifdef _OPENMP
#pragma omp parallel for num_threads(num_thread)
#endif
    for (ALPHA_INT i = 0; i < A->ndiag; i++)
        if(A->distance[i] == 0)
        {
            main_diag_pos = i;
        }

#ifdef _OPENMP
#pragma omp parallel for num_threads(num_thread)
#endif
    for(ALPHA_INT out_y_col = 0; out_y_col < columns; out_y_col++)
    {
        for (ALPHA_INT r = 0; r < m; r++)
        {
            ALPHA_Number temp;
            alpha_setzero(temp);
            for (ALPHA_INT ndiag = 0; ndiag < main_diag_pos; ndiag++)
            {
                if (-A->distance[ndiag] <= r)
                {
                    ALPHA_INT ac = r + A->distance[ndiag];
                    alpha_madde(temp, A->values[ndiag * A->lval + r], y[out_y_col * ldy + ac]);
                }
            }
            ALPHA_Number t;
            alpha_setzero(t);
            alpha_mul(t, alpha, x[out_y_col * ldx + r]);
            alpha_sub(y[out_y_col * ldy + r], t, temp);
        }
    }

    return ALPHA_SPARSE_STATUS_SUCCESS;
}
