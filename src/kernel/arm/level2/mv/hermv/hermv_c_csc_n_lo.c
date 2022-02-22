#include "alphasparse/kernel.h"
#ifdef _OPENMP
#include <omp.h>
#endif
#include "alphasparse/util.h"
#include <memory.h>


static alphasparse_status_t
hermv_csc_n_lo_unroll(const ALPHA_Number alpha,
		              const ALPHA_SPMAT_CSC *A,
		              const ALPHA_Number *x,
		              const ALPHA_Number beta,
		              ALPHA_Number *y)
{
    // m == n
	const ALPHA_INT m = A->rows;
	const ALPHA_INT n = A->cols;

	const ALPHA_INT num_threads = alpha_get_thread_num();

#ifdef _OPENMP
#pragma omp parallel for num_threads(num_threads)
#endif
	for(ALPHA_INT i = 0; i < m; ++i)
	{
		alpha_mul(y[i], y[i], beta);
	}

	ALPHA_Number **y_local = alpha_memalign(num_threads * sizeof(ALPHA_Number *), DEFAULT_ALIGNMENT);

#ifdef _OPENMP
#pragma omp parallel for num_threads(num_threads)
#endif
	for(ALPHA_INT i = 0; i < num_threads; i++)
	{
		y_local[i] = alpha_memalign(m * sizeof(ALPHA_Number), DEFAULT_ALIGNMENT);
		memset(y_local[i], '\0', sizeof(ALPHA_Number) * m);
	}

#ifdef _OPENMP
#pragma omp parallel for num_threads(num_threads)
#endif	
	for(ALPHA_INT i = 0; i < m; ++i)
	{
		ALPHA_INT tid = alpha_get_thread_id();	

		ALPHA_INT ais = A->cols_start[i];
        ALPHA_INT aie = A->cols_end[i];
        ALPHA_INT ail = aie - ais;

        ALPHA_INT start = alpha_lower_bound(&A->row_indx[ais], &A->row_indx[aie], i) - A->row_indx;
		
		if(start < aie && A->row_indx[start] == i){
			ALPHA_Number tmp;
			alpha_mul(tmp, alpha, A->values[start]);
			alpha_madde(y_local[tid][i], tmp, x[i]);
			start += 1;
		}

        const ALPHA_INT* A_row = &A->row_indx[ais];
        const ALPHA_Number*   A_val = &A->values[ais];
        
        ALPHA_INT ai = start - ais;

		ALPHA_Number alpha_xi, tmp;
		alpha_mul(alpha_xi, alpha, x[i]);
		for(; ai < ail-3; ai+=4)
        {
            ALPHA_Number av0 = A_val[ai];
            ALPHA_Number av1 = A_val[ai + 1];
            ALPHA_Number av2 = A_val[ai + 2];
            ALPHA_Number av3 = A_val[ai + 3];

			ALPHA_INT ar0 = A_row[ai];
            ALPHA_INT ar1 = A_row[ai + 1];
            ALPHA_INT ar2 = A_row[ai + 2];
            ALPHA_INT ar3 = A_row[ai + 3];
            
			alpha_madde(y_local[tid][ar0], av0, alpha_xi);
			alpha_madde(y_local[tid][ar1], av1, alpha_xi);
			alpha_madde(y_local[tid][ar2], av2, alpha_xi);
			alpha_madde(y_local[tid][ar3], av3, alpha_xi);

			alpha_mul_3c(tmp, alpha, av0);
			alpha_madde(y_local[tid][i], tmp, x[ar0]);
			alpha_mul_3c(tmp, alpha, av1);
			alpha_madde(y_local[tid][i], tmp, x[ar1]);
			alpha_mul_3c(tmp, alpha, av2);
			alpha_madde(y_local[tid][i], tmp, x[ar2]);
			alpha_mul_3c(tmp, alpha, av3);
			alpha_madde(y_local[tid][i], tmp, x[ar3]);
        }
        for(; ai < ail; ai++)
        {
            ALPHA_Number   av = A_val[ai];
			ALPHA_INT ar = A_row[ai];
			alpha_madde(y_local[tid][ar], av, alpha_xi);
			alpha_mul_3c(tmp, alpha, av);
			alpha_madde(y_local[tid][i], tmp, x[ar]);
        }
	}

#ifdef _OPENMP
#pragma omp parallel for num_threads(num_threads)
#endif
	for(ALPHA_INT col = 0; col < m; col++)
		for(ALPHA_INT i = 0; i < num_threads; i++)
		{
			alpha_add(y[col], y[col], y_local[i][col]);
		}

	for(ALPHA_INT i = 0; i < num_threads; i++)
	{
		alpha_free(y_local[i]);
	}

	alpha_free(y_local);
	return ALPHA_SPARSE_STATUS_SUCCESS;
}

alphasparse_status_t
ONAME(const ALPHA_Number alpha,
		              const ALPHA_SPMAT_CSC *A,
		              const ALPHA_Number *x,
		              const ALPHA_Number beta,
		              ALPHA_Number *y)
{
    return hermv_csc_n_lo_unroll(alpha, A, x, beta, y);
}
