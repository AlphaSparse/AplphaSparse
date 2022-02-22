#include "openspblas/util.h"
#include "openspblas/opt.h"
#include "openspblas/spapi.h"
#include "openspblas/kernel.h"
#include "openspblas/spdef.h"

/*
* 
* Solve a set of equations in which a sparse matrix and a dense vector are multiplied
*
* details:
* trsv_csr_n_lo         Triangular Matrics defined in csr storage format, clculate lower triangle including diagal
* trsv_csr_u_lo         Triangular Matrics defined in csr storage format, clculate lower triangle excluding diagal 
* trsv_csr_n_hi         Triangular Matrics defined in csr storage format, clculate higher triangle including diagal
* trsv_csr_u_hi         Triangular Matrics defined in csr storage format, clculate higher triangle excluding diagal 
* trsv_csr_n_lo_trans    Transpose of Triangular Matrics defined in csr storage format, clculate lower triangle including diagal
* trsv_csr_u_lo_trans    Transpose of Triangular Matrics defined in csr storage format, clculate lower triangle excluding diagal
* trsv_csr_n_hi_trans    Transpose of Triangular Matrics defined in csr storage format, clculate higher triangle including diagal
* trsv_csr_u_hi_trans    Transpose of Triangular Matrics defined in csr storage format, clculate higher triangle excluding diagal
*
* op(A)*y = alpha * x
* op: op(A) = A
*     op(A) = AT
*     op(A) = AH
*
* alpha     a scalor value
* x         a sparse vector in a compressed format of complex numbers
* y         a fully stored vector of real numbers
* op(A)     Data structure of sparse matrix
*
* input:
* alpha     a scalor value
* beta      a scalor value
* x         a sparse vector in a compressed format of complex numbers
* y         a fully stored vector of real numbers
* A         Data structure of sparse matrix
* 
* output:
* y         a fully stored vector of real numbers
*
*/

static openspblas_sparse_status_t (*trsv_csr_diag_fill_operation[])(const OPENSPBLAS_Number alpha,
                                                        const OPENSPBLAS_SPMAT_CSR *A,
                                                        const OPENSPBLAS_Number *x,
                                                        OPENSPBLAS_Number *y) = {
    trsv_csr_n_lo,
    trsv_csr_u_lo,
    trsv_csr_n_hi,
    trsv_csr_u_hi,
    trsv_csr_n_lo_trans,
    trsv_csr_u_lo_trans,
    trsv_csr_n_hi_trans,
    trsv_csr_u_hi_trans,
#ifdef COMPLEX
    trsv_csr_n_lo_conj, 
    trsv_csr_u_lo_conj, 
    trsv_csr_n_hi_conj, 
    trsv_csr_u_hi_conj, 
#endif
};

/*
* 
* Solve a set of equations in which a sparse matrix and a dense vector are multiplied
*
* details:
* diagsv_csr_n      Diagonal Matrics defined in csr storage format, including diagal
* diagsv_csr_u      Diagonal Matrics defined in csr storage format, all diagal elements are 1
*
* op(A)*y = alpha * x
* op: op(A) = A
*     op(A) = AT
*     op(A) = AH
*
* alpha     a scalor value
* x         a sparse vector in a compressed format of complex numbers
* y         a fully stored vector of real numbers
* op(A)     Data structure of sparse matrix
*
* input:
* alpha     a scalor value
* beta      a scalor value
* x         a sparse vector in a compressed format of complex numbers
* y         a fully stored vector of real numbers
* A         Data structure of sparse matrix
* 
* output:
* y         a fully stored vector of real numbers
*
*/

static openspblas_sparse_status_t (*diagsv_csr_diag[])(const OPENSPBLAS_Number alpha,
                                           const OPENSPBLAS_SPMAT_CSR *A,
                                           const OPENSPBLAS_Number *x,
                                           OPENSPBLAS_Number *y) = {
    diagsv_csr_n,
    diagsv_csr_u,
};

/*
* 
* Solve a set of equations in which a sparse matrix and a dense vector are multiplied
*
* details:
* trsv_coo_n_lo         Triangular Matrics defined in coo storage format, clculate lower triangle including diagal
* trsv_coo_u_lo         Triangular Matrics defined in coo storage format, clculate lower triangle excluding diagal 
* trsv_coo_n_hi         Triangular Matrics defined in coo storage format, clculate higher triangle including diagal
* trsv_coo_u_hi         Triangular Matrics defined in coo storage format, clculate higher triangle excluding diagal 
* trsv_coo_n_lo_trans    Transpose of Triangular Matrics defined in coo storage format, clculate lower triangle including diagal
* trsv_coo_u_lo_trans    Transpose of Triangular Matrics defined in coo storage format, clculate lower triangle excluding diagal
* trsv_coo_n_hi_trans    Transpose of Triangular Matrics defined in coo storage format, clculate higher triangle including diagal
* trsv_coo_u_hi_trans    Transpose of Triangular Matrics defined in coo storage format, clculate higher triangle excluding diagal
*
* op(A)*y = alpha * x
* op: op(A) = A
*     op(A) = AT
*     op(A) = AH
*
* alpha     a scalor value
* x         a sparse vector in a compressed format of complex numbers
* y         a fully stored vector of real numbers
* op(A)     Data structure of sparse matrix
*
* input:
* alpha     a scalor value
* beta      a scalor value
* x         a sparse vector in a compressed format of complex numbers
* y         a fully stored vector of real numbers
* A         Data structure of sparse matrix
* 
* output:
* y         a fully stored vector of real numbers
*
*/

static openspblas_sparse_status_t (*trsv_coo_diag_fill_operation[])(const OPENSPBLAS_Number alpha,
                                                        const OPENSPBLAS_SPMAT_COO *A,
                                                        const OPENSPBLAS_Number *x,
                                                        OPENSPBLAS_Number *y) = {
    trsv_coo_n_lo,
    trsv_coo_u_lo,
    trsv_coo_n_hi,
    trsv_coo_u_hi,
    trsv_coo_n_lo_trans,
    trsv_coo_u_lo_trans,
    trsv_coo_n_hi_trans,
    trsv_coo_u_hi_trans,
#ifdef COMPLEX
    trsv_coo_n_lo_conj, 
    trsv_coo_u_lo_conj, 
    trsv_coo_n_hi_conj, 
    trsv_coo_u_hi_conj, 
#endif
};

/*
* 
* Solve a set of equations in which a sparse matrix and a dense vector are multiplied
*
* details:
* diagsv_coo_n      Diagonal Matrics defined in coo storage format, including diagal
* diagsv_coo_u      Diagonal Matrics defined in coo storage format, all diagal elements are 1
*
* op(A)*y = alpha * x
* op: op(A) = A
*     op(A) = AT
*     op(A) = AH
*
* alpha     a scalor value
* x         a sparse vector in a compressed format of complex numbers
* y         a fully stored vector of real numbers
* op(A)     Data structure of sparse matrix
*
* input:
* alpha     a scalor value
* beta      a scalor value
* x         a sparse vector in a compressed format of complex numbers
* y         a fully stored vector of real numbers
* A         Data structure of sparse matrix
* 
* output:
* y         a fully stored vector of real numbers
*
*/

static openspblas_sparse_status_t (*diagsv_coo_diag[])(const OPENSPBLAS_Number alpha,
                                           const OPENSPBLAS_SPMAT_COO *A,
                                           const OPENSPBLAS_Number *x,
                                           OPENSPBLAS_Number *y) = {
    diagsv_coo_n,
    diagsv_coo_u,
};

/*
* 
* Solve a set of equations in which a sparse matrix and a dense vector are multiplied
*
* details:
* trsv_bsr_n_lo         Triangular Matrics defined in bsr storage format, clculate lower triangle including diagal
* trsv_bsr_u_lo         Triangular Matrics defined in bsr storage format, clculate lower triangle excluding diagal 
* trsv_bsr_n_hi         Triangular Matrics defined in bsr storage format, clculate higher triangle including diagal
* trsv_bsr_u_hi         Triangular Matrics defined in bsr storage format, clculate higher triangle excluding diagal 
* trsv_bsr_n_lo_trans    Transpose of Triangular Matrics defined in bsr storage format, clculate lower triangle including diagal
* trsv_bsr_u_lo_trans    Transpose of Triangular Matrics defined in bsr storage format, clculate lower triangle excluding diagal
* trsv_bsr_n_hi_trans    Transpose of Triangular Matrics defined in bsr storage format, clculate higher triangle including diagal
* trsv_bsr_u_hi_trans    Transpose of Triangular Matrics defined in bsr storage format, clculate higher triangle excluding diagal
*
* op(A)*y = alpha * x
* op: op(A) = A
*     op(A) = AT
*     op(A) = AH
*
* alpha     a scalor value
* x         a sparse vector in a compressed format of complex numbers
* y         a fully stored vector of real numbers
* op(A)     Data structure of sparse matrix
*
* input:
* alpha     a scalor value
* beta      a scalor value
* x         a sparse vector in a compressed format of complex numbers
* y         a fully stored vector of real numbers
* A         Data structure of sparse matrix
* 
* output:
* y         a fully stored vector of real numbers
*
*/

static openspblas_sparse_status_t (*trsv_bsr_diag_fill_operation[])(const OPENSPBLAS_Number alpha,
                                                        const OPENSPBLAS_SPMAT_BSR *A,
                                                        const OPENSPBLAS_Number *x,
                                                        OPENSPBLAS_Number *y) = {
    trsv_bsr_n_lo,
    trsv_bsr_u_lo,
    trsv_bsr_n_hi,
    trsv_bsr_u_hi,
    trsv_bsr_n_lo_trans,
    trsv_bsr_u_lo_trans,
    trsv_bsr_n_hi_trans,
    trsv_bsr_u_hi_trans,
#ifdef COMPLEX
    trsv_bsr_n_lo_conj, 
    trsv_bsr_u_lo_conj, 
    trsv_bsr_n_hi_conj, 
    trsv_bsr_u_hi_conj, 
#endif
};

/*
* 
* Solve a set of equations in which a sparse matrix and a dense vector are multiplied
*
* details:
* diagsv_bsr_n      Diagonal Matrics defined in bsr storage format, including diagal
* diagsv_bsr_u      Diagonal Matrics defined in bsr storage format, all diagal elements are 1
*
* op(A)*y = alpha * x
* op: op(A) = A
*     op(A) = AT
*     op(A) = AH
*
* alpha     a scalor value
* x         a sparse vector in a compressed format of complex numbers
* y         a fully stored vector of real numbers
* op(A)     Data structure of sparse matrix
*
* input:
* alpha     a scalor value
* beta      a scalor value
* x         a sparse vector in a compressed format of complex numbers
* y         a fully stored vector of real numbers
* A         Data structure of sparse matrix
* 
* output:
* y         a fully stored vector of real numbers
*
*/

static openspblas_sparse_status_t (*diagsv_bsr_diag[])(const OPENSPBLAS_Number alpha,
                                           const OPENSPBLAS_SPMAT_BSR *A,
                                           const OPENSPBLAS_Number *x,
                                           OPENSPBLAS_Number *y) = {
    diagsv_bsr_n,
    diagsv_bsr_u,
};

/*
* 
* Solve a set of equations in which a sparse matrix and a dense vector are multiplied
*
* details:
* trsv_csc_n_lo         Triangular Matrics defined in csc storage format, clculate lower triangle including diagal
* trsv_csc_u_lo         Triangular Matrics defined in csc storage format, clculate lower triangle excluding diagal 
* trsv_csc_n_hi         Triangular Matrics defined in csc storage format, clculate higher triangle including diagal
* trsv_csc_u_hi         Triangular Matrics defined in csc storage format, clculate higher triangle excluding diagal 
* trsv_csc_n_lo_trans    Transpose of Triangular Matrics defined in csc storage format, clculate lower triangle including diagal
* trsv_csc_u_lo_trans    Transpose of Triangular Matrics defined in csc storage format, clculate lower triangle excluding diagal
* trsv_csc_n_hi_trans    Transpose of Triangular Matrics defined in csc storage format, clculate higher triangle including diagal
* trsv_csc_u_hi_trans    Transpose of Triangular Matrics defined in csc storage format, clculate higher triangle excluding diagal
*
* op(A)*y = alpha * x
* op: op(A) = A
*     op(A) = AT
*     op(A) = AH
*
* alpha     a scalor value
* x         a sparse vector in a compressed format of complex numbers
* y         a fully stored vector of real numbers
* op(A)     Data structure of sparse matrix
*
* input:
* alpha     a scalor value
* beta      a scalor value
* x         a sparse vector in a compressed format of complex numbers
* y         a fully stored vector of real numbers
* A         Data structure of sparse matrix
* 
* output:
* y         a fully stored vector of real numbers
*
*/

static openspblas_sparse_status_t (*trsv_csc_diag_fill_operation[])(const OPENSPBLAS_Number alpha,
                                                        const OPENSPBLAS_SPMAT_CSC *A,
                                                        const OPENSPBLAS_Number *x,
                                                        OPENSPBLAS_Number *y) = {
    trsv_csc_n_lo,
    trsv_csc_u_lo,
    trsv_csc_n_hi,
    trsv_csc_u_hi,
    trsv_csc_n_lo_trans,
    trsv_csc_u_lo_trans,
    trsv_csc_n_hi_trans,
    trsv_csc_u_hi_trans,
#ifdef COMPLEX
    trsv_csc_n_lo_conj, 
    trsv_csc_u_lo_conj, 
    trsv_csc_n_hi_conj, 
    trsv_csc_u_hi_conj, 
#endif
};

/*
* 
* Solve a set of equations in which a sparse matrix and a dense vector are multiplied
*
* details:
* diagsv_csc_n      Diagonal Matrics defined in csc storage format, including diagal
* diagsv_csc_u      Diagonal Matrics defined in csc storage format, all diagal elements are 1
*
* op(A)*y = alpha * x
* op: op(A) = A
*     op(A) = AT
*     op(A) = AH
*
* alpha     a scalor value
* x         a sparse vector in a compressed format of complex numbers
* y         a fully stored vector of real numbers
* op(A)     Data structure of sparse matrix
*
* input:
* alpha     a scalor value
* beta      a scalor value
* x         a sparse vector in a compressed format of complex numbers
* y         a fully stored vector of real numbers
* A         Data structure of sparse matrix
* 
* output:
* y         a fully stored vector of real numbers
*
*/

static openspblas_sparse_status_t (*diagsv_csc_diag[])(const OPENSPBLAS_Number alpha,
                                           const OPENSPBLAS_SPMAT_CSC *A,
                                           const OPENSPBLAS_Number *x,
                                           OPENSPBLAS_Number *y) = {
    diagsv_csc_n,
    diagsv_csc_u,
};

/*
* 
* Solve a set of equations in which a sparse matrix and a dense vector are multiplied
*
* details:
* trsv_sky_n_lo         Triangular Matrics defined in sky storage format, clculate lower triangle including diagal
* trsv_sky_u_lo         Triangular Matrics defined in sky storage format, clculate lower triangle excluding diagal 
* trsv_sky_n_hi         Triangular Matrics defined in sky storage format, clculate higher triangle including diagal
* trsv_sky_u_hi         Triangular Matrics defined in sky storage format, clculate higher triangle excluding diagal 
* trsv_sky_n_lo_trans    Transpose of Triangular Matrics defined in sky storage format, clculate lower triangle including diagal
* trsv_sky_u_lo_trans    Transpose of Triangular Matrics defined in sky storage format, clculate lower triangle excluding diagal
* trsv_sky_n_hi_trans    Transpose of Triangular Matrics defined in sky storage format, clculate higher triangle including diagal
* trsv_sky_u_hi_trans    Transpose of Triangular Matrics defined in sky storage format, clculate higher triangle excluding diagal
*
* op(A)*y = alpha * x
* op: op(A) = A
*     op(A) = AT
*     op(A) = AH
*
* alpha     a scalor value
* x         a sparse vector in a compressed format of complex numbers
* y         a fully stored vector of real numbers
* op(A)     Data structure of sparse matrix
*
* input:
* alpha     a scalor value
* beta      a scalor value
* x         a sparse vector in a compressed format of complex numbers
* y         a fully stored vector of real numbers
* A         Data structure of sparse matrix
* 
* output:
* y         a fully stored vector of real numbers
*
*/


static openspblas_sparse_status_t (*trsv_sky_diag_fill_operation[])(const OPENSPBLAS_Number alpha,
                                                        const OPENSPBLAS_SPMAT_SKY *A,
                                                        const OPENSPBLAS_Number *x,
                                                        OPENSPBLAS_Number *y) = {
    trsv_sky_n_lo,
    trsv_sky_u_lo,
    trsv_sky_n_hi,
    trsv_sky_u_hi,
    trsv_sky_n_lo_trans,
    trsv_sky_u_lo_trans,
    trsv_sky_n_hi_trans,
    trsv_sky_u_hi_trans,
#ifdef COMPLEX
    trsv_sky_n_lo_conj, 
    trsv_sky_u_lo_conj, 
    trsv_sky_n_hi_conj, 
    trsv_sky_u_hi_conj, 
#endif
};

/*
* 
* Solve a set of equations in which a sparse matrix and a dense vector are multiplied
*
* details:
* diagsv_sky_n      Diagonal Matrics defined in sky storage format, including diagal
* diagsv_sky_u      Diagonal Matrics defined in sky storage format, all diagal elements are 1
*
* op(A)*y = alpha * x
* op: op(A) = A
*     op(A) = AT
*     op(A) = AH
*
* alpha     a scalor value
* x         a sparse vector in a compressed format of complex numbers
* y         a fully stored vector of real numbers
* op(A)     Data structure of sparse matrix
*
* input:
* alpha     a scalor value
* beta      a scalor value
* x         a sparse vector in a compressed format of complex numbers
* y         a fully stored vector of real numbers
* A         Data structure of sparse matrix
* 
* output:
* y         a fully stored vector of real numbers
*
*/

static openspblas_sparse_status_t (*diagsv_sky_diag[])(const OPENSPBLAS_Number alpha,
                                           const OPENSPBLAS_SPMAT_SKY *A,
                                           const OPENSPBLAS_Number *x,
                                           OPENSPBLAS_Number *y) = {
    diagsv_sky_n,
    diagsv_sky_u,
};

/*
* 
* Solve a set of equations in which a sparse matrix and a dense vector are multiplied
*
* details:
* trsv_dia_n_lo         Triangular Matrics defined in dia storage format, clculate lower triangle including diagal
* trsv_dia_u_lo         Triangular Matrics defined in dia storage format, clculate lower triangle excluding diagal 
* trsv_dia_n_hi         Triangular Matrics defined in dia storage format, clculate higher triangle including diagal
* trsv_dia_u_hi         Triangular Matrics defined in dia storage format, clculate higher triangle excluding diagal 
* trsv_dia_n_lo_trans    Transpose of Triangular Matrics defined in dia storage format, clculate lower triangle including diagal
* trsv_dia_u_lo_trans    Transpose of Triangular Matrics defined in dia storage format, clculate lower triangle excluding diagal
* trsv_dia_n_hi_trans    Transpose of Triangular Matrics defined in dia storage format, clculate higher triangle including diagal
* trsv_dia_u_hi_trans    Transpose of Triangular Matrics defined in dia storage format, clculate higher triangle excluding diagal
*
* op(A)*y = alpha * x
* op: op(A) = A
*     op(A) = AT
*     op(A) = AH
*
* alpha     a scalor value
* x         a sparse vector in a compressed format of complex numbers
* y         a fully stored vector of real numbers
* op(A)     Data structure of sparse matrix
*
* input:
* alpha     a scalor value
* beta      a scalor value
* x         a sparse vector in a compressed format of complex numbers
* y         a fully stored vector of real numbers
* A         Data structure of sparse matrix
* 
* output:
* y         a fully stored vector of real numbers
*
*/

static openspblas_sparse_status_t (*trsv_dia_diag_fill_operation[])(const OPENSPBLAS_Number alpha,
                                                        const OPENSPBLAS_SPMAT_DIA *A,
                                                        const OPENSPBLAS_Number *x,
                                                        OPENSPBLAS_Number *y) = {
    trsv_dia_n_lo,
    trsv_dia_u_lo,
    trsv_dia_n_hi,
    trsv_dia_u_hi,
    trsv_dia_n_lo_trans,
    trsv_dia_u_lo_trans,
    trsv_dia_n_hi_trans,
    trsv_dia_u_hi_trans,
#ifdef COMPLEX
    trsv_dia_n_lo_conj, 
    trsv_dia_u_lo_conj, 
    trsv_dia_n_hi_conj, 
    trsv_dia_u_hi_conj, 
#endif
};

/*
* 
* Solve a set of equations in which a sparse matrix and a dense vector are multiplied
*
* details:
* diagsv_dia_n      Diagonal Matrics defined in dia storage format, including diagal
* diagsv_dia_u      Diagonal Matrics defined in dia storage format, all diagal elements are 1
*
* op(A)*y = alpha * x
* op: op(A) = A
*     op(A) = AT
*     op(A) = AH
*
* alpha     a scalor value
* x         a sparse vector in a compressed format of complex numbers
* y         a fully stored vector of real numbers
* op(A)     Data structure of sparse matrix
*
* input:
* alpha     a scalor value
* beta      a scalor value
* x         a sparse vector in a compressed format of complex numbers
* y         a fully stored vector of real numbers
* A         Data structure of sparse matrix
* 
* output:
* y         a fully stored vector of real numbers
*
*/

static openspblas_sparse_status_t (*diagsv_dia_diag[])(const OPENSPBLAS_Number alpha,
                                           const OPENSPBLAS_SPMAT_DIA *A,
                                           const OPENSPBLAS_Number *x,
                                           OPENSPBLAS_Number *y) = {
    diagsv_dia_n,
    diagsv_dia_u,
};

openspblas_sparse_status_t ONAME(const openspblas_sparse_operation_t operation, const OPENSPBLAS_Number alpha, const openspblas_sparse_matrix_t A, const struct openspblas_matrix_descr descr, /* openspblas_sparse_matrix_type_t + openspblas_sparse_fill_mode_t + openspblas_sparse_diag_type_t */
                                      const OPENSPBLAS_Number *x, OPENSPBLAS_Number *y)
{
    check_null_return(A->mat, OPENSPBLAS_SPARSE_STATUS_NOT_INITIALIZED);
    check_null_return(x, OPENSPBLAS_SPARSE_STATUS_NOT_INITIALIZED);
    check_null_return(y, OPENSPBLAS_SPARSE_STATUS_NOT_INITIALIZED);
    check_return(A->datatype != OPENSPBLAS_SPARSE_DATATYPE, OPENSPBLAS_SPARSE_STATUS_INVALID_VALUE);

#ifndef COMPLEX
    if(operation == OPENSPBLAS_SPARSE_OPERATION_CONJUGATE_TRANSPOSE)
        return OPENSPBLAS_SPARSE_STATUS_INVALID_VALUE;
#endif

    // Check if it is a square matrix 
    check_return(!check_equal_row_col(A),OPENSPBLAS_SPARSE_STATUS_INVALID_VALUE);

    if(A->format == OPENSPBLAS_SPARSE_FORMAT_CSR)
    {
        if (descr.type == OPENSPBLAS_SPARSE_MATRIX_TYPE_TRIANGULAR)
        {
            check_null_return(trsv_csr_diag_fill_operation[index3(operation, descr.mode, descr.diag, OPENSPBLAS_SPARSE_FILL_MODE_NUM, OPENSPBLAS_SPARSE_DIAG_TYPE_NUM)], OPENSPBLAS_SPARSE_STATUS_NOT_SUPPORTED);
            return trsv_csr_diag_fill_operation[index3(operation, descr.mode, descr.diag, OPENSPBLAS_SPARSE_FILL_MODE_NUM, OPENSPBLAS_SPARSE_DIAG_TYPE_NUM)](alpha, A->mat, x, y);
        }
        else if (descr.type == OPENSPBLAS_SPARSE_MATRIX_TYPE_DIAGONAL)
        {
            check_null_return(diagsv_csr_diag[descr.diag], OPENSPBLAS_SPARSE_STATUS_NOT_SUPPORTED);
            return diagsv_csr_diag[descr.diag](alpha, A->mat, x, y);
        }else
        {
            return OPENSPBLAS_SPARSE_STATUS_NOT_SUPPORTED;
        }
    }
    else if(A->format == OPENSPBLAS_SPARSE_FORMAT_BSR)
    {
        if (descr.type == OPENSPBLAS_SPARSE_MATRIX_TYPE_TRIANGULAR)
        {
            check_null_return(trsv_bsr_diag_fill_operation[index3(operation, descr.mode, descr.diag, OPENSPBLAS_SPARSE_FILL_MODE_NUM, OPENSPBLAS_SPARSE_DIAG_TYPE_NUM)], OPENSPBLAS_SPARSE_STATUS_NOT_SUPPORTED);
            return trsv_bsr_diag_fill_operation[index3(operation, descr.mode, descr.diag, OPENSPBLAS_SPARSE_FILL_MODE_NUM, OPENSPBLAS_SPARSE_DIAG_TYPE_NUM)](alpha, A->mat, x, y);
        }
        else if (descr.type == OPENSPBLAS_SPARSE_MATRIX_TYPE_DIAGONAL)
        {
            check_null_return(diagsv_bsr_diag[descr.diag], OPENSPBLAS_SPARSE_STATUS_NOT_SUPPORTED);
            return diagsv_bsr_diag[descr.diag](alpha, A->mat, x, y);
        }else
        {
            return OPENSPBLAS_SPARSE_STATUS_NOT_SUPPORTED;
        }       
    }
    else if(A->format == OPENSPBLAS_SPARSE_FORMAT_COO)
    {
        if (descr.type == OPENSPBLAS_SPARSE_MATRIX_TYPE_TRIANGULAR)
        {
            check_null_return(trsv_coo_diag_fill_operation[index3(operation, descr.mode, descr.diag, OPENSPBLAS_SPARSE_FILL_MODE_NUM, OPENSPBLAS_SPARSE_DIAG_TYPE_NUM)], OPENSPBLAS_SPARSE_STATUS_NOT_SUPPORTED);
            return trsv_coo_diag_fill_operation[index3(operation, descr.mode, descr.diag, OPENSPBLAS_SPARSE_FILL_MODE_NUM, OPENSPBLAS_SPARSE_DIAG_TYPE_NUM)](alpha, A->mat, x, y);
        }
        else if (descr.type == OPENSPBLAS_SPARSE_MATRIX_TYPE_DIAGONAL)
        {
            check_null_return(diagsv_coo_diag[descr.diag], OPENSPBLAS_SPARSE_STATUS_NOT_SUPPORTED);
            return diagsv_coo_diag[descr.diag](alpha, A->mat, x, y);
        }else
        {
            return OPENSPBLAS_SPARSE_STATUS_NOT_SUPPORTED;
        }       
    }
    else if(A->format == OPENSPBLAS_SPARSE_FORMAT_CSC)
    {
        if (descr.type == OPENSPBLAS_SPARSE_MATRIX_TYPE_TRIANGULAR)
        {
            check_null_return(trsv_csc_diag_fill_operation[index3(operation, descr.mode, descr.diag, OPENSPBLAS_SPARSE_FILL_MODE_NUM, OPENSPBLAS_SPARSE_DIAG_TYPE_NUM)], OPENSPBLAS_SPARSE_STATUS_NOT_SUPPORTED);
            return trsv_csc_diag_fill_operation[index3(operation, descr.mode, descr.diag, OPENSPBLAS_SPARSE_FILL_MODE_NUM, OPENSPBLAS_SPARSE_DIAG_TYPE_NUM)](alpha, A->mat, x, y);
        }
        else if (descr.type == OPENSPBLAS_SPARSE_MATRIX_TYPE_DIAGONAL)
        {
            check_null_return(diagsv_csc_diag[descr.diag], OPENSPBLAS_SPARSE_STATUS_NOT_SUPPORTED);
            return diagsv_csc_diag[descr.diag](alpha, A->mat, x, y);
        }else
        {
            return OPENSPBLAS_SPARSE_STATUS_NOT_SUPPORTED;
        }       
    }
    else if(A->format == OPENSPBLAS_SPARSE_FORMAT_SKY)
    {
        if (descr.type == OPENSPBLAS_SPARSE_MATRIX_TYPE_TRIANGULAR)
        {
            check_null_return(trsv_sky_diag_fill_operation[index3(operation, descr.mode, descr.diag, OPENSPBLAS_SPARSE_FILL_MODE_NUM, OPENSPBLAS_SPARSE_DIAG_TYPE_NUM)], OPENSPBLAS_SPARSE_STATUS_NOT_SUPPORTED);
            return trsv_sky_diag_fill_operation[index3(operation, descr.mode, descr.diag, OPENSPBLAS_SPARSE_FILL_MODE_NUM, OPENSPBLAS_SPARSE_DIAG_TYPE_NUM)](alpha, A->mat, x, y);
        }
        else if (descr.type == OPENSPBLAS_SPARSE_MATRIX_TYPE_DIAGONAL)
        {
            check_null_return(diagsv_sky_diag[descr.diag], OPENSPBLAS_SPARSE_STATUS_NOT_SUPPORTED);
            return diagsv_sky_diag[descr.diag](alpha, A->mat, x, y);
        }else
        {
            return OPENSPBLAS_SPARSE_STATUS_NOT_SUPPORTED;
        }       
    }
    else if(A->format == OPENSPBLAS_SPARSE_FORMAT_DIA)
    {
        if (descr.type == OPENSPBLAS_SPARSE_MATRIX_TYPE_TRIANGULAR)
        {
            check_null_return(trsv_dia_diag_fill_operation[index3(operation, descr.mode, descr.diag, OPENSPBLAS_SPARSE_FILL_MODE_NUM, OPENSPBLAS_SPARSE_DIAG_TYPE_NUM)], OPENSPBLAS_SPARSE_STATUS_NOT_SUPPORTED);
            return trsv_dia_diag_fill_operation[index3(operation, descr.mode, descr.diag, OPENSPBLAS_SPARSE_FILL_MODE_NUM, OPENSPBLAS_SPARSE_DIAG_TYPE_NUM)](alpha, A->mat, x, y);
        }
        else if (descr.type == OPENSPBLAS_SPARSE_MATRIX_TYPE_DIAGONAL)
        {
            check_null_return(diagsv_dia_diag[descr.diag], OPENSPBLAS_SPARSE_STATUS_NOT_SUPPORTED);
            return diagsv_dia_diag[descr.diag](alpha, A->mat, x, y);
        }else
        {
            return OPENSPBLAS_SPARSE_STATUS_NOT_SUPPORTED;
        }       
    }
    else
    {
        return OPENSPBLAS_SPARSE_STATUS_NOT_SUPPORTED;
    }  
}
