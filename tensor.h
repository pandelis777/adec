#pragma once

#include<stddef.h>

enum tensor_type {
	TENSOR_FLOAT = 0,
	TENSOR_MPFR,
	TENSOR_DOUBLE,
	TENSOR_BOOL,
	TENSOR_INT
};
#define TENSOR_FLOAT (0)
#define TENSOR_MPFR (1)
#define TENSOR_DOUBLE (2)
#define TENSOR_BOOL (3)
#define TENSOR_INT (4)


typedef struct {
	enum tensor_type type;
	int order;
	int* shape;
	size_t comps_len;
	void* comps;
} tensor_t;


tensor_t* tensor_init(enum tensor_type type, int order, int shape[static order]);
void tensor_clear(tensor_t* t);

tensor_t* tensor_load(char* filename);
void tensor_save(tensor_t t[static 1], char* filename);

tensor_t* tensor_add(tensor_t res[static 1], tensor_t op1[static 1], tensor_t op2[static 1]);
tensor_t* tensor_sub(tensor_t res[static 1], tensor_t op1[static 1], tensor_t op2[static 1]);
tensor_t* tensor_mul(tensor_t res[static 1], tensor_t op1[static 1], tensor_t op2[static 1]);
tensor_t* tesnor_div(tensor_t res[static 1], tensor_t op1[static 1], tensor_t op2[static 1]);
tensor_t* tensor_neg(tensor_t res[static 1], tensor_t op[static 1]);

tensor_t* matrix_transpose(tensor_t A_T[static 1], tensor_t A[static 1]);
tensor_t* matrix_inv(tensor_t A_inv[static 1], tensor_t A[static 1]);
tensor_t* matrix_mul(tensor_t AB[static 1], tensor_t A[static 1], tensor_t B[static 1]);


