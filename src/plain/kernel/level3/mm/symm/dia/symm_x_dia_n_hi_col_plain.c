#include "alphasparse/kernel_plain.h"
#include "alphasparse/util.h"

alphasparse_status_t ONAME(const ALPHA_Number alpha, const ALPHA_SPMAT_DIA *mat, const ALPHA_Number *x, const ALPHA_INT columns, const ALPHA_INT ldx, const ALPHA_Number beta, ALPHA_Number *y, const ALPHA_INT ldy)
{
    for (ALPHA_INT cc = 0; cc < columns; ++cc)
    {
        ALPHA_Number* Y = &y[index2(cc,0,ldy)];
        for (ALPHA_INT i = 0; i < mat->rows; i++)
            alpha_mul(Y[i],Y[i],beta);
        const ALPHA_Number* X = &x[index2(cc,0,ldx)];
        for(ALPHA_INT di = 0; di < mat->ndiag;++di){
            ALPHA_INT d = mat->distance[di];
            if(d > 0){
                ALPHA_INT ars = alpha_max(0,-d);
                ALPHA_INT acs = alpha_max(0,d);
                ALPHA_INT an = alpha_min(mat->rows - ars,mat->cols - acs);
                for(ALPHA_INT i = 0; i < an; ++i){
                    ALPHA_INT ar = ars + i;
                    ALPHA_INT ac = acs + i;
                    ALPHA_Number val;
                    alpha_mul(val,mat->values[index2(di,ar,mat->lval)],alpha);
                    alpha_madde(Y[ar],val,X[ac]);
                    alpha_madde(Y[ac],val,X[ar]);
                }
            }
            if(d == 0){
                for(ALPHA_INT r = 0; r < mat->rows; ++r){
                    ALPHA_Number val;
                    alpha_mul(val,mat->values[index2(di,r,mat->lval)],alpha);
                    alpha_madde(Y[r],val,X[r]);
                }
            }
        } 	
    }
    return ALPHA_SPARSE_STATUS_SUCCESS;
}
