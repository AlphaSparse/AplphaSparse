#include "alphasparse/kernel_plain.h"
#include "alphasparse/util.h"

alphasparse_status_t ONAME(const ALPHA_Number alpha, const ALPHA_SPMAT_COO *mat, const ALPHA_Number *x, const ALPHA_INT columns, const ALPHA_INT ldx, const ALPHA_Number beta, ALPHA_Number *y, const ALPHA_INT ldy)
{
    ALPHA_INT rowC = mat->rows;
    ALPHA_INT colC = columns;

    for (ALPHA_INT r = 0; r < rowC; ++r)
    {
        for (ALPHA_INT c = 0; c < colC; ++c)
        {
            ALPHA_Number t;
            alpha_mul(t, alpha, x[index2(r, c, ldx)]);
            alpha_mul(y[index2(r, c, ldy)], beta, y[index2(r, c, ldy)]);
            alpha_add(y[index2(r, c, ldy)], y[index2(r, c, ldy)], t);
            // y[index2(r,c,ldy)] = beta * y[index2(r,c,ldy)] + alpha * x[index2(r,c,ldy)];
        }
    }
    return ALPHA_SPARSE_STATUS_SUCCESS;
}
