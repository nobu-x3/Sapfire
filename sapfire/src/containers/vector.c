#include "vector.h"
#include "core/logger.h"
#include "core/sfmemory.h"

void *_vector_create (u64 length, u64 stride) {
	u64 header_size = VECTOR_HEADER_LENGTH * sizeof (u64);
	u64 vec_size	= stride * length;
	u64 *new_vec	= sfalloc (header_size + vec_size, MEMORY_TAG_VECTOR);
	new_vec[VECTOR_CAPACITY] = length;
	new_vec[VECTOR_LENGTH]	 = 0;
	new_vec[VECTOR_STRIDE]	 = stride;
	return (void *)(new_vec + VECTOR_HEADER_LENGTH);
}

void _vector_destroy (void *vector) {
	u64 *header		= (u64 *)vector - VECTOR_HEADER_LENGTH;
	u64 header_size = VECTOR_HEADER_LENGTH * sizeof (u64);
	u64 total_size =
		header_size + header[VECTOR_CAPACITY] * header[VECTOR_STRIDE];
	sffree (header, total_size, MEMORY_TAG_VECTOR);
}

u64 _vector_field_get (void *vector, u64 field) {
	u64 *header = (u64 *)vector - VECTOR_HEADER_LENGTH;
	return header[field];
}

void _vector_field_set (void *vector, u64 field, u64 val) {
	u64 *header	  = (u64 *)vector - VECTOR_HEADER_LENGTH;
	header[field] = val;
}

void *_vector_resize (void *vector) {
	u64 length = vector_len (vector);
	u64 stride = vector_stride (vector);
	void *temp = _vector_create (
		(VECTOR_RESIZE_FACTOR * vector_capacity (vector)), stride);
	sfmemcpy (temp, vector, length * stride);
	_vector_field_set (temp, VECTOR_LENGTH, length);
	_vector_destroy (vector);
	return temp;
}

void _vector_pop (void *vector, void *dest) {
	u64 len	   = vector_len (vector);
	u64 stride = vector_stride (vector);
	u64 addr   = (u64)vector + ((len - 1) * stride);
	sfmemcpy (dest, (void *)addr, stride);
	_vector_field_set (vector, VECTOR_LENGTH, len - 1);
}

void *_vector_push (void *vector, const void *val_ptr) {
	u64 len	   = vector_len (vector);
	u64 stride = vector_stride (vector);
	if (len >= vector_capacity (vector)) { vector = _vector_resize (vector); }
	u64 addr = (u64)vector + (len * stride);
	sfmemcpy ((void *)addr, val_ptr, stride);
	_vector_field_set (vector, VECTOR_LENGTH, len + 1);
	return vector;
}

void *_vector_pop_at (void *vector, u64 index, void *dest) {
	u64 len	   = vector_len (vector);
	u64 stride = vector_stride (vector);
	if (index >= len) {
		SF_ERROR ("Index out of range! Length: %i, index: %i", len, index);
		return vector;
	}
	u64 addr = (u64)vector;
	sfmemcpy (dest, (void *)(addr + (index * stride)), stride);
	if (index != len - 1) {
		sfmemcpy ((void *)(addr + (index * stride)),
				  (void *)(addr + ((index + 1) * stride)),
				  (len - index) * stride);
	}
	_vector_field_set (vector, VECTOR_LENGTH, len - 1);
	return vector;
}

void *_vector_insert_at (void *vector, u64 index, const void *val_ptr) {
	u64 len	   = vector_len (vector);
	u64 stride = vector_stride (vector);
	if (index >= len) {
		SF_ERROR ("Index out of range! Length: %i, index: %i", len, index);
		return vector;
	}
	if (len >= vector_capacity (vector)) { _vector_resize (vector); }
	u64 addr = (u64)vector;
	if (index != len - 1) {
		sfmemcpy ((void *)(addr + ((index + 1) * stride)),
				  (void *)(addr + (index * stride)), (len - index) * stride);
	}
	sfmemcpy ((void *)(addr + (index * stride)), val_ptr, stride);
	_vector_field_set (vector, VECTOR_LENGTH, len + 1);
	return vector;
}
