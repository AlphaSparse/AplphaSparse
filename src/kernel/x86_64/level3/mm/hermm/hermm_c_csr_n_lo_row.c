#include "alphasparse/kernel.h"
#include "alphasparse/util.h"

alphasparse_status_t ONAME(const ALPHA_Complex alpha, const ALPHA_SPMAT_CSR *mat, const ALPHA_Complex *x, const ALPHA_INT columns, const ALPHA_INT ldx, const ALPHA_Complex beta, ALPHA_Complex *y, const ALPHA_INT ldy)
{
    ALPHA_INT m = mat->rows;
    ALPHA_INT n = columns;
    for (ALPHA_INT r = 0; r < m; ++r)
    {
        for (ALPHA_INT c = 0; c < n; c++)
            alpha_mul(y[index2(r, c, ldy)], y[index2(r, c, ldy)], beta);
        for (ALPHA_INT ai = mat->rows_start[r]; ai < mat->rows_end[r]; ai++)
        {
            ALPHA_INT ac = mat->col_indx[ai];
            if (ac < r)
            {
                ALPHA_Complex val;
                ALPHA_Complex val_c;
                val_c.real = mat->values[ai].real;
                val_c.imag = 0.0 - mat->values[ai].imag;
                alpha_mul(val, alpha, mat->values[ai]);
                alpha_mul(val_c, alpha, val_c);
                for (ALPHA_INT c = 0; c < n; ++c)
                {
                    ALPHA_Complex tmp;
                    alpha_mul(tmp, val, x[index2(ac, c, ldx)]);
                    alpha_add(y[index2(r, c, ldy)], y[index2(r, c, ldy)], tmp);
                }                    
                for (ALPHA_INT c = 0; c < n; ++c)
                {
                    ALPHA_Complex tmp;
                    alpha_mul(tmp, val_c, x[index2(r, c, ldx)]);
                    alpha_add(y[index2(ac, c, ldy)], y[index2(ac, c, ldy)], tmp);
                }                    
            }
            else if (ac == r)
            {
                ALPHA_Complex val;
                alpha_mul(val, alpha, mat->values[ai]);
                for (ALPHA_INT c = 0; c < n; ++c)
                {
                    ALPHA_Complex tmp;
                    alpha_mul(tmp, val, x[index2(ac, c, ldx)]);
                    alpha_add(y[index2(r, c, ldy)], y[index2(r, c, ldy)], tmp);
                }                    
            }
        }
    }

    return ALPHA_SPARSE_STATUS_SUCCESS;
}
