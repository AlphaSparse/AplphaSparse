#include <hip/hip_runtime.h>

#include "alphasparse/handle.h"
#include "alphasparse/util_dcu.h"
#include "alphasparse/compute.h"
#include "alphasparse/spdef.h"
#include "alphasparse/types.h"

__global__ static void
bsr_gemm_plain(alphasparse_layout_t dir,
               ALPHA_INT mb,
               ALPHA_INT n,
               ALPHA_INT kb,
               ALPHA_INT nnzb,
               const ALPHA_Number alpha,
               const ALPHA_Number *bsr_val,
               const ALPHA_INT *bsr_row_ptr,
               const ALPHA_INT *bsr_col_ind,
               ALPHA_INT bs,
               const ALPHA_Number *x,
               ALPHA_INT ldx,
               const ALPHA_Number beta,
               ALPHA_Number *y,
               ALPHA_INT ldy)
{
    ALPHA_INT tid    = hipBlockIdx_x * hipBlockDim_x + hipThreadIdx_x;
    ALPHA_INT stride = hipBlockDim_x * hipGridDim_x;

    ALPHA_INT m = mb * bs;

    for (ALPHA_INT j = tid * bs; j < n; j += stride * bs) {
        for (ALPHA_INT i = 0; i < m; ++i) {
            for (ALPHA_INT l = 0; l < bs; l++) {
                // y[index2(j, i, ldy)] *= beta;
                alpha_mul(y[index2(j + l, i, ldy)], beta, y[index2(j + l, i, ldy)]);
            }
        }
    }

    switch (dir) {
        case ALPHA_SPARSE_LAYOUT_ROW_MAJOR:
            for (ALPHA_INT c = tid * bs; c < n; c += bs * stride) { // choose a column from x
                for (ALPHA_INT r = 0; r < m; r += bs) { // choose a block of row
                    ALPHA_INT br = r / bs;
                    for (ALPHA_INT ai = bsr_row_ptr[br]; ai < bsr_row_ptr[br + 1]; ++ai) { // choose a block
                        const ALPHA_Number *blk = &bsr_val[ai * bs * bs];
                        for (ALPHA_INT cc = 0; cc < bs; ++cc)
                            for (ALPHA_INT lr = 0; lr < bs; ++lr) { // choose a inner row

                                ALPHA_INT ac = bsr_col_ind[ai] * bs;
                                ALPHA_Number extra;
                                alpha_setzero(extra);

                                for (ALPHA_INT lc = 0; lc < bs; ++lc) {
                                    // extra += blk[index2(lr, lc, bs)] * x[index2(c + cc, ac + lc, ldx)];
                                    alpha_madde(extra, blk[index2(lr, lc, bs)], x[index2(c + cc, ac + lc, ldx)]);
                                }
                                // y[index2(c + cc, r + lr, ldy)] += alpha * extra;
                                alpha_madde(y[index2(c + cc, r + lr, ldy)], alpha, extra);
                            }
                    }
                }
            }
            break;

        case ALPHA_SPARSE_LAYOUT_COLUMN_MAJOR:
            for (ALPHA_INT c = tid * bs; c < n; c += bs * stride) { // choose a column from x
                for (ALPHA_INT r = 0; r < m; r += bs) { // choose a block of row
                    ALPHA_INT br = r / bs;
                    for (ALPHA_INT ai = bsr_row_ptr[br]; ai < bsr_row_ptr[br + 1]; ++ai) { // choose a block
                        for (ALPHA_INT cc = 0; cc < bs; ++cc)
                            for (ALPHA_INT lr = 0; lr < bs; ++lr) { // choose a inner row

                                ALPHA_INT ac            = bsr_col_ind[ai] * bs;
                                const ALPHA_Number *blk = &bsr_val[ai * bs * bs];
                                ALPHA_Number extra;
                                alpha_setzero(extra);

                                for (ALPHA_INT lc = 0; lc < bs; ++lc) {
                                    // extra += blk[index2(lc, lr, bs)] * x[index2(c + cc, ac + lc, ldx)];
                                    alpha_madde(extra, blk[index2(lc, lr, bs)], x[index2(c + cc, ac + lc, ldx)]);
                                }
                                // y[index2(c + cc, r + lr, ldy)] += alpha * extra;
                                alpha_madde(y[index2(c + cc, r + lr, ldy)], alpha, extra);
                            }
                    }
                }
            }
            break;
    }
}

#define BLOCKSIZE 128
#define WF_SIZE   8
static __global__ void
    __launch_bounds__(BLOCKSIZE)
        bsrmm_2x2(alphasparse_layout_t direction,
                  ALPHA_INT Mb,
                  ALPHA_INT N,
                  ALPHA_Number alpha,
                  const ALPHA_INT *__restrict__ bsr_row_ptr,
                  const ALPHA_INT *__restrict__ bsr_col_ind,
                  const ALPHA_Number *__restrict__ bsr_val,
                  const ALPHA_Number *__restrict__ B,
                  ALPHA_INT ldb,
                  ALPHA_Number beta,
                  ALPHA_Number *__restrict__ matC,
                  ALPHA_INT ldc)
{
    constexpr ALPHA_INT BSR_BLOCK_DIM        = 2;
    constexpr ALPHA_INT PADDED_BSR_BLOCK_DIM = (BSR_BLOCK_DIM + 1);

    ALPHA_INT tid  = hipThreadIdx_x;
    ALPHA_INT gid  = hipBlockIdx_x * hipBlockDim_x + tid;
    ALPHA_INT lid  = gid & (WF_SIZE - 1);
    ALPHA_INT wid  = tid / WF_SIZE;
    ALPHA_INT nwfb = hipGridDim_x * hipBlockDim_x / (WF_SIZE * BSR_BLOCK_DIM);
    ALPHA_INT col  = lid + hipBlockIdx_y * WF_SIZE;

    ALPHA_INT colB = col * ldb;
    ALPHA_INT colC = col * ldc;

    ALPHA_Number zero;
    alpha_setzero(zero);

    // global row
    ALPHA_INT global_row = (gid / WF_SIZE);

    // local row within block row
    ALPHA_INT local_row = (gid / WF_SIZE) % BSR_BLOCK_DIM;

    __shared__ ALPHA_INT shared_col[BLOCKSIZE / WF_SIZE][WF_SIZE];
    __shared__ ALPHA_Number shared_val[BLOCKSIZE / WF_SIZE][WF_SIZE * PADDED_BSR_BLOCK_DIM];

    for (ALPHA_INT block_row = gid / (WF_SIZE * BSR_BLOCK_DIM); block_row < Mb; block_row += nwfb) {
        ALPHA_INT block_row_start = bsr_row_ptr[block_row];
        ALPHA_INT block_row_end   = bsr_row_ptr[block_row + 1];

        ALPHA_Number sum = zero;

        for (ALPHA_INT j = block_row_start; j < block_row_end; j += WF_SIZE) {
            ALPHA_INT k = j + lid;

            shared_col[wid][lid] = (k < block_row_end) ? BSR_BLOCK_DIM * (bsr_col_ind[k]) : 0;

            if (direction == ALPHA_SPARSE_LAYOUT_ROW_MAJOR) {
                shared_val[wid][PADDED_BSR_BLOCK_DIM * lid]     = (k < block_row_end) ? bsr_val[BSR_BLOCK_DIM * BSR_BLOCK_DIM * k + BSR_BLOCK_DIM * local_row] : zero;
                shared_val[wid][PADDED_BSR_BLOCK_DIM * lid + 1] = (k < block_row_end) ? bsr_val[BSR_BLOCK_DIM * BSR_BLOCK_DIM * k + BSR_BLOCK_DIM * local_row + 1] : zero;
            } else {
                shared_val[wid][PADDED_BSR_BLOCK_DIM * lid]     = (k < block_row_end) ? bsr_val[BSR_BLOCK_DIM * BSR_BLOCK_DIM * k + local_row] : zero;
                shared_val[wid][PADDED_BSR_BLOCK_DIM * lid + 1] = (k < block_row_end) ? bsr_val[BSR_BLOCK_DIM * BSR_BLOCK_DIM * k + BSR_BLOCK_DIM * 1 + local_row] : zero;
            }

            __syncthreads();

            if (col < N) {
                for (ALPHA_INT i = 0; i < WF_SIZE; ++i) {
                    // sum += shared_val[wid][PADDED_BSR_BLOCK_DIM * i] * B[shared_col[wid][i] + colB];
                    // sum += shared_val[wid][PADDED_BSR_BLOCK_DIM * i + 1] * B[shared_col[wid][i] + 1 + colB];
                    alpha_madde(sum, shared_val[wid][PADDED_BSR_BLOCK_DIM * i], B[shared_col[wid][i] + colB]);
                    alpha_madde(sum, shared_val[wid][PADDED_BSR_BLOCK_DIM * i + 1], B[shared_col[wid][i] + 1 + colB]);
                }
            }
        }

        if (col < N) {
            // matC[global_row + colC] = alpha * sum + beta * matC[global_row + colC];
            ALPHA_Number t1, t2;
            alpha_mul(t1, alpha, sum);
            alpha_mul(t2, beta, matC[global_row + colC]);
            alpha_add(matC[global_row + colC], t1, t2);
        }
    }
}

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

alphasparse_status_t
ONAME(alphasparse_dcu_handle_t handle,
      alphasparse_layout_t dir,
      ALPHA_INT mb,
      ALPHA_INT n,
      ALPHA_INT kb,
      ALPHA_INT nnzb,
      const ALPHA_Number alpha,
      const ALPHA_Number *bsr_val,
      const ALPHA_INT *bsr_row_ptr,
      const ALPHA_INT *bsr_col_ind,
      ALPHA_INT block_dim,
      const ALPHA_Number *matB,
      ALPHA_INT ldb,
      const ALPHA_Number beta,
      ALPHA_Number *matC,
      ALPHA_INT ldc)
{
    if (block_dim == 2) {
        dim3 bsrmm_blocks((WF_SIZE * mb * block_dim - 1) / BLOCKSIZE + 1, (n - 1) / WF_SIZE + 1);
        dim3 bsrmm_threads(BLOCKSIZE);
        hipLaunchKernelGGL(bsrmm_2x2, bsrmm_blocks, bsrmm_threads, 0, handle->stream, dir, mb, n, alpha, bsr_row_ptr, bsr_col_ind, bsr_val, matB, ldb, beta, matC, ldc);
    } else {
        //TODO a more effcient methord for general size
        dim3 bsrmm_threads(512);
        dim3 bsrmm_blocks(mb / 512 + 1);
        hipLaunchKernelGGL(bsr_gemm_plain, bsrmm_blocks, bsrmm_threads, 0, handle->stream, dir, mb, n, kb, nnzb, alpha, bsr_val, bsr_row_ptr, bsr_col_ind, block_dim, matB, ldb, beta, matC, ldc);
    }

    return ALPHA_SPARSE_STATUS_SUCCESS;
}
#undef BLOCKSIZE
#undef WF_SIZE

#ifdef __cplusplus
}
#endif /*__cplusplus */
