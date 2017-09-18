// TODO
// need to create own types for int, double, etc.: my_int, my_double, etc.

// todo: need to do a range check

#include "types.h"
#include "utils.h"



string string_utils::EMPTY_STRING = "";

double_array_1d memutils::create_1d_double_array(my_uint size,
		my_double default_value) {
	my_double* v = new my_double[size];

	for (my_uint i=0; i < size; i++) {
		v[i] = default_value;
	}

	return double_array_1d(v, size);
}

double_array_2d memutils::create_2d_double_array(my_uint rows,
		my_uint cols, my_double default_value) {
	my_double* v = new my_double[rows*cols];

	for (my_uint i=0; i < rows*cols; i++) {
		v[i] = default_value;
	}

	return double_array_2d(v, rows, cols);
}

double_array_3d memutils::create_3d_double_array(my_uint slices,
		my_uint rows, my_uint cols, my_double default_value) {
	my_double* v = new my_double[slices*rows*cols];

	for (my_uint i=0; i < slices*rows*cols; i++) {
		v[i] = default_value;
	}

	return double_array_3d(v, slices, rows, cols);
}
