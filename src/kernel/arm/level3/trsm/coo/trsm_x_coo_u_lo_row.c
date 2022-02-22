#include "alphasparse/kernel.h"
#include "alphasparse/util.h"
#include "alphasparse/opt.h"

alphasparse_status_t ONAME(const ALPHA_Number alpha, const ALPHA_SPMAT_COO *A, const ALPHA_Number *x, const ALPHA_INT columns, const ALPHA_INT ldx, ALPHA_Number *y, const ALPHA_INT ldy)
{
    int m = A->rows;
    int num_thread = alpha_get_thread_num(); 

#ifdef _OPENMP
#pragma omp parallel for num_threads(num_thread)
#endif
    for(ALPHA_INT out_y_col = 0; out_y_col < columns; out_y_col++)
    {        
        for (ALPHA_INT r = 0; r < m; r++)
        {
            ALPHA_Number temp;
            alpha_setzero(temp);
            for (ALPHA_INT cr = 0; cr < A->nnz; cr++)
            {
                int row = A->row_indx[cr];
                int col = A->col_indx[cr];
                if(row == r && col < r)
                    alpha_madde(temp, A->values[cr], y[col * ldy + out_y_col]);
            }
            ALPHA_Number t;
            alpha_mul(t, alpha, x[r * ldx + out_y_col]);
            alpha_sub(y[r * ldy + out_y_col], t, temp);
        }
    }
    return ALPHA_SPARSE_STATUS_SUCCESS;
}
