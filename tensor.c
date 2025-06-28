
#include"tensor.h"

#include<stdio.h>
#include<stdlib.h>
#include<mpfr.h>
#include<stdbool.h>
#include<string.h>
#include"error.h"



static const size_t sizeof_tensor_type[] = {
#define X(name, val, type) [val] = sizeof(type),
	TENSOR_TYPES_LIST
#undef X
};



tensor_t* tensor_init(enum tensor_type type, int order, int shape[static order]) {
	expect( sizeof(*shape) == sizeof(((tensor_t*){0})->shape), 
			"function %s() definition is not inline with struct definition.", __func__);

	int comps_count = 1;
	for (int i = 0; i < order; i++) {
		comps_count *= shape[i];
	}

	tensor_t* t = malloc(sizeof(*t) + comps_count *sizeof_tensor_type[type]);
	expect(t != NULL, "malloc failed.");
	
	*t = (tensor_t) {
		.type = type,
		.order = order,
		.shape = malloc(order *sizeof(*shape)),
		.comps_len = comps_count,
		.comps = t + 1,
	};

	expect(t->shape != NULL, "malloc failed.");


	memcpy(t->shape, shape, order *sizeof(*shape));
	return t;
}

void tensor_clear(tensor_t* t) {
	free(t->shape);
	free(t);
	return;
}

tensor_t* tensor_load(char* filename) {
	FILE* fp = fopen(filename, "rb");
	expect(fp, "failed to open file %s: %s", filename, strerror(errno));

	
	tensor_t tmp = { 0 };
	fread(&tmp, sizeof(tmp), 1, fp);

	tmp.shape = malloc(tmp.order *sizeof(*tmp.shape));
	expect(tmp.shape != NULL, "malloc failed.");
	fread(tmp.shape, sizeof(*tmp.shape), tmp.order, fp);


	tensor_t* t = tensor_init(tmp.type, tmp.order, tmp.shape);
	fread(t->comps, sizeof_tensor_type[t->type], t->comps_len, fp);


	free(tmp.shape);	
	fclose(fp);
	return t;
}

void tensor_save(tensor_t t[static 1], char* filename) {
	expect(filename != NULL, "no filename provided.");

	FILE* fp = fopen(filename, "wb");
	expect(fp, "failed to open file %s: %s", filename, strerror(errno));

	fwrite(t, sizeof(*t), 1, fp);
	fwrite(t->shape, sizeof(*t->shape), t->order, fp);
	fwrite(t->comps, sizeof_tensor_type[t->type], t->comps_len, fp);

	fclose(fp);
	return;
}

static int* __result_shape(tensor_t* t1, tensor_t* t2) {
	int min_order = 0;
	int max_order = 0;
	int* max_shape = NULL;
	if (t1->order > t2->order) {
		min_order = t2->order;
		max_order = t1->order;
		max_shape = t1->shape;
	} else {
		min_order = t1->order;
		max_order = t2->order;
		max_shape = t2->shape;
	}
	
	
	int* shape = malloc(max_order *sizeof(*shape));
	
	for (int i = 1; i <= min_order; i++) {
		int t1_dim = t1->shape[t1->order -i];
		int t2_dim = t2->shape[t2->order -i];
		
		
		if (t1_dim == t2_dim || t1_dim == 1) {
			shape[max_order -i] = t2_dim;
		} else if (t2_dim == 1) {
			shape[max_order -i] = t1_dim;
		} else {
			free(max_shape);
			fatal("invalid shape.");
		}
	}
	memcpy(shape, max_shape, (max_order - min_order) *sizeof(*max_shape));
	
	return shape;
}







typedef struct {
	tensor_t* t;
	int casted_order;
	size_t iterations_remaining;
	int* padded_shape;
	int* casted_shape;
	int* index;
	int casted_index[];
} tensor_iterator_t;

tensor_iterator_t* titer_init(tensor_t* t, int casted_order, int casted_shape[static casted_order]) {
	size_t iterations_remaining = 1;
	
	for (int i = 0; i < casted_order; i++) {
		iterations_remaining *= casted_shape[i];
	}

	tensor_iterator_t* iter = malloc(sizeof(*iter) + 3* casted_order*sizeof(*iter->casted_index));
	*iter = (tensor_iterator_t) {
		.t = t, 
		.casted_order      = casted_order,
		.iterations_remaining = iterations_remaining,
		.padded_shape      = ((int*)(iter + 1)) + 2*casted_order,
		.casted_shape      = casted_shape,
		.index             = ((int*)(iter + 1)) + casted_order * 3,
		//.casted_index      =  (int*)(iter + 1)
	};
	memset(iter->casted_index, 0, 3* casted_order*sizeof(*iter->casted_index));
	
	for (int i = 0; i < casted_order - t->order; i++) {
		iter->padded_shape[i] = 1;
	}
	memcpy(iter->padded_shape + casted_order - t->order, t->shape, t->order*sizeof(*t->shape));

	return iter;
}
void titer_clear(tensor_iterator_t* iter) {
	free(iter);
	return;
}

bool titer_has_next(tensor_iterator_t iter[static 1]) { // TODO inline
	if (iter->iterations_remaining != 0) 
		return true;
	else
		return false;
}

void titer_next(tensor_iterator_t iter[static 1]) {	

	int i = 0;

	while (i < iter->casted_order && iter->casted_index[i]+1 >= iter->casted_shape[i]) {
		iter->casted_index[i] = 0;
		iter->index[i] = 0;
		
		i++;
	}
	iter->casted_index[i]++;
	iter->index[i] = iter->casted_index[i] % iter->padded_shape[i];


	if (iter->casted_index[iter->casted_order-1] >= iter->casted_shape[iter->casted_order-1]) {
		fatal("the iteration has no more elements.");
	}

	expect(iter->iterations_remaining != 0, "iterations remaining are about to be decremented too much.");
	iter->iterations_remaining--;	
	return;
}



int titer_flatten_index(tensor_iterator_t iter[static 1]) {

	int ret = 0;
	int stride = 1;
	
	for (int i = iter->casted_order-1; i >= 0; i--) {
		ret += iter->index[i] * stride;
		stride *= iter->padded_shape[i];
	}

	return ret;
}




inline void* tensor_get_comp(tensor_t t[static 1], size_t offset) {
	return (int8_t*)t->comps + offset*sizeof_tensor_type[t->type];
}

static tensor_t* tensor_scalar_operation(void (*op)(void*, void*, void*), tensor_t op1[static 1], tensor_t op2[static 1]) {
	expect(op1->type == op2->type, "in %s(), expected tensors of same type.", __func__);

	int* res_shape = __result_shape(op1, op2);
	tensor_t* res = tensor_init(
			op1->type, 
			(op1->order > op2->order) ? op1->order : op2->order, 
			res_shape
	);	

	tensor_iterator_t* op1_iter = titer_init(op1, res->order, res->shape);
	tensor_iterator_t* op2_iter = titer_init(op2, res->order, res->shape);
	int res_index = 0;

	while (titer_has_next(op1_iter) && titer_has_next(op2_iter)) {
		int op1_index = titer_flatten_index(op1_iter);
		int op2_index = titer_flatten_index(op2_iter);

		op(tensor_comp_ref(res, res_index), tensor_comp_ref(op1, op1_index),tensor_comp_ref(op2, op2_index));
		// res->comps[res_index] = op1->comps[op1_index] + op2->comps[op2_index];

		res_index++;
		titer_next(op1_iter);
		titer_next(op2_iter);
	}
	
	expect(titer_has_next(op1_iter) == titer_has_next(op2_iter),
			"iterators did not finish in sync. Check result size or increment procedure.");

	titer_clear(op1_iter);
	titer_clear(op2_iter);
	return res;
}

