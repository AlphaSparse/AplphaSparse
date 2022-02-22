#include "alphasparse/kernel_plain.h"
#include "alphasparse/util.h"

alphasparse_status_t ONAME(const ALPHA_Number alpha, const ALPHA_SPMAT_SKY *A, const ALPHA_Number *x, const ALPHA_INT columns, const ALPHA_INT ldx, ALPHA_Number *y, const ALPHA_INT ldy)
{
    for(ALPHA_INT out_y_col = 0; out_y_col < columns; out_y_col++)
    {
        for (ALPHA_INT c = A->cols - 1; c >= 0; c--)
        {
            ALPHA_Number temp;
            alpha_setzero(temp);
            for (ALPHA_INT ic = A->cols - 1; ic > c; ic--)
            {
                ALPHA_INT start = A->pointers[ic];
                ALPHA_INT end   = A->pointers[ic + 1];
                ALPHA_INT eles_num = ic - c;
                if(end - eles_num - 1 >= start)
                    alpha_madde(temp, A->values[end - eles_num - 1], y[ic * ldy + out_y_col]);
            }

            ALPHA_Number t;
            alpha_mul(t, alpha, x[c * ldx + out_y_col]);
            alpha_sub(y[c * ldy + out_y_col], t, temp);
            // y[c * ldy + out_y_col] = alpha * x[c * ldx + out_y_col] - temp;
        }
    }
    return ALPHA_SPARSE_STATUS_SUCCESS;
}
