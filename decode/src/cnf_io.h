#ifndef CNF_IO_H_
#define CNF_IO_H_

#include <string>
#include <iostream>
#include <sstream>
#include "debug.h"
#include <math.h>

#include "grammar.h"
#include "utils.h"
#include "rules.h"
#include "semiring.h"
#include "lattice.h"
#include "full_lattice.h"

#include "pruner.h"

using namespace std;
using namespace std::tr1;

class cnf_simple_pruner;

template<class P> class cnf_io {
protected:
	typedef cnf_grammar<inpre_symbol, vocab_symbol, P> io_cnf_grammar;
	typedef cnf_term_rule<inpre_symbol, vocab_symbol> io_cnf_term_rule;
	typedef unordered_map<inpre_symbol, P> sym2double;

	array_3d<P> i_table;
	array_3d<P> o_table;

	bool is_lattice, multiplyLatticeWeight;
	double addVal;

	bool rightBranch;

	double overflow_prevention_factor;

	array_3d<bool> i_demand_table;
	array_3d<bool> o_demand_table;

	bool on_demand;

	//iterable_string_lattice<sym2double>* o_preterminal_table;
	abstract_lattice<sym2double>* o_preterminal_table;

	bool calc_goal;

	P calculated_goal;

	cnf_span_bag* span_bag;

	//cnf_simple_pruner<inpre_symbol, inpre_symbol>* simple_pruner;

	abstract_semiring<P>* sr;
	abstract_lattice<vocab_symbol>* lattice;
	io_cnf_grammar* grammar;

	constituent_bag_interface<inpre_symbol, inpre_symbol>* pruner;

	void _create_io_tables(my_uint num_symbols, my_uint num_states);

	virtual void _create_io_tables_further(my_uint num_symbols,
			my_uint num_states) {
	}

	void _calc_inside(my_uint symbol, my_uint state1, my_uint state2);

	void _calc_outside_on_demand(inpre_symbol a_sym, my_uint i, my_uint j);

	void _calc_inside_on_demand(inpre_symbol a_sym, my_uint i, my_uint j);

	void _calc_outside(my_uint symbol, my_uint state1, my_uint state2);

	P _inside(inpre_symbol a, my_uint i, my_uint j);

	P _outside(inpre_symbol a, my_uint i, my_uint j);

	virtual bool equals_zero(P w) {
		return sr->equals_zero(w);
	}

	// rule iterators in grammar
	virtual unweighted_cnf_grammar<inpre_symbol, vocab_symbol>::cnf_binary_rule_set& binary_rules_with_ab(
			inpre_symbol a_sym, inpre_symbol b_sym, bool b, my_uint i,
			my_uint k, my_uint j) {
		if (span_bag != NULL) {
			return span_bag->binary_rules_with_ab(a_sym, b_sym, b, i, k, j);
		} else {
			return current_unweighted_grammar()->binary_rules_with_ab(a_sym,
					b_sym, b);
		}
	}

	virtual unweighted_cnf_grammar<inpre_symbol, vocab_symbol>::cnf_binary_rule_set& binary_rules_with_ac(
			inpre_symbol a_sym, inpre_symbol c_sym, bool b, my_uint i,
			my_uint k, my_uint j) {
		if (span_bag != NULL) {
			return span_bag->binary_rules_with_ac(a_sym, c_sym, b, i, k, j);
		} else {
			return current_unweighted_grammar()->binary_rules_with_ac(a_sym,
					c_sym, b);
		}
	}

	virtual unweighted_cnf_grammar<inpre_symbol, vocab_symbol>::cnf_binary_rule_set& binary_rules_with_nonterminals(
			inpre_symbol a_sym, my_uint i, my_uint j) {
		if (span_bag != NULL) {
			return span_bag->binary_rules_with_nonterminals(a_sym, i, j);
		} else {
			return current_unweighted_grammar()->binary_rules_with_nonterminals(
					a_sym);
		}
	}

	virtual unweighted_cnf_grammar<inpre_symbol, vocab_symbol>::cnf_binary_rule_set& binary_rules_with_c(
			inpre_symbol c_sym, my_uint i, my_uint k, my_uint j) {
		if (span_bag != NULL) {
			return span_bag->binary_rules_with_c(c_sym, i, k, j);
		} else {
			return current_unweighted_grammar()->binary_rules_with_c(c_sym);
		}
	}

	virtual unweighted_cnf_grammar<inpre_symbol, vocab_symbol>::cnf_binary_rule_set& binary_rules_with_b(
			inpre_symbol b_sym, my_uint i, my_uint k, my_uint j) {
		if (span_bag != NULL) {
			return span_bag->binary_rules_with_b(b_sym, i, k, j);
		} else {
			return current_unweighted_grammar()->binary_rules_with_b(b_sym);
		}
	}

	virtual unweighted_cnf_grammar<inpre_symbol, vocab_symbol>::cnf_nonterm_set& roots() {
		if (span_bag != NULL) {
			return span_bag->roots();
		} else {
			return current_unweighted_grammar()->roots();
		}
	}

	virtual const unweighted_cnf_grammar<inpre_symbol, vocab_symbol>::cnf_term_rule_set& _rule_iter_preterminal(
			vocab_symbol x1, my_uint s1, my_uint s2) {
		if (span_bag != NULL) {
			return span_bag->preterminal_rules_with_term(x1, s1, s2);
		} else {
			return current_unweighted_grammar()->preterminal_rules_with_term(x1);
		}
	}

public:
	cnf_io(abstract_semiring<P>* sr) {
		this->sr = sr;
		this->o_preterminal_table = NULL;
		this->grammar = NULL;
		this->lattice = NULL;
		this->calc_goal = false;
		this->pruner = NULL;
		this->span_bag = NULL;
		this->overflow_prevention_factor = 1.0;
	}

	virtual ~cnf_io() {
		destroy_tables();

		if (o_preterminal_table != NULL) {
			o_preterminal_table->destroy_all();
		}

		lattice->destroy_all();

		delete o_preterminal_table;
		delete lattice;

		i_table.destroy();
		o_table.destroy();

		i_demand_table.destroy();
		o_demand_table.destroy();
	}

	void set_overflow_prevention_factor(double f) {
		overflow_prevention_factor = f;
	}

	double get_overflow_prevention_factor() {
		return overflow_prevention_factor;
	}

	double get_overflow_prevention_lattice_factor() {
		return pow(overflow_prevention_factor, (double) sentence_length());
	}

	my_uint sentence_length() {
		assert(lattice != NULL);
		return lattice->last_state();
	}

	virtual void destroy_tables() {

	}

	virtual io_cnf_grammar* current_grammar() {
		return this->grammar;
	}

	virtual unweighted_cnf_grammar<inpre_symbol, vocab_symbol>* current_unweighted_grammar() {
		return this->grammar;
	}

	void set_pruner(constituent_bag_interface<inpre_symbol, inpre_symbol>* p);

	bool pruned(inpre_symbol a, my_uint s1, my_uint s2);

	virtual void
	io_start_string(
			array_1d<vocab_symbol> sentence,
			io_cnf_grammar* grammar,
			constituent_bag_interface<inpre_symbol, inpre_symbol>* pruner =
					NULL,
			cnf_grammar<inpre_symbol, vocab_symbol, my_double>* prune_grammar =
					NULL);


	virtual void io_start_lattice(string latticeString, bool mulOrAdd, double add,
					io_cnf_grammar* grammar,
					constituent_bag_interface<inpre_symbol, inpre_symbol>* pruner = NULL,
					cnf_grammar<inpre_symbol, vocab_symbol, my_double>* prune_grammar = NULL, bool rightBranch = false);

	virtual void io_constrain_bag(cnf_span_bag* bag) {
		this->span_bag = bag;
		this->lattice->set_span_bag(bag);
	}

	void io_do_inside() {
		_calc_inside(0, 0, 0);
	}

	void io_do_outside() {
		_calc_outside(0, 0, 0);
	}

	virtual void printChart() {
		cerr << "Cannot print chart for this class" << endl;
		assert(false);
	}

	virtual P inside_span_weight(
			const cnf_term_rule<inpre_symbol, vocab_symbol>& rule, my_uint i,
			my_uint j) {
		P w = grammar->weight(rule);

		return w;
	}

	virtual P inside_span_weight(const cnf_binary_rule<inpre_symbol>& rule,
			my_uint i, my_uint k, my_uint j) {
		return grammar->weight(rule);
	}

	virtual P root_span_weight(inpre_symbol a, my_uint s1, my_uint s2) {
		return grammar->weight(a);
	}

	P inside_goal();

	P goal();

	P inside(inpre_symbol a, my_uint i, my_uint j) {
		return _inside(a, i, j);
	}

	P outside(inpre_symbol a, my_uint i, my_uint j) {
		return _outside(a, i, j);
	}

	abstract_semiring<P>* semiring() {
		return sr;
	}

	void io_print_table_inside() {
		for (my_uint i = 0; i < i_table.slices(); i++) {

			cerr << "slice for nt=" << i << endl;

			for (my_uint j = 0; j < i_table.rows(); j++) {
				for (my_uint k = 0; k < i_table.cols(); k++) {
					cerr << i_table[i][j][k] << " ";
				}
				cerr << endl;
			}
		}
	}

	void io_print_table_outside() {
		for (my_uint i = 0; i < o_table.slices(); i++) {

			cerr << "slice for nt=" << i << endl;

			for (my_uint j = 0; j < o_table.rows(); j++) {
				for (my_uint k = 0; k < o_table.cols(); k++) {
					cerr << o_table[i][j][k] << " ";
				}
				cerr << endl;
			}
		}
	}

};

class cnf_io_simple_tensor: public cnf_io<simple_tensor> {
private:
	bool numeric_calc_goal;
	my_double numeric_calculated_goal;

public:
	cnf_io_simple_tensor(tensor_semiring* sr) :
		cnf_io<simple_tensor> (sr) {
		numeric_calc_goal = false;
	}

	my_double goal_numeric() {
		if (numeric_calc_goal) {
			return numeric_calculated_goal;
		} else {
			assert(lattice!=NULL);
			my_uint first = lattice->first_state();
			my_uint last = lattice->last_state();

			auto& roots_ = current_unweighted_grammar()->roots();

			numeric_calculated_goal = 0;

			for (auto iter = roots_.begin(); iter != roots_.end(); iter++) {
				numeric_calculated_goal = numeric_calculated_goal
						+ ((tensor_semiring*) semiring())->marginalize_vector(
								inside(*iter, first, last),
								outside(*iter, first, last));
			}

			numeric_calc_goal = true;

			return numeric_calculated_goal;
		}
	}

	virtual void destroy_tables();

};

template<class T, class E> class cnf_double_backpointer_pair {
public:

	cnf_double_backpointer_pair(inpre_symbol sym, my_uint s1, my_uint s2,
			my_uint s3, my_double v, bool isPre) {
		this->_v = v;
		this->a = sym;
		this->i = s1;
		this->k = s2;
		this->j = s3;
		this->isPre = isPre;
	}

	cnf_double_backpointer_pair(my_double v) {
		this->_v = v;
		this->isPre = false;
	}

	cnf_double_backpointer_pair() {

		this->_v = -std::numeric_limits<double>::infinity();
	}

	void set_merge(cnf_double_backpointer_pair<T, E>& a,
			cnf_double_backpointer_pair<T, E>& b) {
		assert(false);
	}

	void set_merge(cnf_double_backpointer_pair<T, E>& a,
			cnf_double_backpointer_pair<T, E>& b,
			cnf_double_backpointer_pair<T, E>& c) {

		assert(a.j == b.i);

		this->i = a.i;
		this->k = a.j;
		this->j = b.j;
		this->b = a.a;
		this->c = b.a;
		this->a = c.a;
		this->isPre = false;
	}

	my_double& v() {
		return this->_v;
	}

	bool is_pre() { return isPre; }

	inpre_symbol get_a() {
		return this->a;
	}

	inpre_symbol get_b() {
		return this->b;
	}

	inpre_symbol get_c() {
		return this->c;
	}

	my_uint get_i() {
		return this->i;
	}

	my_uint get_j() {
		return this->j;
	}

	my_uint get_k() {
		return this->k;
	}

	friend ostream &operator<<(ostream &output,
			const cnf_double_backpointer_pair<T, E> &a) {
		output << a._v;

		return output;
	}

	friend istream &operator>>(istream &input,
			cnf_double_backpointer_pair<T, E>& a) {
		assert(false);

		return input;
	}

private:
	inpre_symbol a;
	inpre_symbol b;
	inpre_symbol c;
	my_uint i, k, j;
	bool isPre;

	my_double _v;
};

class cnf_io_viterbi: public cnf_io<cnf_double_backpointer_pair<inpre_symbol,
		vocab_symbol> > {

public:
	cnf_io_viterbi(
			max_log_semiring<cnf_double_backpointer_pair<inpre_symbol,
					vocab_symbol> >* sr) :
		cnf_io<cnf_double_backpointer_pair<inpre_symbol, vocab_symbol> > (sr) {

	}

	string goal_string() {
		ostringstream s;

		cnf_double_backpointer_pair<inpre_symbol, vocab_symbol> g =
				inside_goal();

		if (g.v() == NEG_INF) {
			return "()";
		} else {
			goal_string(s, g);

			return s.str();
		}
	}

	double goal_numeric() {
		ostringstream s;

		cnf_double_backpointer_pair<inpre_symbol, vocab_symbol> g =
				inside_goal();

		return g.v();
	}

	void goal_string(ostream& s,
			cnf_double_backpointer_pair<inpre_symbol, vocab_symbol> trace) {

		if (trace.get_k() == trace.get_j()) {
			return;
		} else {
			s << "("
					<< current_unweighted_grammar()->alphabet()->nonterminal_to_string(
							trace.get_a()) << " ";

			my_uint
					b_idx =
							current_unweighted_grammar()->alphabet()->nonterminal_lookup(
									trace.get_b());

			my_uint
					c_idx =
							current_unweighted_grammar()->alphabet()->nonterminal_lookup(
									trace.get_c());

			//if (lattice->span_length(trace.get_i(), trace.get_k()) == 1) {
			if (trace.get_b().is_pre()) {
				s << "("
						<< current_unweighted_grammar()->alphabet()->nonterminal_to_string(
								trace.get_b()) << " "
						<< current_unweighted_grammar()->alphabet()->vocab_to_string(
								lattice->symbol_at(trace.get_i(), trace.get_k()))
						<< ")";
			} else {
				goal_string(s, i_table[b_idx][trace.get_i()][trace.get_k()]);
			}

			s << " ";
			//if (lattice->span_length(trace.get_k(), trace.get_j()) == 1) {
			if (trace.get_c().is_pre()) {
				s << "("
						<< current_unweighted_grammar()->alphabet()->nonterminal_to_string(
								trace.get_c()) << " "
						<< current_unweighted_grammar()->alphabet()->vocab_to_string(
								lattice->symbol_at(trace.get_k(), trace.get_j()))
						<< ")";
			} else {
				goal_string(s, i_table[c_idx][trace.get_k()][trace.get_j()]);
			}

			s << ")";
		}

		//cerr<<"exiting trace\n";
	}

};

class cnf_score_interface {
public:
	virtual my_double binary_score(cnf_binary_rule<inpre_symbol> rule,
			simple_tensor& a, simple_tensor& b, simple_tensor& c,
			simple_tensor& a_in, simple_tensor& b_out, simple_tensor& c_out,
			my_double g, vector<my_double>* phi,
			unordered_map<int, int>* phi_map, vector<my_double>* gen_phi) = 0;
	virtual my_double
	term_score(cnf_term_rule<inpre_symbol, vocab_symbol> rule,
			simple_tensor& a_in, simple_tensor& a_out, my_double g,
			vector<my_double>* phi, unordered_map<int, int>* phi_map,
			vector<my_double>* gen_phi) = 0;
};

class cnf_io_mbr_lpcfg: public cnf_io_viterbi {

protected:
	cnf_io_simple_tensor* lpcfg_io;
	array_3d<unordered_map<cnf_binary_rule<inpre_symbol> , my_double> >
			marginals;
	bool shouldFlipSign;
	bool shouldPrintChart;

	typedef unordered_set<string> ChartStringTable;

	ChartStringTable printedChart;

public:
	cnf_io_mbr_lpcfg(
			max_log_semiring<cnf_double_backpointer_pair<inpre_symbol,
					vocab_symbol> >* sr, cnf_io_simple_tensor* lpcfg,
			bool flipNegativeSign, bool printChart) :
		cnf_io_viterbi(sr) {

		lpcfg_io = lpcfg;

		shouldFlipSign = flipNegativeSign;

		shouldPrintChart = printChart;
	}

	virtual ~cnf_io_mbr_lpcfg() {
		marginals.destroy();
	}

	unweighted_cnf_grammar<inpre_symbol, vocab_symbol>* current_unweighted_grammar() {
		assert(lpcfg_io != NULL);

		return lpcfg_io->current_unweighted_grammar();
	}

	cnf_double_backpointer_pair<inpre_symbol, vocab_symbol> inside_span_weight(
			const cnf_term_rule<inpre_symbol, vocab_symbol>& rule, my_uint s1,
			my_uint s2) {

		// TODO need to hash this

		my_double marg;

		if (pruned(rule.a(), s1, s2)) {
			marg = NEG_INF;
		} else {
			simple_tensor tensor_a_out = lpcfg_io->outside(rule.a(), s1, s2);
			simple_tensor tensor_a_in = lpcfg_io->inside(rule.a(), s1, s2);

			marg
					= ((tensor_semiring*) lpcfg_io->semiring())->marginalize_vector(
							tensor_a_in, tensor_a_out);
		/*		cout << "pre "
						<< current_unweighted_grammar()->alphabet()->nonterminal_to_string(
								rule.a()) << " " << s1 << " " << s2 << " "
						<< marg << endl;
			cout<<"tensor_a_in"<<endl;
			for (int l=0; l < tensor_a_in.cols(); l++) 
			{
				cout<<tensor_a_in.get(l)<<endl;
			}
			cout<<"tensor_a_out"<<endl;
			for (int l=0; l < tensor_a_out.cols(); l++) 
			{
				cout<<tensor_a_out.get(l)<<endl;
			}*/
		}

		if ((shouldFlipSign) && (marg < 0) && (marg > NEG_INF)) {
			marg = -marg;
		}

		if (is_lattice) {
                        double w2 = ((FullLatticeString*)lattice)->weight_at(s1, s2);
			if (multiplyLatticeWeight) {
                        	marg = marg * w2 * addVal;
			} else {
				marg += w2 * addVal;
			}
                }

		if (marg > NEG_INF) {
			if (shouldPrintChart) {
				ostringstream str;

				str << "pre "
						<< current_unweighted_grammar()->alphabet()->nonterminal_to_string(
								rule.a()) << " " << s1 << " " << s2 << " "
						<< marg;

				printedChart.insert(str.str());
			}
		}

		return cnf_double_backpointer_pair<inpre_symbol, vocab_symbol> (
				rule.a(), s1, s2, s2, marg, true);
	}

	cnf_double_backpointer_pair<inpre_symbol, vocab_symbol> root_span_weight(
			inpre_symbol a, my_uint s1, my_uint s2) {

		// TODO need to hash this

		simple_tensor tensor_a_out = lpcfg_io->outside(a, s1, s2);
		simple_tensor tensor_a_in = lpcfg_io->inside(a, s1, s2);

		my_double marg =
				((tensor_semiring*) lpcfg_io->semiring())->marginalize_vector(
						tensor_a_in, tensor_a_out);

		if ((shouldFlipSign) && (marg < 0) && (marg > NEG_INF)) {
			marg = -marg;
		}

		if (marg > NEG_INF) {
			if (shouldPrintChart) {
				ostringstream str;

				str << "rt "
						<< current_unweighted_grammar()->alphabet()->nonterminal_to_string(
								a) << " " << s1 << " " << s2 << " " << marg;

				printedChart.insert(str.str());
			}
		}

		return cnf_double_backpointer_pair<inpre_symbol, vocab_symbol> (a, s1,
				s2, s2, marg, false);
	}

	void _create_io_tables_further(my_uint num_symbols, my_uint num_states) {

		marginals = memutils::create_3d_array(num_states, num_states,
				num_states,
				unordered_map<cnf_binary_rule<inpre_symbol> , my_double> ());
	}

	cnf_double_backpointer_pair<inpre_symbol, vocab_symbol> inside_span_weight(
			const cnf_binary_rule<inpre_symbol>& rule, my_uint i, my_uint k,
			my_uint j) {

		unordered_map<cnf_binary_rule<inpre_symbol> , my_double>& h =
				marginals.get_ref(i, k, j);

		if (h.find(rule) == h.end()) {

			// TODO now not marginalizing rules with b and c, but only with a
			// need to have an option to do marginalization with b and c.

			simple_tensor tensor_a = lpcfg_io->outside(rule.a(), i, j);
			simple_tensor tensor_a_in = lpcfg_io->inside(rule.a(), i, j);

			//simple_tensor tensor_b = lpcfg_io->inside(rule.b(), i, k);
			//simple_tensor tensor_c = lpcfg_io->inside(rule.c(), k, j);
			//simple_tensor rule_weight = lpcfg_io->current_grammar()->weight(rule);

			/*my_double marg = 0;
			 
			 for (int i=0; i < tensor_a.cols(); i++)
			 {
			 marg += tensor_a.get(i);
			 }*/

			//my_double marg = ((tensor_semiring*)lpcfg_io->semiring())->marginalize_tensor(tensor_a, tensor_b,
			//		tensor_c, rule_weight);

			my_double
					marg =
							((tensor_semiring*) lpcfg_io->semiring())->marginalize_vector(
									tensor_a, tensor_a_in);
			/*simple_tensor tensor_a_in = lpcfg_io->outside(rule.a(), i, j);
			 simple_tensor tensor_a_out = lpcfg_io->inside(rule.a(), i, j);

			 simple_tensor tensor_b_in = lpcfg_io->outside(rule.b(), i, k);
			 simple_tensor tensor_b_out = lpcfg_io->inside(rule.b(), i, k);

			 simple_tensor tensor_c_in = lpcfg_io->outside(rule.c(), k, j);
			 simple_tensor tensor_c_out = lpcfg_io->inside(rule.c(), k, j);
			 
			 my_double marg1 = ((tensor_semiring*)lpcfg_io->semiring())->marginalize_vector(tensor_a_in, tensor_a_out);
			 my_double marg2 = ((tensor_semiring*)lpcfg_io->semiring())->marginalize_vector(tensor_b_in, tensor_b_out);
			 my_double marg3 = ((tensor_semiring*)lpcfg_io->semiring())->marginalize_vector(tensor_c_in, tensor_c_out);

			 my_double marg = marg1*marg2*marg3;*/

			if ((shouldFlipSign) && (marg < 0) && (marg > NEG_INF)) {
				marg = -marg;
			}

			h[rule] = marg;

			if (marg > NEG_INF) {
				if (shouldPrintChart) {
					// TODO maybe we should print the whole rule
					// note that we are not printing for the whole rule, only for the nonterminal

					ostringstream str;

					str << "in "
							<< current_unweighted_grammar()->alphabet()->nonterminal_to_string(
									rule.a()) << " " << i << " " << k << " "
							<< j << " " << marg;

					printedChart.insert(str.str());
				}
			}

			return cnf_double_backpointer_pair<inpre_symbol, vocab_symbol> (
					rule.a(), i, k, j, marg, false);
		} else {
			return cnf_double_backpointer_pair<inpre_symbol, vocab_symbol> (
					rule.a(), i, k, j, h[rule], false);
		}
	}

	void printChart() {
		bool first = true;

		for (auto i = printedChart.begin(); i != printedChart.end(); i++) {
			if (!first) {
				cout << ";";
			}

			cout << *i;

			first = false;
		}

		cout<<endl;
	}

};

class cnf_io_simple_tensor_interface {
public:

	virtual simple_tensor inside(inpre_symbol a, my_uint i, my_uint j) = 0;

	virtual simple_tensor outside(inpre_symbol a, my_uint i, my_uint j) = 0;

	virtual my_double goal() = 0;

	virtual unweighted_cnf_grammar<inpre_symbol, vocab_symbol>
	* current_unweighted_grammar() = 0;

	virtual tensor_semiring* semiring() = 0;

	virtual my_double goal_numeric() = 0;
};

class cnf_io_simple_tensor_file: public cnf_io_simple_tensor_interface {
private:
	typedef tuple<my_uint, my_uint, inpre_symbol> key_type_no_mid;

	unordered_map<key_type_no_mid, simple_tensor> inside_table;

	unordered_map<key_type_no_mid, simple_tensor> outside_table;

	simple_tensor empty_tensor;

	bool delete_all;

	bool calc_goal;

	my_double calculated_goal;

	int sent_len;

	unweighted_cnf_grammar<inpre_symbol, vocab_symbol>* g;

	tensor_semiring* sr;

public:

	cnf_io_simple_tensor_file(
			unweighted_cnf_grammar<inpre_symbol, vocab_symbol>* gra) {
		delete_all = false;
		g = gra;
		calc_goal = false;
		sent_len = 0;
		sr = new tensor_semiring();
	}

	virtual ~cnf_io_simple_tensor_file() {
		delete_tensors();
		delete sr;
	}

	unweighted_cnf_grammar<inpre_symbol, vocab_symbol>* current_unweighted_grammar() {
		return g;
	}

	void delete_tensors() {
		if (delete_all) {
			for (auto iter = inside_table.begin(); iter != inside_table.end(); iter++) {
				iter->second.destroy();
			}

			for (auto iter = outside_table.begin(); iter != outside_table.end(); iter++) {
				iter->second.destroy();
			}
		}

		// calculated_goal.destroy();

		sent_len = 0;

		calc_goal = false;
	}

	string file_signature(string path_name, array_1d<vocab_symbol> sent,
			grammar_alphabet<inpre_symbol, vocab_symbol>* alphabet) {
		ostringstream str;

		for (auto i = sent.length() * 0; i < sent.length(); i++) {
			str << alphabet->vocab_to_string(sent[i]) << endl;
		}

		std::tr1::hash<string> str_hash;

		ostringstream str2;

		ostringstream str3;

		str3 << (my_uint) (str_hash(str.str()));

		const char* x = str3.str().c_str();

		str2 << path_name << "/" << x[0] << x[1] << "/" << str3.str();

		return str2.str();
	}

	void copy_chart(cnf_io<simple_tensor>* chart) {
		delete_all = false;

		auto grammar = chart->current_unweighted_grammar();

		my_uint len = chart->sentence_length();

		for (my_uint i = 0; i < len + 1; i++) {
			for (my_uint j = i + 1; j < len + 1; j++) {
				if ((j - i) > 1) {
					for (auto iter =
							grammar->alphabet()->nonterminals().begin(); iter
							!= grammar->alphabet()->nonterminals().end(); iter++) {
						key_type_no_mid k(i, j, *iter);

						simple_tensor t = chart->inside(*iter, i, j);

						if (!t.empty()) {
							inside_table[k] = t;
						}

						t = chart->outside(*iter, i, j);

						if (!t.empty()) {
							outside_table[k] = t;
						}
					}
				} else {
					for (auto iter =
							grammar->alphabet()->preterminals().begin(); iter
							!= grammar->alphabet()->preterminals().end(); iter++) {
						key_type_no_mid k(i, j, *iter);

						simple_tensor t = chart->inside(*iter, i, j);

						if (!t.empty()) {
							inside_table[k] = t;
						}

						t = chart->outside(*iter, i, j);

						if (!t.empty()) {
							outside_table[k] = t;
						}
					}
				}
			}
		}
	}

	bool load(string path_name, array_1d<vocab_symbol> sent,
			grammar_alphabet<inpre_symbol, vocab_symbol>* alphabet) {

		delete_tensors();

		delete_all = true;

		// format is: symbol i j m <vector>

		string filename = file_signature(path_name, sent, alphabet);

		cerr << "Loading " << filename << endl;

		ifstream myfile(filename.c_str());

		if (!myfile.is_open()) {
			return false;
		}

		string s;

		getline(myfile, s); // first line is the sentence

		while (getline(myfile, s)) {
			vector<string> out;

			string_utils::split(out, s, " ");

			key_type_no_mid k(stoi(out[2]), stoi(out[3]),
					alphabet->nonterminal_lookup_string(out[1]));

			simple_tensor t(stoi(out[4]), 0.0);

			for (int i = 0; i < t.cols(); i++) {
				t.set(i, stod(out[5+i]));
			}

			if (out[0] == string("i")) {
				inside_table[k] = t;
			} else {
				outside_table[k] = t;
			}
		}

		sent_len = sent.length();

		return true;
	}

	bool save(string path_name, array_1d<vocab_symbol> sent,
			grammar_alphabet<inpre_symbol, vocab_symbol>* alphabet) {
		string filename = file_signature(path_name, sent, alphabet);

		cerr << "Saving " << filename << endl;

		ofstream myfile(filename.c_str());

		if (!myfile.is_open()) {
			return false;
		}

		myfile << "#";
		for (auto i = 0 * sent.length(); i < sent.length(); i++) {
			myfile << " " << alphabet->vocab_to_string(sent[i]);
		}
		myfile << endl;

		for (auto iter = inside_table.begin(); iter != inside_table.end(); iter++) {
			auto k = iter->first;
			auto t = iter->second;

			myfile << "i " << alphabet->nonterminal_to_string(get<2> (k))
					<< " " << get<0> (k) << " " << get<1> (k) << " "
					<< t.cols();

			for (int i = 0; i < t.cols(); i++) {
				myfile << " " << t.get(i);
			}

			myfile << endl;
		}

		for (auto iter = outside_table.begin(); iter != outside_table.end(); iter++) {
			auto k = iter->first;
			auto t = iter->second;

			myfile << "o " << alphabet->nonterminal_to_string(get<2> (k))
					<< " " << get<0> (k) << " " << get<1> (k) << " "
					<< t.cols();

			for (int i = 0; i < t.cols(); i++) {
				myfile << " " << t.get(i);
			}

			myfile << endl;
		}

		myfile.close();

		return true;
	}

	simple_tensor inside(inpre_symbol a, my_uint i, my_uint j) {
		key_type_no_mid k(i, j, a);

		if (inside_table.find(k) == inside_table.end()) {
			return empty_tensor;
		} else {
			return inside_table[k];
		}
	}

	simple_tensor outside(inpre_symbol a, my_uint i, my_uint j) {
		key_type_no_mid k(i, j, a);

		if (outside_table.find(k) == outside_table.end()) {
			return empty_tensor;
		} else {
			return outside_table[k];
		}
	}

	tensor_semiring* semiring() {
		return sr;
	}

	my_double goal() {
		if (calc_goal) {
			return calculated_goal;
		} else {
			my_uint first = 0;
			my_uint last = sent_len;

			auto& roots_ = current_unweighted_grammar()->roots();

			calculated_goal = 0;

			for (auto iter = roots_.begin(); iter != roots_.end(); iter++) {
				calculated_goal = calculated_goal
						+ sr->marginalize_vector(inside(*iter, first, last),
								outside(*iter, first, last));
			}

			calc_goal = true;

			return calculated_goal;
		}
	}

	my_double goal_numeric() {
		return goal();
	}

};

class cnf_io_score_lpcfg: public cnf_io_viterbi {

protected:
	cnf_io_simple_tensor* lpcfg_io;
	cnf_io_simple_tensor_interface* lpcfg_interface;
	cnf_score_interface* lpcfg_score;
	array_3d<unordered_map<cnf_binary_rule<inpre_symbol> , my_double> >
			marginals;

public:
	cnf_io_score_lpcfg(
			max_log_semiring<cnf_double_backpointer_pair<inpre_symbol,
					vocab_symbol> >* sr, cnf_io_simple_tensor* lpcfg,
			cnf_score_interface* score) :
		cnf_io_viterbi(sr) {

		lpcfg_score = score;
		lpcfg_io = lpcfg;
		lpcfg_interface = NULL;
	}

	cnf_io_score_lpcfg(
			max_log_semiring<cnf_double_backpointer_pair<inpre_symbol,
					vocab_symbol> >* sr, cnf_io_simple_tensor_interface* lpcfg,
			cnf_score_interface* score) :
		cnf_io_viterbi(sr) {

		lpcfg_score = score;
		lpcfg_io = NULL;
		lpcfg_interface = lpcfg;
	}

	virtual ~cnf_io_score_lpcfg() {
		marginals.destroy();
	}

	unweighted_cnf_grammar<inpre_symbol, vocab_symbol>* current_unweighted_grammar() {
		if (lpcfg_io == NULL) {
			return lpcfg_interface->current_unweighted_grammar();
		} else {
			assert(lpcfg_io != NULL);

			return lpcfg_io->current_unweighted_grammar();
		}
	}

	cnf_double_backpointer_pair<inpre_symbol, vocab_symbol> inside_span_weight(
			const cnf_term_rule<inpre_symbol, vocab_symbol>& rule, my_uint s1,
			my_uint s2) {

		simple_tensor tensor_a_out;
		simple_tensor tensor_a_in;

		my_double marg, g;

		if (pruned(rule.a(), s1, s2)) {
			marg = NEG_INF;
		} else {

			if (lpcfg_io != NULL) {
				tensor_a_out = lpcfg_io->outside(rule.a(), s1, s2);
				tensor_a_in = lpcfg_io->inside(rule.a(), s1, s2);
				g = lpcfg_io->goal_numeric();
			} else {
				tensor_a_out = lpcfg_interface->outside(rule.a(), s1, s2);
				tensor_a_in = lpcfg_interface->inside(rule.a(), s1, s2);
				g = lpcfg_interface->goal_numeric();
			}

			if (tensor_a_out.empty() || tensor_a_in.empty()) {
				// TODO hack
				marg = NEG_INF;
			} else {
				marg = lpcfg_score->term_score(rule, tensor_a_in, tensor_a_out,
						g, NULL, NULL, NULL);
			}

		}

		if (is_lattice) {
                        double w2 = ((FullLatticeString*)lattice)->weight_at(s1, s2);

			if (multiplyLatticeWeight) {
                        	marg = marg * w2 * addVal;
			} else {
				marg += w2 * addVal;
			}
                }

		return cnf_double_backpointer_pair<inpre_symbol, vocab_symbol> (
				rule.a(), s1, s2, s2, marg, true);
	}

	cnf_double_backpointer_pair<inpre_symbol, vocab_symbol> root_span_weight(
			inpre_symbol a, my_uint s1, my_uint s2) {

		// TODO need to hash this

		simple_tensor tensor_a_out;
		simple_tensor tensor_a_in;

		if (lpcfg_io != NULL) {
			tensor_a_out = lpcfg_io->outside(a, s1, s2);
			tensor_a_in = lpcfg_io->inside(a, s1, s2);
		} else {
			tensor_a_out = lpcfg_interface->outside(a, s1, s2);
			tensor_a_in = lpcfg_interface->inside(a, s1, s2);
		}

		my_double marg;

		if (tensor_a_out.empty() || tensor_a_in.empty()) {
			// TODO hack
			marg = NEG_INF;
		}

		marg = ((tensor_semiring*) lpcfg_io->semiring())->marginalize_vector(
				tensor_a_in, tensor_a_out);

		return cnf_double_backpointer_pair<inpre_symbol, vocab_symbol> (a, s1,
				s2, s2, marg, false);

		return marg;
	}

	void _create_io_tables_further(my_uint num_symbols, my_uint num_states) {

		marginals = memutils::create_3d_array(num_states, num_states,
				num_states,
				unordered_map<cnf_binary_rule<inpre_symbol> , my_double> ());
	}

	cnf_double_backpointer_pair<inpre_symbol, vocab_symbol> inside_span_weight(
			const cnf_binary_rule<inpre_symbol>& rule, my_uint i, my_uint k,
			my_uint j) {

		unordered_map<cnf_binary_rule<inpre_symbol> , my_double>& h =
				marginals.get_ref(i, k, j);

		my_double g = 0.0;

		if (h.find(rule) == h.end()) {

			simple_tensor tensor_a;
			simple_tensor tensor_b;
			simple_tensor tensor_c;
			simple_tensor tensor_a_in;
			simple_tensor tensor_b_out;
			simple_tensor tensor_c_out;

			if (lpcfg_io != NULL) {
				tensor_a = lpcfg_io->outside(rule.a(), i, j);
				tensor_b = lpcfg_io->inside(rule.b(), i, k);
				tensor_c = lpcfg_io->inside(rule.c(), k, j);

				tensor_a_in = lpcfg_io->inside(rule.a(), i, j);
				tensor_b_out = lpcfg_io->outside(rule.b(), i, k);
				tensor_c_out = lpcfg_io->outside(rule.c(), k, j);

				g = lpcfg_io->goal_numeric();
			} else {
				tensor_a = lpcfg_interface->outside(rule.a(), i, j);
				tensor_b = lpcfg_interface->inside(rule.b(), i, k);
				tensor_c = lpcfg_interface->inside(rule.c(), k, j);

				tensor_a_in = lpcfg_interface->inside(rule.a(), i, j);
				tensor_b_out = lpcfg_interface->outside(rule.b(), i, k);
				tensor_c_out = lpcfg_interface->outside(rule.c(), k, j);

				g = lpcfg_interface->goal_numeric();
			}

			my_double marg;

			if (tensor_a.empty() || tensor_b.empty() || tensor_c.empty()) {
				// TODO hack
				marg = NEG_INF;
			} else {
				marg = lpcfg_score->binary_score(rule, tensor_a, tensor_b,
						tensor_c, tensor_a_in, tensor_b_out, tensor_c_out, g,
						NULL, NULL, NULL);
			}

			h[rule] = marg;

			return cnf_double_backpointer_pair<inpre_symbol, vocab_symbol> (
					rule.a(), i, k, j, marg, false);
		} else {
			return cnf_double_backpointer_pair<inpre_symbol, vocab_symbol> (
					rule.a(), i, k, j, h[rule], false);
		}
	}
};

#endif /*CNF_IO_H_*/

/*
 * 
 * these are preparatations so that one could do EM and compute marginals.
 * there would be a need for a special "grammar" that returns only the possible rule (from the bag of constituents)
 * and also there would be a need for a special "iterator" that only returns the mid-point that really appears in
 * the data.
 * 
 auto& rule_set = current_unweighted_grammar()->binary_rules_with_ab(a_sym, b_sym, true);
 auto& rule_set = current_unweighted_grammar()->binary_rules_with_ab(a_sym, b_sym, false);
 auto& rule_set = current_unweighted_grammar()->binary_rules_with_ac(a_sym, c_sym, false);
 auto& rule_set = current_unweighted_grammar()->binary_rules_with_nonterminals(a_sym);
 auto& rule_set = current_unweighted_grammar()->binary_rules_with_c(c_sym);
 auto& rule_set = current_unweighted_grammar()->binary_rules_with_c(c_sym);
 auto& rule_set = current_unweighted_grammar()->binary_rules_with_c(c_sym);
 auto& root_set = current_unweighted_grammar()->roots();
 
 this is what's needed to be added to cnf_io, with indices for the midpoints i , k , j
 */
