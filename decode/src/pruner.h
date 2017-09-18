#ifndef PRUNER_H_
#define PRUNER_H_

//#include "cnf_io.h"
#include "utils.h"
#include "rules.h"
#include "grammar.h"

template <class W> class cnf_io;
class cnf_io_simple_tensor_interface;
class cnf_io_simple_tensor;

template <class N, class P> class constituent_bag_interface {
protected:
	vector<P> tags;
public:
	virtual ~constituent_bag_interface<N, P>() {
	}

	virtual bool has_preterminal_constituent(P pre, my_uint s1, my_uint s2) = 0;

	virtual bool has_nonterminal_constituent(N non, my_uint s1, my_uint s2) = 0;

	virtual void enforce_tags(vector<P> t) {
		tags = t;
	}

	virtual void setRightBranch(bool rightBranch, int endS)
	{
		//assert(false);
	}
};

class cnf_simple_pruner :
	public constituent_bag_interface<inpre_symbol, inpre_symbol> {
public:
	my_double threshold;
	cnf_io<my_double>* _io;
	bool rightBranching;
	int endState;

	cnf_simple_pruner(cnf_io<my_double>* io, my_double t) {
		threshold = t;
		this->_io = io;
		this->rightBranching = false;
	}

	cnf_io<my_double>* io() {
		return _io;
	}

	void setRightBranch(bool rightBranch, int endS)
	{
		this->endState = endS;
		this->rightBranching = rightBranch;	
	}

	bool has_preterminal_constituent(inpre_symbol pre, my_uint s1, my_uint s2);

	bool has_nonterminal_constituent(inpre_symbol non, my_uint s1, my_uint s2);

};

class cnf_file_simple_pruner :
	public constituent_bag_interface<inpre_symbol, inpre_symbol> {
public:
	my_double threshold;
	cnf_io_simple_tensor_interface* _io;

	cnf_file_simple_pruner(cnf_io_simple_tensor_interface* io, my_double t) {
		threshold = t;
		this->_io = io;
	}

	cnf_io_simple_tensor_interface* io() {
		return _io;
	}

	bool has_preterminal_constituent(inpre_symbol pre, my_uint s1, my_uint s2);

	bool has_nonterminal_constituent(inpre_symbol non, my_uint s1, my_uint s2);

};

class cnf_lpcfg_pruner :
	public constituent_bag_interface<inpre_symbol, inpre_symbol> {
public:
	my_double threshold;
	cnf_io_simple_tensor* _io;

	cnf_lpcfg_pruner(cnf_io_simple_tensor* io, my_double t) {
		threshold = t;
		this->_io = io;
	}

	cnf_io_simple_tensor* io() {
		return _io;
	}

	bool has_preterminal_constituent(inpre_symbol pre, my_uint s1, my_uint s2);

	bool has_nonterminal_constituent(inpre_symbol non, my_uint s1, my_uint s2);

};

#endif /*PRUNER_H_*/
