#ifndef UTILS_H_
#define UTILS_H_

#include "types.h"
#include <stddef.h>
#include <assert.h>
#include <vector>
#include <string>
#include <iostream>

using namespace std;

#define stoi(x) atoi(((string)x).c_str())
#define stod(x) atof(((string)x).c_str())

template <class T> class array_1d {
private:
	T* ptr;
	my_uint len;

public:
	array_1d(T* ptr, my_uint len) {
		this->ptr = ptr;
		this->len = len;
	}

	array_1d(my_uint len, T default_val) {
		this->ptr = new int[len];

		for (my_uint i = 0; i < len; i++) {
			ptr[i] = default_val;
		}
	}

	array_1d() {
		ptr = NULL;
		len = 0;
	}

	T& operator[](my_uint idx) const {
		// idx < len?
		assert(idx < len);
		return *(ptr+idx);
	}

	T safe_access(my_uint idx, T val) const {
		if (idx >= len) { return val; } else { return *(ptr+idx); }
	}

	my_uint length() const {
		return len;
	}

	void destroy() {
		delete [] ptr;

		ptr = NULL;

		len = 0;
	}
};

template <class T> class array_2d {
private:
	T* ptr;
	my_uint r;
	my_uint c;

public:
	array_2d(T* ptr, my_uint rows, my_uint cols) {
		this->ptr = ptr;
		this->r = rows;
		this->c = cols;
	}

	array_2d() {
		ptr = NULL;
		r = 0;
		c = 0;
	}

	array_1d<T> operator[](my_uint idx) const {
		// idx < rows?
		assert(idx < r);
		return array_1d<T>(ptr+c*idx, c);
	}

	my_uint rows() const {
		return r;
	}

	my_uint cols() const {
		return c;
	}

	void destroy()
	{
		delete [] ptr;

		ptr = NULL;

		r = 0; c = 0;
	}
};

template <class T> class array_3d {
private:
	T* ptr;
	my_uint s, c, r;

public:
	array_3d(T* ptr, my_uint slices, my_uint rows, my_uint cols) {
		this->ptr = ptr;
		this->c = cols;
		this->r = rows;
		this->s = slices;
	}

	T* get_ptr() {
		return ptr;
	}

	void destroy() {
		delete [] ptr;
		ptr = NULL;

		s = 0;
		r = 0;
		c = 0;
	}

	array_3d() {
		ptr = NULL;
		s = 0;
		r = 0;
		c = 0;
	}

	array_2d<T> operator[](my_uint idx) const {
		// idx < slices?
		assert(idx < s);
		return array_2d<T>(ptr+c*r*idx, r, c);
	}

	T& get_ref(my_uint i, my_uint j, my_uint k) {
		return *(ptr + c*r*i + c*j + k);
	}

	my_uint slices() const {
		return s;
	}

	my_uint rows() const {
		return r;
	}

	my_uint cols() const {
		return c;
	}
};

typedef array_1d<my_double> double_array_1d;
typedef array_2d<my_double> double_array_2d;
typedef array_3d<my_double> double_array_3d;

// TODO: there is a serious issue with memory leaks here, since we are creating a new
// array but never deleting it. maybe need to use "smart pointers" for that.
class memutils {

public:
	template <class T> static array_1d<T> create_1d_array(my_uint size,
			T default_value) {
		T* v = new T[size];

		for (my_uint i=0; i < size; i++) {
			v[i] = default_value;
		}

		return array_1d<T>(v, size);
	}

	template <class T> static array_2d<T> create_2d_array(my_uint rows,
			my_uint cols, T default_value) {
		T* v = new T[rows*cols];

		for (my_uint i=0; i < rows*cols; i++) {
			v[i] = default_value;
		}

		return array_2d<T>(v, rows, cols);
	}

	template <class T> static array_3d<T> create_3d_array(my_uint slices,
			my_uint rows, my_uint cols, T default_value) {
		T* v = new T[slices*rows*cols];

		for (my_uint i=0; i < slices*rows*cols; i++) {
			v[i] = default_value;
		}

		return array_3d<T>(v, slices, rows, cols);
	}

	static double_array_1d create_1d_double_array(my_uint size,
			my_double default_value);
	static double_array_2d create_2d_double_array(my_uint rows, my_uint cols,
			my_double default_value);
	static double_array_3d create_3d_double_array(my_uint slices, my_uint rows,
			my_uint cols, my_double default_value);
};

class string_utils {

public:
	static string EMPTY_STRING;

	static void split(vector<string> & theStringVectorOut, /* Altered/returned value */
	const string & theString, const string & theDelimiter,
			bool ignore_empty_slots = true) {
		assert(theDelimiter.size()> 0);

		size_t start = 0, end = 0;

		while (end != string::npos) {
			end = theString.find(theDelimiter, start);

			string s = theString.substr(start,
					(end == string::npos) ? string::npos : end - start);

			// If at end, use length=maxLength.  Else use length=end-start.
			if ((!ignore_empty_slots) || (s.compare(EMPTY_STRING))) {
				theStringVectorOut.push_back(s);
			}

			// If at end, use start=maxSize.  Else use start=end+delimiter.
			start
					= ( (end> (string::npos - theDelimiter.size()) ) ? string::npos
							: end + theDelimiter.size());
		}

	}
};
#endif /*UTILS_H_*/
