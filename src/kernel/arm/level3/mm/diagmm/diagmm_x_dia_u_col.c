#include "alphasparse/kernel.h"
#include "alphasparse/util.h"
#include "alphasparse/opt.h"
#ifdef _OPENMP
#include <omp.h>
#endif

alphasparse_status_t ONAME(const ALPHA_Number alpha, const ALPHA_SPMAT_DIA *mat, const ALPHA_Number *x, const ALPHA_INT columns, const ALPHA_INT ldx, const ALPHA_Number beta, ALPHA_Number *y, const ALPHA_INT ldy)
{

    ALPHA_INT num_threads = alpha_get_thread_num();
#ifdef _OPENMP
#pragma omp parallel for num_threads(num_threads)
#endif
    for (ALPHA_INT c = 0; c < columns; ++c)
    {
        for (ALPHA_INT r = 0; r < mat->rows; ++r)
        {
            alpha_mul(y[index2(c,r,ldy)],y[index2(c,r,ldy)],beta);
            alpha_madde(y[index2(c,r,ldy)],x[index2(c,r,ldx)],alpha);
        }
    }
    return ALPHA_SPARSE_STATUS_SUCCESS;
}
