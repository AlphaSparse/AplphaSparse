#include "alphasparse/kernel.h"
#include "alphasparse/util.h"

alphasparse_status_t ONAME(const ALPHA_Number alpha, const ALPHA_SPMAT_SKY *mat, const ALPHA_Number *x, const ALPHA_INT columns, const ALPHA_INT ldx, const ALPHA_Number beta, ALPHA_Number *y, const ALPHA_INT ldy)
{
    ALPHA_INT m = mat->rows;
    ALPHA_INT n = columns;
    for (ALPHA_INT r = 0; r < m; ++r)
        for (ALPHA_INT c = 0; c < n; c++)
        {
            ALPHA_Number tmp;
            alpha_mul(tmp, alpha, x[index2(r, c, ldx)]);
            alpha_mul(y[index2(r, c, ldy)], beta, y[index2(r, c, ldy)]);
            alpha_add(y[index2(r, c, ldy)], y[index2(r, c, ldy)], tmp);
        }

    for (ALPHA_INT ac = 0; ac < mat->cols; ++ac)
    {
        ALPHA_INT start = mat->pointers[ac];
        ALPHA_INT end   = mat->pointers[ac + 1];
        ALPHA_INT idx = 1;
        ALPHA_INT eles_num = end - start;

        for (ALPHA_INT ai = start; ai < end; ++ai)
        {
            ALPHA_INT r = ac - eles_num + idx;

            if (ac > r)
            {
                ALPHA_Number val;
                alpha_mul(val, alpha, mat->values[ai]);
                for (ALPHA_INT c = 0; c < n; ++c)
                    alpha_madde(y[index2(r, c, ldy)], val, x[index2(ac, c, ldx)]);
                for (ALPHA_INT c = 0; c < n; ++c)
                    alpha_madde(y[index2(ac, c, ldy)], val, x[index2(r, c, ldx)]);
            }
            idx++;
        }
    }
    return ALPHA_SPARSE_STATUS_SUCCESS;
}
