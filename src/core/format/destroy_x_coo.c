#include "alphasparse/format.h"
#include <alphasparse/util.h>

alphasparse_status_t ONAME(ALPHA_SPMAT_COO *A)
{
    alpha_free(A->row_indx);
    alpha_free(A->col_indx);
    alpha_free(A->values);

    alpha_free(A);
    return ALPHA_SPARSE_STATUS_SUCCESS;
}