#include "pruner.h"
#include "cnf_io.h"

bool cnf_simple_pruner::has_preterminal_constituent(inpre_symbol pre,
		my_uint s1, my_uint s2) {
	/*if (io()->semiring()->equals_zero(io()->goal())) {
	 return false;
	 }

	 my_double marginal = io()->semiring()->div(io()->semiring()->mul(io()->inside(pre, s1, s2), io()->outside(pre, s1, s2)),
	 io()->goal());

	 cerr<<"pre marg: "<<marginal<<endl;

	 return (io()->semiring()->ge(marginal, threshold));*/

	// TODO not using lattice here
	if (tags.size() > 0) {
		return (io()->current_unweighted_grammar()->alphabet()->chain_ends_in_nonterminal(pre, tags[s1]));
	}

	return true;
}

bool cnf_simple_pruner::has_nonterminal_constituent(inpre_symbol non,
		my_uint s1, my_uint s2) {

	if (io()->semiring()->equals_zero(io()->goal())) {
		return false;
	}

	my_double v = io()->semiring()->div(io()->outside(non, s1, s2), io()->goal());
	my_double marginal = io()->semiring()->mul(io()->inside(non, s1, s2), v);
	
	bool b = (io()->semiring()->ge(marginal, threshold));

	if (rightBranching)
	{
		return (s2 == endState);
	}

	return b;
}



bool cnf_file_simple_pruner::has_preterminal_constituent(inpre_symbol pre,
		my_uint s1, my_uint s2) {
	/*if (io()->semiring()->equals_zero(io()->goal())) {
	 return false;
	 }

	 my_double marginal = io()->semiring()->div(io()->semiring()->mul(io()->inside(pre, s1, s2), io()->outside(pre, s1, s2)),
	 io()->goal());

	 cerr<<"pre marg: "<<marginal<<endl;

	 return (io()->semiring()->ge(marginal, threshold));*/
	
	if (tags.size() > 0) {
		return (io()->current_unweighted_grammar()->alphabet()->chain_ends_in_nonterminal(pre, tags[s1]));
	}

	return true;
}

bool cnf_file_simple_pruner::has_nonterminal_constituent(inpre_symbol non,
		my_uint s1, my_uint s2) {

	if (io()->goal() == 0.0) {
		return false;
	}

	simple_tensor tensor_a_out = io()->outside(non, s1, s2);
	simple_tensor tensor_a_in = io()->inside(non, s1, s2);

	my_double marginal = ((tensor_semiring*)io()->semiring())->marginalize_vector(tensor_a_in, tensor_a_out) / io()->goal();

	if (marginal < 0) { marginal = -marginal; }

	bool b = marginal > threshold;
	
	return b;
}

bool cnf_lpcfg_pruner::has_preterminal_constituent(inpre_symbol pre,
		my_uint s1, my_uint s2) {
	/*if (io()->semiring()->equals_zero(io()->goal())) {
	 return false;
	 }

	 my_double marginal = io()->semiring()->div(io()->semiring()->mul(io()->inside(pre, s1, s2), io()->outside(pre, s1, s2)),
	 io()->goal());

	 cerr<<"pre marg: "<<marginal<<endl;

	 return (io()->semiring()->ge(marginal, threshold));*/
	
	if (tags.size() > 0) {
		bool b = (io()->current_unweighted_grammar()->alphabet()->chain_ends_in_nonterminal(pre, tags[s1]));

		return b;
	}

	return true;
}

bool cnf_lpcfg_pruner::has_nonterminal_constituent(inpre_symbol non,
		my_uint s1, my_uint s2) {

	if (io()->goal_numeric() == 0.0) {
		return false;
	}

	simple_tensor tensor_a_out = io()->outside(non, s1, s2);
	simple_tensor tensor_a_in = io()->inside(non, s1, s2);

	my_double marginal = ((tensor_semiring*)io()->semiring())->marginalize_vector(tensor_a_in, tensor_a_out) / io()->goal_numeric();

	if (marginal < 0) { marginal = -marginal; }

	bool b = marginal > threshold;
	
	return b;
}
