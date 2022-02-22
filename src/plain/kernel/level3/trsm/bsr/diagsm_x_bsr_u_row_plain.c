#include "alphasparse/kernel_plain.h"
#include "alphasparse/util.h"

alphasparse_status_t ONAME(const ALPHA_Number alpha, const ALPHA_SPMAT_BSR *A, const ALPHA_Number *x, const ALPHA_INT columns, const ALPHA_INT ldx, ALPHA_Number *y, const ALPHA_INT ldy)
{
#ifdef DEBUG
    printf("kernel diagsm_bsr_u_row_plain called \n");
#endif
    for (ALPHA_INT r = 0; r < A->rows * A->block_size; ++r)
    {
        for (ALPHA_INT c = 0; c < columns; ++c)
        {
            alpha_mul(y[index2(r, c, ldy)] , alpha , x[index2(r, c, ldx)]);
        }
    }
    return ALPHA_SPARSE_STATUS_SUCCESS;
}
