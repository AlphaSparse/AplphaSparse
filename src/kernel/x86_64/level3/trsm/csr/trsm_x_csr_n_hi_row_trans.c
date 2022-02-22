#include "alphasparse/kernel.h"

alphasparse_status_t ONAME(const ALPHA_Number alpha, const ALPHA_SPMAT_CSR *A, const ALPHA_Number *x, const ALPHA_INT columns, const ALPHA_INT ldx, ALPHA_Number *y, const ALPHA_INT ldy)
{
    ALPHA_SPMAT_CSR *transposed_mat;
    transpose_csr(A, &transposed_mat);
    alphasparse_status_t status = trsm_csr_n_lo_row(alpha, transposed_mat, x, columns, ldx, y, ldy);
    destroy_csr(transposed_mat);
    return status;
}
