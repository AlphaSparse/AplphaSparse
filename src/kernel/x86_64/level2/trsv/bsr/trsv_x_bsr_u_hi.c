#include "alphasparse/kernel.h"
#include "alphasparse/util.h"
#include <memory.h>

alphasparse_status_t ONAME(const ALPHA_Number alpha, const ALPHA_SPMAT_BSR *A, const ALPHA_Number *x, ALPHA_Number *y)
{
    ALPHA_INT block_rowA = A->rows;
    ALPHA_INT rowA = A->rows * A->block_size;
    ALPHA_INT bs = A->block_size;
    
    ALPHA_Number temp[rowA];
    memset(temp, '\0', sizeof(ALPHA_Number)*rowA);
    for (ALPHA_INT r = block_rowA-1; r >=0 ; r--)
    {
        for (ALPHA_INT ai = A->rows_end[r]-1; ai >= A->rows_start[r]; ai--)
        {
            ALPHA_INT ac = A->col_indx[ai];
            if(ac == r)
            {
                for(ALPHA_INT block_r = bs-1; block_r>=0 ; block_r--)
                {
                    for(ALPHA_INT block_c = bs-1; block_c >= 0; block_c--)
                    {
			            if(block_r == block_c)
                        {
                            ALPHA_Number t;
                            alpha_mul(t, alpha, x[r*bs + block_r]);
                            alpha_sub(y[r*bs + block_r], t, temp[r*bs + block_r]);
                            continue;
                        }
                        if(A->block_layout == ALPHA_SPARSE_LAYOUT_ROW_MAJOR)
                        {
                            alpha_madde(temp[r*bs + block_r], A->values[ai*bs*bs + block_r*bs + block_c], y[ac*bs + block_c]);
                        }
                        else if(A->block_layout == ALPHA_SPARSE_LAYOUT_COLUMN_MAJOR)
                        {
                            alpha_madde(temp[r*bs + block_r], A->values[ai*bs*bs + block_c*bs + block_r], y[ac*bs + block_c]);
                        }
                        else
                        {
                            return ALPHA_SPARSE_STATUS_INVALID_VALUE;
                        }
                    }
                }
            }
            else if (ac > r)
            {
                for(ALPHA_INT block_r = 0; block_r < bs; block_r++)
                {
                    for(ALPHA_INT block_c = 0; block_c < bs; block_c++)
                    {
                        if(A->block_layout == ALPHA_SPARSE_LAYOUT_ROW_MAJOR)
                        {
                            alpha_madde(temp[r*bs + block_r], A->values[ai*bs*bs + block_r*bs + block_c], y[ac*bs + block_c]);
                        }
                        else if(A->block_layout == ALPHA_SPARSE_LAYOUT_COLUMN_MAJOR)
                        {
                            alpha_madde(temp[r*bs + block_r], A->values[ai*bs*bs + block_c*bs + block_r], y[ac*bs + block_c]);
                        }
                        else
                        {
                            return ALPHA_SPARSE_STATUS_INVALID_VALUE;
                        }
                    }
                }
            }
        }
    }
    return ALPHA_SPARSE_STATUS_SUCCESS;
}
