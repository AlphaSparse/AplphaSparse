#include "alphasparse/kernel_plain.h"
#include "alphasparse/util.h"

alphasparse_status_t ONAME(const ALPHA_Number alpha, const ALPHA_SPMAT_DIA *A, const ALPHA_Number *x, const ALPHA_INT columns, const ALPHA_INT ldx, ALPHA_Number *y, const ALPHA_INT ldy)
{
    ALPHA_INT m = A->rows;
    ALPHA_INT main_diag_pos = 0;

    for (ALPHA_INT i = 0; i < A->ndiag; i++)
        if(A->distance[i] == 0)
        {
            main_diag_pos = i;
            break;
        }

    for(ALPHA_INT out_y_col = 0; out_y_col < columns; out_y_col++)
    {
        for (ALPHA_INT r = m - 1; r >= 0; r--)
        {
            ALPHA_Number temp;
            alpha_setzero(temp);
            for (ALPHA_INT ndiag = main_diag_pos + 1; ndiag < A->ndiag; ndiag++)
            {
                if (m - A->distance[ndiag] > r)
                {
                    ALPHA_INT ac = r + A->distance[ndiag];
                    alpha_madde(temp, A->values[ndiag * A->lval + r], y[out_y_col * ldy + ac]);
                    // temp += A->values[ndiag * A->lval + r] * y[out_y_col * ldy + ac];
                }
            }
            ALPHA_Number t;
            alpha_setzero(t);
            alpha_mul(t, alpha, x[out_y_col * ldx + r]);
            alpha_sub(y[out_y_col * ldy + r], t, temp);
            // y[out_y_col * ldy + r] = alpha * x[out_y_col * ldx + r] - temp;
        }
    }

    return ALPHA_SPARSE_STATUS_SUCCESS;
}
