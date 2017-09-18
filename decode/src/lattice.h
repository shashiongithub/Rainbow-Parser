#ifndef LATTICE_H_
#define LATTICE_H_

#include "utils.h"
#include "grammar.h"
#include "span_bag.h"

class mid_pointer_iterator_struct {
public:
		int mid_point;
		int last_mid_point;
		
		bool is_constrained;
		
		int constrained_points[3];
};

template <class E> class abstract_lattice {
public:

	typedef enum {iterator_reverse, iterator_forward} lattice_iterator_type;
	typedef enum {midstate_iterator_no_end_inclusive, midstate_iterator_last_inclusive, midstate_iterator_first_inclusive, midstate_iterator_both_inclusive}
			lattice_midstate_iterator_type;

	virtual ~abstract_lattice()
	{
		
	}

	virtual void set_span_bag(cnf_span_bag* bag) { assert(false); }
	
	virtual bool is_root_span(int i, int j) { assert(false); }

        virtual bool has_span_length_1(my_uint i, my_uint j) { assert(false); }

        virtual bool has_span_length_1plus(my_uint i, my_uint j) { assert(false); }

	virtual my_uint mid_state(mid_pointer_iterator_struct& num) { assert(false); }

	virtual my_uint length() const { assert(false); }
	
	virtual my_uint span_length(my_uint i, my_uint j) { assert(false); }

	virtual bool next_midstate(mid_pointer_iterator_struct& num)  { assert(false); }

	virtual void reset_span_iterator(int iter_type) { assert(false); }

	virtual bool next_span() { assert(false); }

	virtual mid_pointer_iterator_struct reset_midstates_iterator(my_uint i, my_uint j, int typ, bool do_not_use_constrain_bag = false) { assert(false); }

	virtual E& symbol_at(my_uint s1, my_uint s2) { assert(false); }

	virtual my_uint first_state() const { assert(false); }

	virtual my_uint last_state() const { assert(false); }

	virtual my_uint left_state() { assert(false); }

	virtual my_uint right_state() { assert(false); }

	virtual void destroy_all() { assert(false); }	
};

template <class E> class string_lattice : public abstract_lattice<E> {

protected:
	array_1d<E> str;
	my_uint n;

	void _init_lattice(array_1d<E>& sentence);

public:
	
	void destroy_all()
	{
		// TODO: changed it because got an error need to check where str is deleted
		//str.destroy();
	}

	template<class F> string_lattice<F> create_data_lattice(F default_val);

	E& symbol_at(my_uint s1, my_uint s2);

	string_lattice(array_1d<E>& sentence);

	my_uint length() const {
		return n;
	}

	my_uint first_state() const {
		return 0;
	}

	my_uint last_state() const {
		return n;
	}

	my_uint span_length(my_uint i, my_uint j);
};

template <class E> class iterable_string_lattice : public string_lattice<E> {

public:

protected:
	
	cnf_span_bag* constrain_bag;

	int left_idx, right_idx, current_len;
	bool iterating;
	int iter_type;

	bool iterating_incoming_state;
	int target_state;

	int midstate_iterator_count;

public:
	
	iterable_string_lattice(array_1d<E>& sentence, cnf_span_bag* bag = NULL);

	void set_span_bag(cnf_span_bag* bag)
	{
		constrain_bag = bag;
	}
	
	void reset_span_iterator(int iter_type);

	bool next_span();

	my_uint left_state();

	my_uint right_state();

	// mid state iterators: they iterate over all midstates between a pair of states
	mid_pointer_iterator_struct reset_midstates_iterator(my_uint i, my_uint j, int typ, bool do_not_use_constrain_bag = false);

	bool next_midstate(mid_pointer_iterator_struct& num);

	my_uint mid_state(mid_pointer_iterator_struct& num);

	// all states that come before a given state. for string latice, it is just the previous word
	void reset_incoming_states(my_uint target_state);

	bool next_incoming_state();

	my_uint incoming_state();

	my_uint mid_state(int num);

	bool is_root_span(int i, int j);

        bool has_span_length_1(my_uint i, my_uint j);

        bool has_span_length_1plus(my_uint i, my_uint j);
};

#endif /*LATTICE_H_*/
