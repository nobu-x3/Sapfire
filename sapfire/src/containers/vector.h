#pragma once

#include "defines.h"

#define VECTOR_DEFAULT_CAPACITY 1
#define VECTOR_RESIZE_FACTOR	2

/*
Memory layout
u64 capacity = number elements that can be held
u64 length = number of elements currently contained
u64 stride = size of each element in bytes
void* elements
*/
enum { VECTOR_CAPACITY, VECTOR_LENGTH, VECTOR_STRIDE, VECTOR_HEADER_LENGTH };

SAPI void* _vector_create (u64 length, u64 stride);
SAPI void _vector_destroy (void* vector);
SAPI u64 _vector_field_get (void* vector, u64 field);
SAPI void _vector_field_set (void* vector, u64 field, u64 val);
SAPI void* _vector_resize (void* vector);
SAPI void _vector_pop (void* vector, void* dest);
SAPI void* _vector_push (void* vector, const void* val_ptr);
SAPI void* _vector_pop_at (void* vector, u64 index, void* dest);
SAPI void* _vector_insert_at (void* vector, u64 index, const void* val_ptr);
#define vector_create(type)                                                    \
	_vector_create (VECTOR_DEFAULT_CAPACITY, sizeof (type))
#define vector_reserve(type, capacity) _vector_create (capacity, sizeof (type))
#define vector_destroy(vector_ptr)	   _vector_destroy (vector_ptr)
#define vector_push(vector_ptr, value)                                         \
	{                                                                          \
		typeof (value) temp = value;                                           \
		vector_ptr			= _vector_push (vector_ptr, &temp);                \
	}
#define vector_pop(vector_ptr, val) _vector_pop (vector_ptr, val)
#define vector_insert_at(vector_ptr, index, val)                               \
	{                                                                          \
		typeof (val) temp = val;                                               \
		vector_ptr		  = _vector_insert_at (vector_ptr, index, &temp);      \
	}
#define vector_pop_at(vector_ptr, index, val_ptr)                              \
	_vector_pop_at (vector_ptr, index, val_ptr)
#define vector_clear(vector_ptr)                                               \
	_vector_field_set (vector_ptr, VECTOR_LENGTH, 0)
#define vector_len(vector_ptr) _vector_field_get (vector_ptr, VECTOR_LENGTH)
#define vector_capacity(vector_ptr)                                            \
	_vector_field_get (vector_ptr, VECTOR_CAPACITY)
#define vector_stride(vector_ptr) _vector_field_get (vector_ptr, VECTOR_STRIDE)
#define vector_set_length(vector_ptr, length)                                  \
	_vector_field_set (vector_ptr, VECTOR_LENGTH, length)
