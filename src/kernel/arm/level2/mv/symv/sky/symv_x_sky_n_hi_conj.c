#include "alphasparse/kernel.h"
#include "alphasparse/util.h"
#include "alphasparse/opt.h"

alphasparse_status_t
ONAME(const ALPHA_Number alpha,
		              const ALPHA_SPMAT_SKY *A,
		              const ALPHA_Number *x,
		              const ALPHA_Number beta,
		              ALPHA_Number *y)
{
#ifdef COMPLEX
    const ALPHA_INT m = A->rows;
	const ALPHA_INT n = A->cols;
	if(m != n) return ALPHA_SPARSE_STATUS_INVALID_VALUE;

	for(ALPHA_INT i = 0; i < m; ++i)
	{
		alpha_mul(y[i], y[i], beta);
	}
	for(ALPHA_INT c = 0; c < n; ++c)
    {
		const ALPHA_INT col_start = A->pointers[c];
		const ALPHA_INT col_end = A->pointers[c + 1];
		ALPHA_INT col_indx = 1;
		for(ALPHA_INT i = col_start; i < col_end; i++)
		{
			ALPHA_INT col_eles = col_end - col_start;
			ALPHA_Number v;
			alpha_conj(v,A->values[i]);
			alpha_mul(v, v, alpha);
			if(i == col_end - 1)
			{
				ALPHA_INT r = c;
				alpha_madde(y[r], v, x[c]);
			}
			else
			{
				ALPHA_INT r = c - col_eles + col_indx;
				alpha_madde(y[r], v, x[c]);
				alpha_madde(y[c], v, x[r]);
				col_indx ++;
			}
		}
    }
	return ALPHA_SPARSE_STATUS_SUCCESS;
#else
	return ALPHA_SPARSE_STATUS_INVALID_VALUE;
#endif
}
