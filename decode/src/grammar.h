#ifndef GRAMMAR_H_
#define GRAMMAR_H_

#include <iostream>
#include "types.h"
#include "rules.h"
#include "semiring.h"
#include "debug.h"
#include <tr1/unordered_set>
#include <tr1/unordered_map>
#include <string>
#include <iostream>
#include <fstream>
#include <math.h>
#include "mapper.h"

using namespace std;
using namespace std::tr1;
//using namespace __gnu_cxx;

class grammar_symbol {
};

class vocab_symbol: public grammar_symbol {
private:
	my_uint symbol_number;

public:
	vocab_symbol(my_uint sym) {
		symbol_number = sym;
	}

	vocab_symbol() {
		symbol_number = (my_uint) -1;
	}

	my_uint sym() const {
		return symbol_number;
	}

	bool equals(const vocab_symbol& vsym) const {
		return sym() == vsym.sym();
	}

	bool operator==(const vocab_symbol& vsym) const {
		return equals(vsym);
	}
};

namespace std {
namespace tr1 {
template<> struct hash<vocab_symbol> {
public:
	size_t operator()(const vocab_symbol& k) const {
		return k.sym();
	}
};
}
}

class inpre_symbol: public grammar_symbol {
private:
	bool is_preterminal;
	my_uint symbol_number;

public:
	inpre_symbol(my_uint sym, bool is_pre) {
		symbol_number = sym;
		is_preterminal = is_pre;
	}

	inpre_symbol() {
		symbol_number = (my_uint) -1;
		is_preterminal = false;
	}

	bool is_pre() const {
		return is_preterminal;
	}

	my_uint sym() const {
		return symbol_number;
	}

	bool operator==(const inpre_symbol& s) const {
		return equals(s);
	}

	bool equals(const inpre_symbol& s) const {
		return ((sym() == s.sym()) && (is_pre() == s.is_pre()));
	}
};

namespace std {
namespace tr1 {
template<> struct hash<inpre_symbol> {
public:
	size_t operator()(const inpre_symbol& k) const {
		return (k.sym() << 1) + ((my_uint) k.is_pre());
	}
};
}
}

template<class T, class E> class grammar_alphabet {

public:
	virtual ~grammar_alphabet() {
	}

	typedef unordered_set<T> preterminal_set;
	typedef unordered_set<T> nonterminal_set;

	virtual bool
	chain_ends_in_nonterminal(inpre_symbol complex, inpre_symbol a) = 0;

	virtual T reverse_nonterminal_lookup(my_uint a) = 0;

	virtual my_uint nonterminal_lookup(T c) = 0;

	virtual my_uint preterminal_lookup(T c) = 0;

	virtual T nonterminal_lookup_string(string nt) = 0;

	virtual E vocab_lookup_string(string nt) = 0;

	virtual T preterminal_lookup_string(string nt) = 0;

	virtual my_uint nonterminal_count() const = 0;

	virtual my_uint inpreterminal_count() const = 0;

	virtual preterminal_set& preterminals() = 0;

	virtual nonterminal_set& nonterminals() = 0;

	virtual inpre_symbol prenonterminal_lookup_string(string nt) = 0;

	virtual string nonterminal_to_string(inpre_symbol sym) = 0;

	virtual string vocab_to_string(vocab_symbol sym) = 0;
};

class cnf_grammar_alphabet: public grammar_alphabet<inpre_symbol, vocab_symbol> {

protected:
	typedef unordered_map<string, inpre_symbol> str2sym_map;
	typedef unordered_map<string, vocab_symbol> str2vsym_map;

	typedef unordered_map<inpre_symbol, string> sym2str_map;
	typedef unordered_map<vocab_symbol, string> vsym2str_map;

	unordered_map<inpre_symbol, unordered_set<inpre_symbol> > chains;

	str2sym_map str2sym;
	str2vsym_map str2vsym;

	sym2str_map sym2str;
	vsym2str_map vsym2str;

	preterminal_set preterminals_list;
	nonterminal_set nonterminals_list;

	int nonterminal_num, preterminal_num, vocab_num;

public:

	bool chain_ends_in_nonterminal(inpre_symbol complex, inpre_symbol a) {
		if (a == complex) {
			return true;
		}

		if (chains.find(a) == chains.end()) {
			return false;
		}

		auto iter = chains.find(a);

		return ((iter->second).find(complex) != (iter->second).end());
	}

	string nonterminal_to_string(inpre_symbol sym) {
		//assert(sym2str.find(sym) != sym2str.end());

		if (sym2str.find(sym) == sym2str.end()) {
			return "NULL";
		} else {
			return sym2str[sym];
		}
	}

	string vocab_to_string(vocab_symbol sym) {
		assert(vsym2str.find(sym) != vsym2str.end());

		return vsym2str[sym];
	}

	preterminal_set& preterminals() {
		return preterminals_list;
	}

	nonterminal_set& nonterminals() {
		return nonterminals_list;
	}

	cnf_grammar_alphabet() {
		nonterminal_num = 0;
		preterminal_num = 0;
		vocab_num = 0;
	}

	my_uint nonterminal_count() const {
		return nonterminal_num;
	}

	my_uint inpreterminal_count() const {
		return nonterminal_num + preterminal_num;
	}

	inpre_symbol reverse_nonterminal_lookup(my_uint a) {
		return inpre_symbol(a, false);
	}

	my_uint nonterminal_lookup(inpre_symbol c) {
		return c.sym();
	}

	my_uint preterminal_lookup(inpre_symbol c) {
		return c.sym();
	}

	inpre_symbol nonterminal_lookup_string(string nt) {
		if (str2sym.find(nt) == str2sym.end()) {
			inpre_symbol s(nonterminal_num, false);
			nonterminal_num++;

			str2sym[nt] = s;
			sym2str[s] = nt;

			nonterminals_list.insert(s);

			vector<string> vec;

			string_utils::split(vec, nt, "|");

			if (vec.size() > 1) {
				inpre_symbol a = nonterminal_lookup_string(vec[vec.size() - 1]);

				chains[a].insert(s);
			} else {
				chains[s].insert(s);
			}

			return s;
		} else {
			return str2sym[nt];
		}
	}

	inpre_symbol prenonterminal_lookup_string(string nt) {
		if (str2sym.find(nt) == str2sym.end()) {
			cerr << "Could not find nonterminal " << nt << endl;
		}

		assert(str2sym.find(nt) != str2sym.end());

		return str2sym[nt];
	}

	vocab_symbol vocab_lookup_string(string nt) {
		if (str2vsym.find(nt) == str2vsym.end()) {
			vocab_symbol s(vocab_num);
			vocab_num++;

			str2vsym[nt] = s;
			vsym2str[s] = nt;

			return s;
		} else {
			return str2vsym[nt];
		}
	}

	inpre_symbol preterminal_lookup_string(string nt) {
		if (str2sym.find(nt) == str2sym.end()) {
			inpre_symbol s(preterminal_num, true);
			preterminal_num++;

			str2sym[nt] = s;
			sym2str[s] = nt;

			preterminals_list.insert(s);

			vector<string> vec;

			string_utils::split(vec, nt, "|");

			if (vec.size() > 1) {
				inpre_symbol a = preterminal_lookup_string(vec[vec.size() - 1]);

				chains[a].insert(s);
			} else {
				chains[s].insert(s);
			}

			return s;
		} else {
			return str2sym[nt];
		}
	}
};

template<class T, class E> class unweighted_cnf_grammar {
public:
	typedef unordered_set<cnf_term_rule<T, E> > cnf_term_rule_set;

	typedef unordered_set<cnf_binary_rule<T> > cnf_binary_rule_set;

	typedef unordered_map<E, cnf_term_rule_set> symbol_cnf_term_rule_set;

	typedef unordered_set<T> cnf_nonterm_set;

	unordered_map<T, cnf_binary_rule_set> cnf_binary_rule_b;

	unordered_map<T, cnf_binary_rule_set> cnf_binary_rule_c;

	unordered_map<T, unordered_map<T, cnf_binary_rule_set> >
			cnf_binary_rule_ab_true;

	unordered_map<T, unordered_map<T, cnf_binary_rule_set> >
			cnf_binary_rule_ab_false;

	unordered_map<T, unordered_map<T, cnf_binary_rule_set> >
			cnf_binary_rule_ac_true;

	unordered_map<T, unordered_map<T, cnf_binary_rule_set> >
			cnf_binary_rule_ac_false;

	cnf_nonterm_set roots_set;

	symbol_cnf_term_rule_set pre_map;

	grammar_alphabet<T, E>* g_alphabet;

	cnf_term_rule_set empty_cnf_term_rule_set;

	cnf_binary_rule_set empty_cnf_binary_rule_set;

	unordered_map<T, cnf_binary_rule_set> cnf_binary_rule_nonterminals; // only those with 2 nonterminals

	unordered_map<T, cnf_binary_rule_set> cnf_binary_rule_nonterminals_all;

public:

	unweighted_cnf_grammar<T, E> (grammar_alphabet<T, E>* alphabet) {
		g_alphabet = alphabet;
	}

	grammar_alphabet<T, E>* alphabet() {
		return g_alphabet;
	}

	unordered_map<T, cnf_binary_rule_set>& all_binary_rules() {
		return cnf_binary_rule_nonterminals_all;
	}

	symbol_cnf_term_rule_set& all_term_rules() {
		return pre_map;
	}

	cnf_term_rule_set& preterminal_rules_with_term(E sym) {
		if (pre_map.find(sym) == pre_map.end()) {
			return empty_cnf_term_rule_set;
		} else {
			return pre_map[sym];
		}
	}

	cnf_nonterm_set& roots() {
		return roots_set;
	}

	cnf_binary_rule_set& binary_rules_with_b(T b_sym) {
		if (cnf_binary_rule_b.find(b_sym) == cnf_binary_rule_b.end()) {
			return empty_cnf_binary_rule_set;
		} else {
			return cnf_binary_rule_b[b_sym];
		}
	}

	cnf_binary_rule_set& binary_rules_with_c(T c_sym) {
		if (cnf_binary_rule_c.find(c_sym) == cnf_binary_rule_c.end()) {
			return empty_cnf_binary_rule_set;
		} else {
			return cnf_binary_rule_c[c_sym];
		}
	}

	cnf_binary_rule_set& binary_rules_with_ab(T a_sym, T b_sym, bool b) {
		if (b) {
			if (cnf_binary_rule_ab_true[a_sym].find(b_sym)
					== cnf_binary_rule_ab_true[a_sym].end()) {
				return empty_cnf_binary_rule_set;
			} else {
				return cnf_binary_rule_ab_true[a_sym][b_sym];
			}
		} else {
			if (cnf_binary_rule_ab_false[a_sym].find(b_sym)
					== cnf_binary_rule_ab_false[a_sym].end()) {
				return empty_cnf_binary_rule_set;
			} else {
				return cnf_binary_rule_ab_false[a_sym][b_sym];
			}
		}
	}

	cnf_binary_rule_set& binary_rules_with_nonterminals(T a_sym) {
		if (cnf_binary_rule_nonterminals.find(a_sym)
				== cnf_binary_rule_nonterminals.end()) {
			return empty_cnf_binary_rule_set;
		} else {
			return cnf_binary_rule_nonterminals[a_sym];
		}
	}

	cnf_binary_rule_set& binary_rules_with_ac(T a_sym, T c_sym, bool b) {
		if (b) {
			if (cnf_binary_rule_ac_true[a_sym].find(c_sym)
					== cnf_binary_rule_ac_true[a_sym].end()) {
				return empty_cnf_binary_rule_set;
			} else {
				return cnf_binary_rule_ac_true[a_sym][c_sym];
			}
		} else {
			if (cnf_binary_rule_ac_false[a_sym].find(c_sym)
					== cnf_binary_rule_ac_false[a_sym].end()) {
				return empty_cnf_binary_rule_set;
			} else {
				return cnf_binary_rule_ac_false[a_sym][c_sym];
			}
		}
	}

};

// T for inpre_symbol, E for vocab_symbol, P for weight
template<class T, class E, class P> class cnf_grammar: public unweighted_cnf_grammar<
		T, E> {

protected:
	unordered_map<T, P> cnf_root_rule_weights;

	unordered_map<cnf_binary_rule<T> , P> cnf_binary_rule_weights;

	unordered_map<cnf_term_rule<T, E> , P> cnf_term_rule_weights;

	abstract_semiring<P>* sr;

	int rule_count;

public:
	cnf_grammar<T, E, P> (abstract_semiring<P>* sr,
			grammar_alphabet<T, E>* alphabet) :
		unweighted_cnf_grammar<T, E> (alphabet) {
		this->sr = sr;
		rule_count = 0;
	}

	int size() {
		return rule_count;
	}

	void apply_underflow_prevention_factor(double f) {
		for (auto iter = cnf_term_rule_weights.begin(); iter
				!= cnf_term_rule_weights.end(); iter++) {
			sr->mul_by_scalar(iter->second, f);
		}
	}

	abstract_semiring<P>* semiring() {
		return sr;
	}

	P weight(cnf_binary_rule<T> rule) {
		if (cnf_binary_rule_weights.find(rule) == cnf_binary_rule_weights.end()) {
			return sr->zero();
		} else {
			return cnf_binary_rule_weights[rule];
		}
	}

	P weight(cnf_term_rule<T, E> rule) {
		if (cnf_term_rule_weights.find(rule) == cnf_term_rule_weights.end()) {
			return sr->zero();
		} else {
			return cnf_term_rule_weights[rule];
		}
	}

	P weight(T root) {
		if (this->roots_set.find(root) == this->roots_set.end()) {
			return sr->zero();
		} else {
			return cnf_root_rule_weights[root];
		}
	}

	P& weight_ref(cnf_binary_rule<T> rule) {
		return cnf_binary_rule_weights[rule];
	}

	P& weight_ref(cnf_term_rule<T, E> rule) {
		return cnf_term_rule_weights[rule];
	}

	P& weight_ref(T root) {
		return cnf_root_rule_weights[root];
	}

	void add_root(T nonterm, P weight) {
		this->roots_set.insert(nonterm);

		cnf_root_rule_weights[nonterm] = weight;
	}

	void add_binary_rule(cnf_binary_rule<T> rule, P weight) {

		if (cnf_binary_rule_weights.find(rule) == cnf_binary_rule_weights.end()) {
			this->cnf_binary_rule_b[rule.b()].insert(rule);
			this->cnf_binary_rule_c[rule.c()].insert(rule);
			if (rule.c().is_pre()) {
				this->cnf_binary_rule_ab_true[rule.a()][rule.b()].insert(rule);
			} else {
				this->cnf_binary_rule_ab_false[rule.a()][rule.b()].insert(rule);
			}

			if (rule.b().is_pre()) {
				this->cnf_binary_rule_ac_true[rule.a()][rule.c()].insert(rule);
			} else {
				this->cnf_binary_rule_ac_false[rule.a()][rule.c()].insert(rule);
			}

			if (!rule.b().is_pre() && !rule.c().is_pre()) {
				this->cnf_binary_rule_nonterminals[rule.a()].insert(rule);
			}

			this->cnf_binary_rule_nonterminals_all[rule.a()].insert(rule);
		}

		cnf_binary_rule_weights[rule] = weight;
		rule_count++;
	}

	void add_term_rule(cnf_term_rule<T, E> rule, P weight) {
		cnf_term_rule_weights[rule] = weight;

		this->pre_map[rule.x()].insert(rule);
		rule_count++;
	}
};

class latent_variable_cnf_state_mapper {
public:
	virtual int m(inpre_symbol a) = 0;
};

namespace std
{
namespace tr1
{
template<> struct hash<pair<inpre_symbol, my_uint>> {
public:
	size_t operator()(const pair<inpre_symbol, my_uint>& h) const {
		return (h.first.sym() ^ (h.second << 1));
	}
};
}
}

class latent_variable_cnf_grammar: public cnf_grammar<inpre_symbol,
		vocab_symbol, simple_tensor> , public latent_variable_cnf_state_mapper {
protected:
	unordered_map<inpre_symbol, my_uint> ms;

	bool areSamplingSlicesPrepared; // are the sampling slices prepared?

	class BinaryRuleSampleInformation {
	public:
		BinaryRuleSampleInformation(inpre_symbol b, inpre_symbol c,
				my_uint h_b, my_uint h_c, double v) {
			value = v;
			this->b = b;
			this->c = c;
			this->h_b = h_b;
			this->h_c = h_c;
		}

		double value;
		inpre_symbol b, c;
		my_uint h_b, h_c;
	};

	class TermRuleSampleInformation {
	public:
		TermRuleSampleInformation(vocab_symbol x_, double v) {
			value = v;
			x = x_;
		}

		double value;
		vocab_symbol x;
	};

	class RootSampleInformation {
	public:
		RootSampleInformation(inpre_symbol a_, my_uint h_, double v) {
			value = v;
			h = h_;
			a = a_;
		}

		double value;
		inpre_symbol a;
		my_uint h;
	};

	unordered_map<pair<inpre_symbol, my_uint> , vector<
			BinaryRuleSampleInformation> > binaryRuleSampleSlices;

	unordered_map<pair<inpre_symbol, my_uint> , vector<
			TermRuleSampleInformation> > termRuleSampleSlices;

	vector<RootSampleInformation> rootSampleSlices;

public:

	latent_variable_cnf_grammar(abstract_semiring<simple_tensor>* sr,
			grammar_alphabet<inpre_symbol, vocab_symbol>* alphabet) :
		cnf_grammar<inpre_symbol, vocab_symbol, simple_tensor> (sr, alphabet) {

		areSamplingSlicesPrepared = false;
	}

	virtual ~latent_variable_cnf_grammar() {
		destroy();
	}

	// This function prepeares all the slices that can be later used to sample from the grammar
	// (sampling here refers to sampling a priori from the grammar, not given a string)
	void prepareSamplingSlices() {

		if (!checkIfNormalized(0.99, 1.01))
		{
			cerr<<"Warning: grammar is not normalized. Still creating slices, but sampling will be wrong."<<endl;
		}

		for (auto iter = cnf_term_rule_weights.begin(); iter
				!= cnf_term_rule_weights.end(); iter++) {
			auto rule = (*iter).first;
			auto rule_weights = (*iter).second;

			assert(rule_weights.is_vector());

			for (auto h = 0 * rule_weights.cols(); h < rule_weights.cols(); h++) {
				termRuleSampleSlices[pair<inpre_symbol, my_uint> (rule.a(), h)].push_back(
						TermRuleSampleInformation(rule.x(), rule_weights.get(h)));
			}
		}

		for (auto iter = cnf_binary_rule_weights.begin(); iter
				!= cnf_binary_rule_weights.end(); iter++) {
			auto rule = (*iter).first;
			auto rule_weights = (*iter).second;

			assert(rule_weights.is_tensor());

			for (auto h_a = 0 * rule_weights.slices(); h_a
					< rule_weights.slices(); h_a++) {
				for (auto h_b = 0 * rule_weights.rows(); h_b
						< rule_weights.rows(); h_b++) {
					for (auto h_c = 0 * rule_weights.cols(); h_c
							< rule_weights.cols(); h_c++) {

						binaryRuleSampleSlices[pair<inpre_symbol, my_uint> (
								rule.a(), h_a)].push_back(
								BinaryRuleSampleInformation(rule.b(), rule.c(),
										h_b, h_c,
										rule_weights.get(h_a, h_b, h_c)));
					}
				}
			}
		}

		for (auto iter = cnf_root_rule_weights.begin(); iter
				!= cnf_root_rule_weights.end(); iter++) {
			auto sym = (*iter).first;
			auto sym_weight = (*iter).second;

			assert(sym_weight.is_vector());

			for (auto h_a = 0 * sym_weight.cols(); h_a < sym_weight.cols(); h_a++) {
				rootSampleSlices.push_back(
						RootSampleInformation(sym, h_a, sym_weight.get(h_a)));
			}
		}

		areSamplingSlicesPrepared = true;
	}

	// TODO

	cnf_binary_rule<inpre_symbol> sampleBinaryRule(inpre_symbol a, my_uint h,
			my_uint& h_2, my_uint& h_3) {

		assert(areSamplingSlicesPrepared);

		double v = 0.0;

		double prob_sampled = ((rand() + 0.0) / RAND_MAX);

		auto& sliceVector =
				binaryRuleSampleSlices[pair<inpre_symbol, my_uint> (a, h)];

		for (auto i = 0 * sliceVector.size(); i < sliceVector.size(); i++) {
			auto& item = sliceVector[i];

			v += item.value;

			if (v > prob_sampled) {
				h_2 = item.h_b;
				h_3 = item.h_c;

				return cnf_binary_rule<inpre_symbol> (a, item.b, item.c);
			}
		}

		cerr
				<< "Warning: reached the end of binary rule list and couldn't find a sample."
				<< endl;

		auto& item = sliceVector[0];
		h_2 = item.h_b;
		h_3 = item.h_c;

		return cnf_binary_rule<inpre_symbol> (a, item.b, item.c);
	}

	cnf_term_rule<inpre_symbol, vocab_symbol> sampleTermRule(inpre_symbol a,
			my_uint h) {

		assert(areSamplingSlicesPrepared);

		double v = 0.0;

		double prob_sampled = ((rand() + 0.0) / RAND_MAX);

		auto& sliceVector = termRuleSampleSlices[pair<inpre_symbol, my_uint> (
				a, h)];

		for (auto i = 0 * sliceVector.size(); i < sliceVector.size(); i++) {
			auto& item = sliceVector[i];

			v += item.value;

			if (v > prob_sampled) {
				return cnf_term_rule<inpre_symbol, vocab_symbol> (a, item.x);
			}
		}

		cerr
				<< "Warning: reached the end of term rule list and couldn't find a sample."
				<< endl;

		auto& item = sliceVector[0];

		return cnf_term_rule<inpre_symbol, vocab_symbol> (a, item.x);
	}

	inpre_symbol sampleRoot(my_uint& h) {

		assert(areSamplingSlicesPrepared);

		double v = 0.0;

		double prob_sampled = ((rand() + 0.0) / RAND_MAX);

		for (auto i = 0 * rootSampleSlices.size(); i < rootSampleSlices.size(); i++) {
			auto& item = rootSampleSlices[i];

			v += item.value;

			if (v > prob_sampled) {
				h = item.h;

				return item.a;
			}
		}

		cerr
				<< "Warning: reached the end of root list and couldn't find a sample."
				<< endl;

		auto& item = rootSampleSlices[0];

		h = item.h;
		return item.a;
	}

	bool checkIfNormalized(double min_value, double max_value) {
		for (auto iter = cnf_binary_rule_nonterminals_all.begin(); iter
				!= cnf_binary_rule_nonterminals_all.end(); iter++) {

			auto& ruleset = iter->second;

			int slices_count = -1;

			for (auto iter_ = ruleset.begin(); iter_ != ruleset.end(); iter_++) {
				if (slices_count == -1) {
					slices_count = weight(*iter_).slices();
				} else {
					assert(slices_count == weight(*iter_).slices());
				}
			}

			for (int h = 0; h < slices_count; h++) {
				double v = 0.0;

				for (auto iter_ = ruleset.begin(); iter_ != ruleset.end(); iter_++) {
					v = v + weight(*iter_).sum_slice(h);
				}

				if ((v < min_value) || (v > max_value))
				{
					cerr<<"Warning with normalization check for interminals: v = "<<v<<endl;
					return false;
				}
				/*for (auto iter_ = ruleset.begin(); iter_ != ruleset.end(); iter_++) {
					cnf_binary_rule_weights[*iter_].divide_slice(h, v);
				}*/
			}
		}

		double root_sum = 0.0;

		for (auto iter = roots_set.begin(); iter != roots_set.end(); iter++) {
			root_sum += weight(*iter).sum();
		}

		if ((root_sum < min_value) || (root_sum > max_value))
		{
			cerr<<"Warning with normalization check for root: v = "<<root_sum<<endl;
			return false;
		}

		unordered_map<inpre_symbol, simple_tensor> pre_total;

		for (auto iter = pre_map.begin(); iter != pre_map.end(); iter++) {
			for (auto iter_ = iter->second.begin(); iter_ != iter->second.end(); iter_++) {
				assert(cnf_term_rule_weights[*iter_].cols()> 0);

				pre_total[iter_->a()].plus_vector(cnf_term_rule_weights[*iter_]);
			}
		}

		for (auto iter = pre_total.begin(); iter != pre_total.end(); iter++) {
			auto a = (*iter).first;
			auto v = (*iter).second;

			assert(v.is_vector());

			for (auto i=0*v.cols(); i < v.cols(); i++)
			{
				if ((v.get(i) < min_value) || (v.get(i) > max_value))
				{
					cerr<<"Warning with normalization check for preterminals: v = "<<v.get(i)<<endl;
					return false;
				}
			}
		}

		for (auto iter = alphabet()->preterminals().begin(); iter
				!= alphabet()->preterminals().end(); iter++) {
			pre_total[*iter].destroy();
		}

		return true;
	}


	void destroy() {

		for (auto iter = cnf_root_rule_weights.begin(); iter
				!= cnf_root_rule_weights.end(); iter++) {
			iter->second.destroy();
		}

		for (auto iter = cnf_binary_rule_weights.begin(); iter
				!= cnf_binary_rule_weights.end(); iter++) {
			iter->second.destroy();
		}

		for (auto iter = cnf_term_rule_weights.begin(); iter
				!= cnf_term_rule_weights.end(); iter++) {
			iter->second.destroy();
		}
	}

	int m(inpre_symbol a) {
		if (ms.find(a) == ms.end()) {
			return 1;
		} else {
			return ms[a];
		}
	}

	void set_m(inpre_symbol a, int m_) {
		ms[a] = m_;
	}

	void inc_weight(cnf_binary_rule<inpre_symbol> rule, int m_a, int m_b,
			int m_c, double v) {
		if (cnf_binary_rule_weights.find(rule) == cnf_binary_rule_weights.end()) {
			simple_tensor tensor(m_a, m_b, m_c, v);
			add_binary_rule(rule, tensor);
			//cnf_binary_rule_weights[rule] = tensor;
		} else {
			cnf_binary_rule_weights[rule].inc_scalar(v);
		}
	}

	void inc_weight(cnf_term_rule<inpre_symbol, vocab_symbol> rule, int m_a,
			double v) {
		if (cnf_term_rule_weights.find(rule) == cnf_term_rule_weights.end()) {
			simple_tensor vec(m_a, v);
			add_term_rule(rule, vec);
			//cnf_term_rule_weights[rule] = vec;
		} else {
			cnf_term_rule_weights[rule].inc_scalar(v);
		}
	}

	void inc_weight(inpre_symbol root, int m_a, double v) {
		if (this->roots_set.find(root) == this->roots_set.end()) {
			simple_tensor vec(m_a, v);
			//cnf_root_rule_weights[root] = vec;
			add_root(root, vec);
		} else {
			cnf_root_rule_weights[root].inc_scalar(v);
		}
	}

	simple_tensor& weight_ref(cnf_binary_rule<inpre_symbol> rule, int m_a,
			int m_b, int m_c) {
		if (cnf_binary_rule_weights.find(rule) == cnf_binary_rule_weights.end()) {
			simple_tensor tensor(m_a, m_b, m_c, 0.0);
			add_binary_rule(rule, tensor);
			//cnf_binary_rule_weights[rule] = tensor;
		}
		return cnf_binary_rule_weights[rule];
	}

	simple_tensor& weight_ref(cnf_term_rule<inpre_symbol, vocab_symbol> rule,
			int m_a) {
		if (cnf_term_rule_weights.find(rule) == cnf_term_rule_weights.end()) {
			simple_tensor vec(m_a, 0.0);
			add_term_rule(rule, vec);
			//cnf_term_rule_weights[rule] = vec;
		}
		return cnf_term_rule_weights[rule];
	}

	simple_tensor& weight_ref(inpre_symbol root, int m_a) {
		if (this->roots_set.find(root) == this->roots_set.end()) {
			simple_tensor vec(m_a, 0.0);
			//cnf_root_rule_weights[root] = vec;
			add_root(root, vec);
		}
		return cnf_root_rule_weights[root];
	}

	void noise_matsuzaki() {
		for (auto iter = cnf_binary_rule_nonterminals_all.begin(); iter
				!= cnf_binary_rule_nonterminals_all.end(); iter++) {
			auto& ruleset = iter->second;

			for (auto iter_ = ruleset.begin(); iter_ != ruleset.end(); iter_++) {
				simple_tensor& t = cnf_binary_rule_weights[*iter_];

				for (int i = 0; i < t.slices(); i++) {
					for (int j = 0; j < t.rows(); j++) {
						for (int k = 0; k < t.cols(); k++) {
							t.set(i, j, k,
									t.get(i, j, k) * exp(matsuzaki_rand()));
						}
					}
				}
			}
		}

		for (auto iter = roots_set.begin(); iter != roots_set.end(); iter++) {
			simple_tensor& t = cnf_root_rule_weights[*iter];

			for (int i = 0; i < t.cols(); i++) {
				t.set(i, t.get(i) * exp(matsuzaki_rand()));
			}
		}

		unordered_map<inpre_symbol, double> pre_total;

		for (auto iter = pre_map.begin(); iter != pre_map.end(); iter++) {
			for (auto iter_ = iter->second.begin(); iter_ != iter->second.end(); iter_++) {
				simple_tensor& t = cnf_term_rule_weights[*iter_];

				for (int i = 0; i < t.cols(); i++) {
					t.set(i, t.get(i) * exp(matsuzaki_rand()));
				}
			}
		}
	}

	my_double matsuzaki_rand() {
		return ((double) rand() / (RAND_MAX)) * 2 * log(3) - log(3);
	}

	void normalize_grammar() {
		for (auto iter = cnf_binary_rule_nonterminals_all.begin(); iter
				!= cnf_binary_rule_nonterminals_all.end(); iter++) {

			auto& ruleset = iter->second;

			int slices_count = -1;

			for (auto iter_ = ruleset.begin(); iter_ != ruleset.end(); iter_++) {
				if (slices_count == -1) {
					slices_count = weight(*iter_).slices();
				} else {
					assert(slices_count == weight(*iter_).slices());
				}
			}

			for (int h = 0; h < slices_count; h++) {
				double v = 0.0;

				for (auto iter_ = ruleset.begin(); iter_ != ruleset.end(); iter_++) {
					v = v + weight(*iter_).sum_slice(h);
				}

				for (auto iter_ = ruleset.begin(); iter_ != ruleset.end(); iter_++) {
					cnf_binary_rule_weights[*iter_].divide_slice(h, v);
				}
			}
		}

		double root_sum = 0.0;

		for (auto iter = roots_set.begin(); iter != roots_set.end(); iter++) {
			root_sum += weight(*iter).sum();
		}

		for (auto iter = roots_set.begin(); iter != roots_set.end(); iter++) {
			cnf_root_rule_weights[*iter].divide(root_sum);
		}

		unordered_map<inpre_symbol, simple_tensor> pre_total;

		for (auto iter = pre_map.begin(); iter != pre_map.end(); iter++) {
			for (auto iter_ = iter->second.begin(); iter_ != iter->second.end(); iter_++) {
				assert(cnf_term_rule_weights[*iter_].cols()> 0);

				pre_total[iter_->a()].plus_vector(cnf_term_rule_weights[*iter_]);
			}
		}

		for (auto iter = pre_map.begin(); iter != pre_map.end(); iter++) {
			for (auto iter_ = iter->second.begin(); iter_ != iter->second.end(); iter_++) {
				cnf_term_rule_weights[*iter_].divide_vector(
						pre_total[iter_->a()]);
			}
		}

		for (auto iter = alphabet()->preterminals().begin(); iter
				!= alphabet()->preterminals().end(); iter++) {
			pre_total[*iter].destroy();
		}
	}




	bool no_latent() {
		for (auto iter = cnf_binary_rule_weights.begin(); iter
				!= cnf_binary_rule_weights.end(); iter++) {

			if ((iter->second.cols() != 1) || (iter->second.rows() != 1)
					|| (iter->second.slices() != 1)) {
				return false;
			}

		}

		for (auto iter = cnf_term_rule_weights.begin(); iter
				!= cnf_term_rule_weights.end(); iter++) {
			if (iter->second.cols() != 1) {
				return false;
			}
		}

		for (auto iter = cnf_root_rule_weights.begin(); iter
				!= cnf_root_rule_weights.end(); iter++) {
			if (iter->second.cols() != 1) {
				return false;
			}
		}

		return true;
	}

	cnf_grammar<inpre_symbol, vocab_symbol, my_double>* create_double_grammar(
			abstract_semiring<my_double>* rsr) {

		auto g = new cnf_grammar<inpre_symbol, vocab_symbol, my_double> (rsr,
				alphabet());

		auto& my_roots = roots();

		for (auto iter = my_roots.begin(); iter != my_roots.end(); iter++) {
			auto t = weight(*iter);

			g->add_root(*iter, t.get(0));
		}

		for (auto iter = cnf_binary_rule_weights.begin(); iter
				!= cnf_binary_rule_weights.end(); iter++) {

			g->add_binary_rule(iter->first, iter->second.get(0, 0, 0));
		}

		for (auto iter = cnf_term_rule_weights.begin(); iter
				!= cnf_term_rule_weights.end(); iter++) {

			g->add_term_rule(iter->first, iter->second.get(0));
		}

		return g;
	}

	latent_variable_cnf_grammar* create_latent_pcfg_grammar(
			abstract_semiring<simple_tensor>* rsr,
			latent_variable_cnf_state_mapper* mapper) {

		auto g = new latent_variable_cnf_grammar(rsr, alphabet());

		auto& my_roots = roots();

		for (auto iter = my_roots.begin(); iter != my_roots.end(); iter++) {
			auto t = weight(*iter);

			if (!t.empty()) {
				assert(t.cols() == 1);

				simple_tensor tensor(mapper->m(*iter), t.get(0));

				g->add_root(*iter, tensor);
			}
		}

		for (auto iter = cnf_binary_rule_weights.begin(); iter
				!= cnf_binary_rule_weights.end(); iter++) {

			auto t = iter->second;

			if (!t.empty()) {
				assert(t.cols() == 1);
				assert(t.rows() == 1);
				assert(t.slices() == 1);

				simple_tensor tensor(mapper->m(iter->first.a()),
						mapper->m(iter->first.b()), mapper->m(iter->first.c()),
						t.get(0, 0, 0));

				g->add_binary_rule(iter->first, tensor);
			}
		}

		for (auto iter = cnf_term_rule_weights.begin(); iter
				!= cnf_term_rule_weights.end(); iter++) {

			auto t = iter->second;

			if (!t.empty()) {
				assert(t.cols() == 1);

				simple_tensor tensor(mapper->m(iter->first.a()), t.get(0));

				g->add_term_rule(iter->first, tensor);
			}
		}

		return g;
	}

	bool save_grammar_file_flat(string filename) {
		ofstream myfile(filename.c_str());

		if (!myfile.is_open()) {
			return false;
		}

		myfile << "### Beginning of grammar ###" << endl;

		myfile << "preterminals";
		for (auto iter = alphabet()->preterminals().begin(); iter
				!= alphabet()->preterminals().end(); iter++) {
			myfile << " " << alphabet()->nonterminal_to_string(*iter);

		}
		myfile << endl;

		myfile << "interminals";
		for (auto iter = alphabet()->nonterminals().begin(); iter
				!= alphabet()->nonterminals().end(); iter++) {
			myfile << " " << alphabet()->nonterminal_to_string(*iter);

		}
		myfile << endl;

		for (auto iter = cnf_binary_rule_nonterminals_all.begin(); iter
				!= cnf_binary_rule_nonterminals_all.end(); iter++) {
			auto& ruleset = iter->second;

			for (auto iter_ = ruleset.begin(); iter_ != ruleset.end(); iter_++) {
				simple_tensor v = cnf_binary_rule_weights[*iter_];


				for (int i = 0; i < v.slices(); i++) {
					for (int j = 0; j < v.rows(); j++) {
						for (int k = 0; k < v.cols(); k++) {
							if (v.get(i, j, k) > 0) {
							myfile << "binary " << alphabet()->nonterminal_to_string(
							iter_->a()) << i << " -> "
							<< alphabet()->nonterminal_to_string(iter_->b()) << j <<" "
							<< alphabet()->nonterminal_to_string(iter_->c()) << k <<" "
							<< "1 1 1";

							myfile << " 1,1,1:" << v.get(i, j, k) << endl;
							}
						}
					}
				}
			}
		}

		for (auto iter = roots_set.begin(); iter != roots_set.end(); iter++) {
			simple_tensor v = weight(*iter);


			for (int i = 0; i < v.cols(); i++) {
				if (v.get(i) > 0) {
				myfile << "root " << alphabet()->nonterminal_to_string(*iter) << i
						<< " 1";
				myfile << " 1:" << v.get(i)<<endl;
				}
			}
		}

		for (auto iter = pre_map.begin(); iter != pre_map.end(); iter++) {
			for (auto iter_ = iter->second.begin(); iter_ != iter->second.end(); iter_++) {
				simple_tensor v = cnf_term_rule_weights[*iter_];

				for (int i = 0; i < v.cols(); i++) {
					if (v.get(i) > 0) {
					myfile << "term " << alphabet()->nonterminal_to_string(
						iter_->a()) << i << " -> " << alphabet()->vocab_to_string(
						iter_->x()) << " 1";
					myfile << " 1:" << v.get(i)<<endl;
					}
				}
			}
		}

		myfile << "### End of grammar ###" << endl;

		myfile.close();

		return true;
	}

	bool save_grammar_file(string filename) {
		ofstream myfile(filename.c_str());

		if (!myfile.is_open()) {
			return false;
		}

		myfile << "### Beginning of grammar ###" << endl;

		myfile << "preterminals";
		for (auto iter = alphabet()->preterminals().begin(); iter
				!= alphabet()->preterminals().end(); iter++) {
			myfile << " " << alphabet()->nonterminal_to_string(*iter);

		}
		myfile << endl;

		myfile << "interminals";
		for (auto iter = alphabet()->nonterminals().begin(); iter
				!= alphabet()->nonterminals().end(); iter++) {
			myfile << " " << alphabet()->nonterminal_to_string(*iter);

		}
		myfile << endl;

		for (auto iter = cnf_binary_rule_nonterminals_all.begin(); iter
				!= cnf_binary_rule_nonterminals_all.end(); iter++) {
			auto& ruleset = iter->second;

			for (auto iter_ = ruleset.begin(); iter_ != ruleset.end(); iter_++) {
				simple_tensor v = cnf_binary_rule_weights[*iter_];

				myfile << "binary " << alphabet()->nonterminal_to_string(
						iter_->a()) << " -> "
						<< alphabet()->nonterminal_to_string(iter_->b()) << " "
						<< alphabet()->nonterminal_to_string(iter_->c()) << " "
						<< v.slices() << " " << v.rows() << " " << v.cols();

				for (int i = 0; i < v.slices(); i++) {
					for (int j = 0; j < v.rows(); j++) {
						for (int k = 0; k < v.cols(); k++) {
							myfile << " " << v.get(i, j, k);
						}
					}
				}

				myfile << endl;
			}
		}

		for (auto iter = roots_set.begin(); iter != roots_set.end(); iter++) {
			simple_tensor v = weight(*iter);

			myfile << "root " << alphabet()->nonterminal_to_string(*iter)
					<< " " << v.cols();

			for (int i = 0; i < v.cols(); i++) {
				myfile << " " << v.get(i);
			}

			myfile << endl;
		}

		for (auto iter = pre_map.begin(); iter != pre_map.end(); iter++) {
			for (auto iter_ = iter->second.begin(); iter_ != iter->second.end(); iter_++) {
				simple_tensor v = cnf_term_rule_weights[*iter_];
				myfile << "term " << alphabet()->nonterminal_to_string(
						iter_->a()) << " -> " << alphabet()->vocab_to_string(
						iter_->x()) << " " << v.cols();

				for (int i = 0; i < v.cols(); i++) {
					myfile << " " << v.get(i);
				}

				myfile << endl;
			}
		}

		myfile << "### End of grammar ###" << endl;

		myfile.close();

		return true;
	}

	void parse_grammar_file(string filename, int max_m) {
		string line;

		ifstream myfile(filename.c_str());

		debug::msg("reading grammar file");

		cerr<<"bound on number of latent states: "<<max_m<<endl;

		if (myfile.is_open()) {
			while (getline(myfile, line)) {
				parse_grammar_line(line, max_m);
			}

			myfile.close();
		} else
			debug::msg("could not read grammar file");

		//save_grammar_file("/tmp/x");
	}

	void parse_grammar_file(string filename) {
		cerr<<"Warning: reading grammar without bound on maximal latent state"<<endl;
		parse_grammar_file(filename, 10000);
	}

	void parse_grammar_line(string line, int max_m) {
		string hash = "#";

		if (!line.compare(string_utils::EMPTY_STRING)) {
			return;
		}

		if (std::equal(hash.begin(), hash.end(), line.begin())) {
			return;
		}

		vector<string> tokens;

		string_utils::split(tokens, line, " ");

		string cmd = tokens[0];

		tokens.erase(tokens.begin());

		if (!cmd.compare("preterminals")) {
			preterminals_from_string(tokens);
		} else if (!cmd.compare("interminals")) {
			nonterminals_from_string(tokens);
		} else if (!cmd.compare("binary")) {
			simple_tensor weight;

			cnf_binary_rule<inpre_symbol> rule = binary_rule_from_string(
					tokens, weight, max_m);

			set_m(rule.a(), weight.slices());
			set_m(rule.b(), weight.rows());
			set_m(rule.c(), weight.cols());

			add_binary_rule(rule, weight);

		} else if (!cmd.compare("term")) {
			simple_tensor weight;

			cnf_term_rule<inpre_symbol, vocab_symbol> rule =
					term_rule_from_string(tokens, weight, max_m);

			set_m(rule.a(), weight.cols());

			add_term_rule(rule, weight);
		} else if (!cmd.compare("root")) {
			simple_tensor weight;

			inpre_symbol sym = root_from_string(tokens, weight, max_m);

			set_m(sym, weight.cols());

			add_root(sym, weight);
		} else {
			debug::msg("unknown grammar command:");
			debug::msg(cmd);
		}
	}

	inpre_symbol root_from_string(vector<string>& tokens, simple_tensor& weight, int max_m) {
		string nt = tokens[0];

		int orig_m_a = stoi(tokens[1]);

		int m_a = orig_m_a;

		if (m_a > max_m) { m_a = max_m; }

		auto c = 2 + 0 * tokens.size();

		simple_tensor vec(m_a, 0.0);

		if (tokens[c].find(':') != std::string::npos) {
			for (auto ii = c; ii < tokens.size(); ii++) {
				auto token = tokens[ii];

				vector<string> out1;

				string_utils::split(out1, token, ":");

				int cor_a = stoi(out1[0]) - 1;

				if (cor_a < m_a) {
					vec.set(cor_a, stod(out1[1]));
				}
			}
		} else {
			for (auto i = 0 * m_a; i < m_a; i++) {
				vec.set(i, stod(tokens[c++]));
			}
		}

		weight = vec;

		return alphabet()->nonterminal_lookup_string(nt);
	}

	void preterminals_from_string(vector<string>& tokens) {
		for (auto i = 0 * tokens.size(); i < tokens.size(); i++) {
			alphabet()->preterminal_lookup_string(tokens[i]);
		}
	}

	void nonterminals_from_string(vector<string>& tokens) {
		for (auto i = 0 * tokens.size(); i < tokens.size(); i++) {
			alphabet()->nonterminal_lookup_string(tokens[i]);
		}
	}

	cnf_term_rule<inpre_symbol, vocab_symbol> term_rule_from_string(
			vector<string>& tokens, simple_tensor& weight, int max_m) {
		assert(tokens.size() >= 4);

		string pre = tokens[0];

		assert(!tokens[1].compare("->"));

		string rhs = tokens[2];

		int orig_m_a = stoi(tokens[3]);

		int m_a = orig_m_a;

		if (m_a > max_m) { m_a = max_m; }

		simple_tensor vec(m_a, 0);

		auto c = 4 + 0 * tokens.size();

		if ((c < tokens.size()) && (tokens[c] != ""))
		{
		if (tokens[c].find(':') != std::string::npos) {
			for (auto ii = c; ii < tokens.size(); ii++) {
				auto token = tokens[ii];

				vector<string> out1;
				vector<string> out2;

				string_utils::split(out1, token, ":");

				int cor_a = stoi(out1[0]) - 1;

				if (cor_a < m_a) {
					vec.set(cor_a, stod(out1[1]));
				}
			}
		} else {
			for (auto i = 0 * m_a; i < m_a; i++) {
				vec.set(i, stod(tokens[c++]));
			}
		}
		}

		weight = vec;

		return cnf_term_rule<inpre_symbol, vocab_symbol> (
				alphabet()->prenonterminal_lookup_string(pre),
				alphabet()->vocab_lookup_string(rhs));
	}

	cnf_binary_rule<inpre_symbol> binary_rule_from_string(
			vector<string>& tokens, simple_tensor& weight, int max_m) {
		assert(tokens.size() >= 5);

		string nt = tokens[0];

		assert(!tokens[1].compare("->"));

		string rhs1 = tokens[2];
		string rhs2 = tokens[3];

		int orig_m_a = stoi(tokens[4]);
		int orig_m_b = stoi(tokens[5]);
		int orig_m_c = stoi(tokens[6]);

		int m_a = orig_m_a;
		int m_b = orig_m_b;
		int m_c = orig_m_c;

		if (m_a > max_m) { m_a = max_m; }
		if (m_b > max_m) { m_b = max_m; }
		if (m_c > max_m) { m_c = max_m; }

		simple_tensor tensor(m_a, m_b, m_c, 0);

		auto c = 7 + 0 * tokens.size();

		if (tokens[c].find(':') != std::string::npos) {
			for (auto ii = c; ii < tokens.size(); ii++) {
				auto token = tokens[ii];

				vector<string> out1;
				vector<string> out2;

				string_utils::split(out1, token, ":");

				string_utils::split(out2, out1[0], ",");

				int cor_a = stoi(out2[0]) - 1;
				int cor_b = stoi(out2[1]) - 1;
				int cor_c = stoi(out2[2]) - 1;

				if ((cor_a < m_a) && (cor_b < m_b) && (cor_c < m_c))
				{
					tensor.set(cor_a, cor_b, cor_c, stod(out1[1]));
				}
			}
		} else {
			/*cerr<<nt<<" "<<rhs1<<" "<<rhs2<<" ";
			 cerr<<m_a<<" "<<m_b<<" "<<m_c<<" "<<tokens.size()<<endl;*/
			for (int i = 0; i < orig_m_a; i++) {
				for (int j = 0; j < orig_m_b; j++) {
					for (int k = 0; k < orig_m_c; k++) {
						if ((i < m_a) && (j < m_b) && (k < m_c)) {
							tensor.set(i, j, k, stod(tokens[c++]));
						} else { c++; }
					}
				}
			}
		}

		weight = tensor;

		return cnf_binary_rule<inpre_symbol> (
				alphabet()->prenonterminal_lookup_string(nt),
				alphabet()->prenonterminal_lookup_string(rhs1),
				alphabet()->prenonterminal_lookup_string(rhs2));
	}
};

template<class T, class P> class arbitrary_pcfg_grammar {
private:

	abstract_semiring<P>* sr;

	unordered_map<T, unordered_set<arbitrary_cfg_rule<T> > > rules;

	unordered_set<T> _nonterminals;

	unordered_map<arbitrary_cfg_rule<T> , P> weights;

public:
	// todo
	// complete this

	arbitrary_pcfg_grammar<T, P> (abstract_semiring<P> *sr) {
		this->sr = sr;
	}

	void add_arbitrary_pcfg_rule(arbitrary_cfg_rule<T> rule, P w) {
		rules[rule.lhs()].insert(rule);
		_nonterminals.insert(rule.lhs());

		weights[rule] = w;
	}

	const unordered_set<T>& root_nonterminals() {
		// todo need to change only to root nonterminals

		return _nonterminals;
	}

	const unordered_set<T>& nonterminals() {
		// todo need to change only to root nonterminals

		return _nonterminals;
	}

	const unordered_set<arbitrary_cfg_rule<T> >& rule_set(T a) {
		return rules[a];
	}

	abstract_semiring<P>* semiring() {
		return sr;
	}

	P weight(arbitrary_cfg_rule<T> rule) {
		if (weights.find(rule) == weights.end()) {
			return sr->zero();
		} else {
			return weights[rule];
		}
	}
};

#endif /*GRAMMAR_H_*/
