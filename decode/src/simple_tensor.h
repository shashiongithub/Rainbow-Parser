#ifndef SIMPLE_TENSOR_H_
#define SIMPLE_TENSOR_H_

#include "utils.h"
#include <sstream>
#include <iostream>
#include <math.h>

using namespace std;

class simple_tensor {
private:
	double_array_1d vec;
	double_array_3d tensor;
	bool is_vec, is_empty;

public:
	simple_tensor() {
		is_empty = true;
	}

	void destroy() {
		vec.destroy();
		tensor.destroy();
	}

	simple_tensor(double v, bool as_tensor = false) {
		// behaves like a scalar
		if (as_tensor) {
			tensor = memutils::create_3d_double_array(1, 1, 1, v);
			is_vec = false;
			is_empty = false;
		} else {
			vec = memutils::create_1d_double_array(1, v);
			is_vec = true;
			is_empty = false;
		}

		is_empty = false;
	}

	simple_tensor(int s, int r, int c, double v = 0.0) {
		tensor = memutils::create_3d_double_array(s, r, c, v);
		is_vec = false;
		is_empty = false;
	}

	simple_tensor(int s, double v = 0.0) {
		vec = memutils::create_1d_double_array(s, v);
		is_vec = true;
		is_empty = false;
	}

	void inc_scalar(double v) {
		assert(!empty());

		if (is_vector()) {
			for (auto i=0; i < vec.length(); i++) {
				vec[i] += v;
			}
		} else if (is_tensor()) {
			for (auto i=0; i < tensor.slices(); i++) {
				for (auto j=0; j < tensor.rows(); j++) {
					for (auto k=0; k < tensor.cols(); k++) {
						tensor[i][j][k] += v;
					}
				}
			}
		}
	}

	bool is_vector() const {
		return !is_empty && is_vec;
	}

	bool is_tensor() const {
		return !is_empty && !is_vec;
	}

	bool empty() const {
		return is_empty;
	}

	my_double get(int i, int j, int k) const {
		assert(is_tensor());

		return tensor[i][j][k];
	}

	void set(int i, int j, int k, my_double v) {
		assert(is_tensor());

		tensor[i][j][k] = v;
	}

	void mul_by_scalar(my_double v)
	{
		if (is_tensor()) {
			for (int i=0; i < tensor.slices(); i++) {
				for (int j=0; j < tensor.rows(); j++) {
					for (int k=0; k < tensor.cols(); k++) {
						tensor[i][j][k] *= v;
					}
				}
			}
		}

		if (is_vector()) {
			for (int i=0; i < vec.length(); i++) {
				vec[i] *= v;
			}
		}
	}

	double sum() {
		double v = 0.0;

		if (is_vector()) {
			for (int i=0; i < vec.length(); i++) {
				v = v + vec[i];
			}

			return v;
		}

		if (is_tensor()) {
			for (int i=0; i < tensor.slices(); i++) {
				for (int j=0; j < tensor.rows(); j++) {
					for (int k=0; k < tensor.cols(); k++) {
						v = v + tensor[i][j][k];
					}
				}
			}

			return v;
		}

		return 0.0;
	}

	void divide(double v) {
		if (is_vector()) {
			for (int i=0; i < vec.length(); i++) {
				vec[i] /= v;
			}
		}

		if (is_tensor()) {
			for (int i=0; i < tensor.slices(); i++) {
				for (int j=0; j < tensor.rows(); j++) {
					for (int k=0; k < tensor.cols(); k++) {
						tensor[i][j][k] /= v;
					}
				}
			}
		}
	}

	my_double get(int i) const {
		assert(is_vector());

		return vec[i];
	}

	void set(int i, my_double v) {
		assert(is_vector());

		vec[i] = v;
	}

	int cols() const {
		if (empty()) {
			return 0;
		}

		if (is_vector()) {
			return vec.length();
		} else {
			return tensor.cols();
		}
	}

	int rows() const {
		if (empty())
			return 0;

		assert(is_tensor());

		return tensor.rows();
	}

	int slices() const {
		if (empty())
			return 0;

		assert(is_tensor());

		return tensor.slices();
	}

	bool equals_zero() {
		if (empty())
			return true;

		/*	if (is_tensor()) {
		 for (int i=0; i<tensor.slices(); i++) {
		 for (int j=0; j<tensor.rows(); j++) {
		 for (int k=0; k<tensor.cols(); k++) {
		 if (tensor[i][j][k] != 0)
		 return false;
		 }
		 }
		 }
		 }

		 if (is_vector()) {
		 for (int i=0; i<vec.length(); i++) {
		 if (vec[i] != 0)
		 return false;
		 }
		 }

		 return true;*/

		return false;
	}

	void plus_mul_yz(simple_tensor& a, simple_tensor& b, simple_tensor& c) {

		if (empty()) {
			vec = memutils::create_1d_double_array(c.slices(), 0.0);
			is_vec = true;
			is_empty = false;
		}

		assert(is_vector());

		assert(cols() == c.slices());

		for (int i=0; i < a.cols(); i++) {
			my_double a_v = a.get(i);

			if (a_v != 0) {

				for (int j=0; j < b.cols(); j++) {
					my_double b_v = b.get(j);

					if (b_v != 0) {
						for (int k=0; k < c.slices(); k++) {
							vec[k] = vec[k] + a_v * b_v * c.get(k, i, j);
						}
					}
				}
			}
		}
	}

	void plus_mul_xy(simple_tensor& a, simple_tensor& b, simple_tensor& c) {

		if (empty()) {
			vec = memutils::create_1d_double_array(c.cols(), 0.0);
			is_vec = true;
			is_empty = false;
		}

		assert(is_vector());

		assert(cols() == c.cols());

		for (int i=0; i < a.cols(); i++) {
			my_double a_v = a.get(i);

			if (a_v != 0) {
				for (int j=0; j < b.cols(); j++) {

					my_double b_v = b.get(j);

					if (b_v != 0) {

						for (int k=0; k < c.cols(); k++) {
							vec[k] = vec[k] + a_v * b_v * c.get(i, j, k);
						}
					}
				}
			}
		}
	}

	void plus_mul_xz(simple_tensor& a, simple_tensor& b, simple_tensor& c) {

		if (empty()) {
			vec = memutils::create_1d_double_array(c.rows(), 0.0);
			is_vec = true;
			is_empty = false;
		}

		assert(is_vector());

		assert(cols() == c.rows());

		for (int i=0; i < a.cols(); i++) {
			my_double a_v = a.get(i);

			if (a_v != 0) {
				for (int j=0; j < b.cols(); j++) {
					my_double b_v = b.get(j);

					if (b_v != 0) {
						for (int k=0; k < c.rows(); k++) {
							vec[k] = vec[k] + a_v * b_v * c.get(i, k, j);
						}
					}
				}
			}
		}
	}

	void plus_vector(simple_tensor a, simple_tensor b, double v) {
		assert(a.cols() == b.cols());

		assert(!is_tensor());

		if (is_empty) {
			vec = memutils::create_1d_double_array(a.cols(), 0.0);
			is_vec = true;
			is_empty = false;
		}

		for (int i=0; i < a.cols(); i++) {
			vec[i] = vec[i] + a.get(i) * b.get(i) / v;
		}
	}

	void plus_vector(simple_tensor a) {
		assert(!is_tensor());

		if (is_empty) {
			vec = memutils::create_1d_double_array(a.cols(), 0.0);
			is_vec = true;
			is_empty = false;
		}

		assert(a.cols() == cols());

		for (int i=0; i < a.cols(); i++) {
			vec[i] = vec[i] + a.get(i);
		}
	}

	my_double sum_slice(int h) {
		if (is_empty) {
			return 0.0;
		}

		double v = 0.0;

		assert(is_tensor());

		for (int i=0; i < tensor.rows(); i++) {
			for (int j=0; j < tensor.cols(); j++) {
				v += tensor[h][i][j];
			}
		}

		return v;
	}

	void divide_slice(int h, my_double v) {
		if (is_empty) {
			return;
		}

		assert(is_tensor());

		for (int i=0; i < tensor.rows(); i++) {
			for (int j=0; j < tensor.cols(); j++) {
				tensor.get_ref(h, i, j) /= v;
			}
		}
	}

	void divide_vector(simple_tensor a) {
		assert(!is_tensor());

		if (is_empty) {
			vec = memutils::create_1d_double_array(a.cols(), 0.0);
			is_vec = true;
			is_empty = false;
		}

		assert(a.cols() == cols());

		for (int i=0; i < a.cols(); i++) {
			vec[i] = vec[i] / a.get(i);
		}
	}

	void plus_tensor(simple_tensor t, simple_tensor a, simple_tensor b,
			simple_tensor c, double v) {

		bool fastRet = false;

		if (a.cols() == 0) {
			cerr<<"Warning: plussing tensor with a.cols == 0"<<endl;
			fastRet = true;
		}
		if (b.cols() == 0) {
			cerr<<"Warning: plussing tensor with b.cols == 0"<<endl;
			fastRet = true;
		}
		if (c.cols() == 0) {
			cerr<<"Warning: plussing tensor with c.cols == 0"<<endl;
			fastRet = true;
		}

		if (!fastRet) {
			assert(t.slices() == a.cols());
			assert(t.rows() == b.cols());
			assert(t.cols() == c.cols());
		}

		assert(!is_vector());

		if (is_empty) {
			tensor = memutils::create_3d_double_array(t.slices(), t.rows(),
					t.cols(), 0.0);
			is_vec = false;
			is_empty = false;
		}

		if (fastRet) { return; }

		my_double* p = tensor.get_ptr();
		my_double* src_p = t.tensor.get_ptr();

		my_uint t_slices = t.slices();
		my_uint t_rows = t.rows();
		my_uint t_cols = t.cols();
		
		for (int i=0; i < t_slices; i++) {
			double a_v = a.get(i);

			//if (a_v > 0) {
				for (int j=0; j < t_rows; j++) {
					double b_v = b.get(j);
					
					//if (b_v > 0) {
						for (int k=0; k < t_cols; k++) {
							double v_ = *p + (*src_p * a_v * b_v * c.get(k))/v;

							if (isinf(v_)) {
								cerr<<"internal error, plus_tensor leads to infinity: a_v="<<a_v<<" b_v="<<b_v<<" c_v="<<c.get(k)<<" div="<<v<<endl;
							} else {
								*p = v_;
							}

							p++;
							src_p++;
						}
					//}
				}
			//}
		}
	}

	friend ostream &operator<<(ostream &output, const simple_tensor &a) {

		assert(a.empty() || a.is_vector());

		if (a.empty()) {
			output<<"Z";
		}

		for (int i=0; i < a.cols(); i++) {
			if (i> 0) {
				output<<",";
			}

			output<<a.get(i);
		}

		return output;
	}

	friend istream &operator>>(istream &input, simple_tensor& a) {
		assert(false);

		return input;
	}

};

#endif /*SIMPLE_TENSOR_H_*/
