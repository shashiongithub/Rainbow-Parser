#include "lattice.h"
#include "grammar.h"
#include "cnf_io.h"
#include <assert.h>

template <class E> E& string_lattice<E>::symbol_at(my_uint s1, my_uint s2) {

	assert(this->span_length(s1,s2) == 1);
	return this->str[s1];
}

template <class E> void string_lattice<E>::_init_lattice(array_1d<E>& sentence) {
	n = sentence.length();
	str = sentence;
}

template<class E> template<class F> string_lattice<F> string_lattice<E>::create_data_lattice(
		F default_val) {
	array_1d<F> arr(str.length(), default_val);
	return string_lattice<F>(arr);
}

template <class E> string_lattice<E>::string_lattice(array_1d<E>& sentence) {
	_init_lattice(sentence);
}

template <class E> my_uint string_lattice<E>::span_length(my_uint i, my_uint j) {
	assert(j >= i);
	return j - i;
}

template <class E> iterable_string_lattice<E>::iterable_string_lattice(
		array_1d<E>& sentence, cnf_span_bag* bag) :
	string_lattice<E>(sentence) {

	midstate_iterator_count = 0;

	this->constrain_bag = bag;

	iterating = false;
}

template <class E> void iterable_string_lattice<E>::reset_span_iterator(
		int iter_type) {

	this->iter_type = iter_type;

	if (iter_type == abstract_lattice<E>::iterator_forward) {
		left_idx = -1;
		right_idx = 0;
		current_len = 1;

		iterating = true;
	} else if (iter_type == abstract_lattice<E>::iterator_reverse) {
		left_idx = -1;
		right_idx = 0;
		current_len = this->n+1;

		iterating = true;
	} else {
		assert(false);
	}
}

template <class E> bool iterable_string_lattice<E>::next_span() {

	// assert iterating

	if (iter_type == abstract_lattice<E>::iterator_forward) {
		left_idx++;
		right_idx = left_idx + current_len;

		if (right_idx > this->n) {
			left_idx = 0;
			current_len++;
			right_idx = left_idx + current_len;

			if (right_idx >= this->n + 1) {
				iterating = false;

				return false;
			}
		}
	} else if (iter_type == abstract_lattice<E>::iterator_reverse) {
		left_idx++;
		right_idx = left_idx + current_len;

		if (right_idx > this->n) {
			left_idx = 0;
			current_len--;
			right_idx = left_idx + current_len;

			if (current_len == 0) {
				iterating = false;

				return false;
			}
		}
	} else {
		assert(false);

		return false;
	}

	return true;
}

template <class E> my_uint iterable_string_lattice<E>::left_state() {
	return left_idx;
}

template <class E> my_uint iterable_string_lattice<E>::right_state() {
	return right_idx;
}

template <class E> mid_pointer_iterator_struct iterable_string_lattice<E>::reset_midstates_iterator(my_uint i, my_uint j, int typ, bool do_not_use_constrain_bag) {

	mid_pointer_iterator_struct s;

	if ((constrain_bag == NULL) || (do_not_use_constrain_bag)) {
		s.is_constrained = false;

		if (typ == abstract_lattice<E>::midstate_iterator_no_end_inclusive) {
			s.mid_point = i;
			s.last_mid_point = j;
		} else if (typ == abstract_lattice<E>::midstate_iterator_first_inclusive) {
			s.mid_point = i-1;
			s.last_mid_point = j;
		} else if (typ == abstract_lattice<E>::midstate_iterator_last_inclusive) {
			s.mid_point = i;
			s.last_mid_point = j + 1;
		} else if (typ == abstract_lattice<E>::midstate_iterator_both_inclusive) {
			s.mid_point = i-1;
			s.last_mid_point = j + 1;
		} else {
			assert(false);
		}
	} else {
		s.is_constrained = true;

		if (typ == abstract_lattice<E>::midstate_iterator_no_end_inclusive) {
			s.constrained_points[0] = constrain_bag->mid_point(i, j);
			s.last_mid_point = 1;
		} else if (typ == abstract_lattice<E>::midstate_iterator_first_inclusive) {
			s.constrained_points[1] = i;
			s.constrained_points[0] = constrain_bag->mid_point(i, j);

			s.last_mid_point = 2;
		} else if (typ == abstract_lattice<E>::midstate_iterator_last_inclusive) {
			s.constrained_points[1] = constrain_bag->mid_point(i, j);
			s.constrained_points[0] = j;

			s.last_mid_point = 2;
		} else if (typ == abstract_lattice<E>::midstate_iterator_both_inclusive) {
			s.constrained_points[2] = i;
			s.constrained_points[1] = constrain_bag->mid_point(i, j);
			s.constrained_points[0] = j;

			s.last_mid_point = 3;
		} else {
			assert(false);
		}
	}

	return s;
}

template <class E> bool iterable_string_lattice<E>::next_midstate(
		mid_pointer_iterator_struct& num) {

	if (num.is_constrained) {
		num.last_mid_point--;

		if (num.last_mid_point < 0) {
			return false;
		}

		return true;
	} else {
		num.mid_point++;

		if (num.mid_point < num.last_mid_point) {
			return true;
		} else {
			return false;
		}
	}
}

template <class E> my_uint iterable_string_lattice<E>::mid_state(
		mid_pointer_iterator_struct& num) {

	if (num.is_constrained) {
		return num.constrained_points[num.last_mid_point];
	} else {
		return num.mid_point;
	}
}

template <class E> void iterable_string_lattice<E>::reset_incoming_states(
		my_uint target_state) {
	iterating_incoming_state = true;
	this->target_state = target_state-1;
	if (target_state == 0) {
		iterating_incoming_state = false;
	}
}

template <class E> bool iterable_string_lattice<E>::next_incoming_state() {
	bool b = iterating_incoming_state;

	iterating_incoming_state = false;

	return b;
}

template <class E> bool iterable_string_lattice<E>::is_root_span(int i, int j)
{
	return (this->span_length(i, j) == this->length());
}

template <class E> bool iterable_string_lattice<E>::has_span_length_1(my_uint i, my_uint j)
{
	return (this->span_length(i, j) == 1);
}

template <class E> bool iterable_string_lattice<E>::has_span_length_1plus(my_uint i, my_uint j)
{
	return (this->span_length(i, j) > 1);
}


template <class E> my_uint iterable_string_lattice<E>::incoming_state() {
	return this->target_state;
}

template class abstract_lattice<vocab_symbol>;
template class string_lattice<vocab_symbol>;
template class iterable_string_lattice<vocab_symbol>;

template class abstract_lattice<cnf_io<my_double>::sym2double>;
template class string_lattice<cnf_io<my_double>::sym2double>;
template class iterable_string_lattice<cnf_io<my_double>::sym2double>;

template class abstract_lattice<cnf_io_simple_tensor::sym2double>;
template class string_lattice<cnf_io_simple_tensor::sym2double>;
template class iterable_string_lattice<cnf_io_simple_tensor::sym2double>;

template class abstract_lattice<cnf_io_viterbi::sym2double>;
template class string_lattice<cnf_io_viterbi::sym2double>;
template class iterable_string_lattice<cnf_io_viterbi::sym2double>;
