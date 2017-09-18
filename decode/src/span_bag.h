#ifndef SPAN_BAG_H_
#define SPAN_BAG_H_

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
#include <tuple>
#include "grammar.h"
#include "hashes.h"

using namespace std;

using namespace std::tr1;

class cnf_span_bag {
private:
	typedef tuple<my_uint, my_uint, my_uint, inpre_symbol> key_type;

	typedef tuple<my_uint, my_uint, inpre_symbol> key_type_no_mid;

	typedef tuple<my_uint, my_uint, vocab_symbol> key_type_term;

	typedef pair<my_uint, my_uint> key_type_span;

	unordered_map<key_type, unweighted_cnf_grammar<inpre_symbol,vocab_symbol>::cnf_binary_rule_set>
			cnf_binary_rule_b;

	unordered_map<key_type, unweighted_cnf_grammar<inpre_symbol,vocab_symbol>::cnf_binary_rule_set>
			cnf_binary_rule_c;

	unordered_map<key_type_no_mid, unweighted_cnf_grammar<inpre_symbol,vocab_symbol>::cnf_binary_rule_set>
			cnf_binary_rule_a;

	unweighted_cnf_grammar<inpre_symbol,vocab_symbol>::cnf_binary_rule_set
			empty_set;

	unordered_map<key_type_term, unweighted_cnf_grammar<inpre_symbol, vocab_symbol>::cnf_term_rule_set>
			unary_rule;

	unordered_map<key_type_span, my_uint> mid_points;

	unweighted_cnf_grammar<inpre_symbol,vocab_symbol>::cnf_nonterm_set root;

	unordered_map<key_type_no_mid, unweighted_cnf_grammar<inpre_symbol,vocab_symbol>::cnf_binary_rule_set>::iterator
			iter_binary_main;
	unweighted_cnf_grammar<inpre_symbol,vocab_symbol>::cnf_binary_rule_set::iterator
			iter_binary_minor;
	bool iter_binary_first;

	unordered_map<key_type_term, unweighted_cnf_grammar<inpre_symbol, vocab_symbol>::cnf_term_rule_set>::iterator
			iter_term_main;
	unweighted_cnf_grammar<inpre_symbol, vocab_symbol>::cnf_term_rule_set::iterator
			iter_term_minor;
	bool iter_term_first;

	int root_left_corner, root_right_corner;

	cnf_binary_rule<inpre_symbol>* root_rule;

public:
	cnf_span_bag()
	{
		root_rule = NULL;
	}

	virtual ~cnf_span_bag() {
		if (root_rule != NULL) {
			delete root_rule;
		}
	}

	// binary iterator
	void reset_binary_rule_iterator() {
		iter_binary_main = cnf_binary_rule_a.begin();

		iter_binary_first = true;
	}

	bool next_binary_rule() {
		if (iter_binary_first) {
			iter_binary_first = false;

			if (iter_binary_main != cnf_binary_rule_a.end()) {
				iter_binary_minor = iter_binary_main->second.begin();

				while (iter_binary_minor == iter_binary_main->second.end()) {
					if (iter_binary_main == cnf_binary_rule_a.end()) {
						return false;
					}
					iter_binary_main++;
					if (iter_binary_main == cnf_binary_rule_a.end()) {
						return false;
					}

					iter_binary_minor = iter_binary_main->second.begin();
				}

				return true;
			} else {
				return false;
			}
		} else {
			iter_binary_minor++;

			while (iter_binary_minor == iter_binary_main->second.end()) {
				if (iter_binary_main == cnf_binary_rule_a.end()) {
					return false;
				}
				iter_binary_main++;
				if (iter_binary_main == cnf_binary_rule_a.end()) {
					return false;
				}

				iter_binary_minor = iter_binary_main->second.begin();
			}

			return true;
		}
	}

	cnf_binary_rule<inpre_symbol> current_binary_rule() {
		return *iter_binary_minor;
	}
	
	my_uint current_binary_left_corner_state()
	{
		return get<0>(iter_binary_main->first);
	}

	my_uint current_binary_right_corner_state()
	{
		return get<1>(iter_binary_main->first);
	}

	my_uint current_term_left_corner_state()
	{
		return get<0>(iter_term_main->first);
	}

	my_uint current_term_right_corner_state()
	{
		return get<1>(iter_term_main->first);
	}

	// unary iterator
	void reset_term_rule_iterator() {
		iter_term_main = unary_rule.begin();

		iter_term_first = true;
	}

	bool next_term_rule() {
		if (iter_term_first) {
			iter_term_first = false;

			if (iter_term_main != unary_rule.end()) {
				iter_term_minor = iter_term_main->second.begin();

				while (iter_term_minor == iter_term_main->second.end()) {
					if (iter_term_main == unary_rule.end()) {
						return false;
					}
					iter_term_main++;
					if (iter_term_main == unary_rule.end()) {
						return false;
					}

					iter_term_minor = iter_term_main->second.begin();
				}

				return true;
			} else {
				return false;
			}
		} else {
			iter_term_minor++;

			while (iter_term_minor == iter_term_main->second.end()) {
				if (iter_term_main == unary_rule.end()) {
					return false;
				}
				iter_term_main++;
				if (iter_term_main == unary_rule.end()) {
					return false;
				}

				iter_term_minor = iter_term_main->second.begin();
			}

			return true;
		}
	}

	cnf_term_rule<inpre_symbol, vocab_symbol> current_term_rule() {
		return *iter_term_minor;
	}

	// root "iterator"
	inpre_symbol get_root() {
		assert(root.size()==1);

		auto iter = root.begin();

		return *iter;
	}

	void add_binary_rule(cnf_binary_rule<inpre_symbol>& rule, my_uint i,
			my_uint j, my_uint k, bool is_root) {
		key_type x(i, j, k, rule.b());
		cnf_binary_rule_b[x].insert(rule);
		
		assert(cnf_binary_rule_b[x].size() == 1);

		key_type y(i, j, k, rule.c());
		cnf_binary_rule_c[y].insert(rule);
		assert(cnf_binary_rule_c[y].size() == 1);

		key_type_span z(i, k);
		
		mid_points[z] = j;

		if (is_root) {
			root_left_corner = i;
			root_right_corner = k;
			root.insert(rule.a());
			delete root_rule;
			root_rule = new cnf_binary_rule<inpre_symbol>(rule);
		}

		key_type_no_mid u(i, k, rule.a());

		cnf_binary_rule_a[u].insert(rule);
	}

	cnf_binary_rule<inpre_symbol> get_root_rule()
	{
		return *root_rule;
	}

	my_uint mid_point(my_uint i, my_uint j) {
		key_type_span z(i, j);

		assert(mid_points.find(z) != mid_points.end());

		return mid_points[z];
	}

	int get_root_left_corner()
	{
		return root_left_corner;
	}

	int get_root_right_corner()
	{
		return root_right_corner;
	}

	void add_term_rule(inpre_symbol a, vocab_symbol x, my_uint i, my_uint j) {
		key_type_term z(i, j, x);

		unary_rule[z].insert(cnf_term_rule<inpre_symbol, vocab_symbol>(a, x));
	}

	virtual unweighted_cnf_grammar<inpre_symbol,vocab_symbol>::cnf_nonterm_set& roots() {
		return root;
	}

	virtual unweighted_cnf_grammar<inpre_symbol,vocab_symbol>::cnf_binary_rule_set& binary_rules_with_ac(
			inpre_symbol a_sym, inpre_symbol c_sym, bool b, my_uint i,
			my_uint k, my_uint j) {
		key_type y(i, k, j, c_sym);

		assert(cnf_binary_rule_c[y].size() <= 1);

		if (cnf_binary_rule_c[y].size() == 0) {
			return empty_set;
		}

		auto& s = cnf_binary_rule_c[y];

		auto rule = *(s.begin());

		if ( (rule.b().is_pre() == b) && (rule.a() == a_sym)) {
			return s;
		} else {
			return empty_set;
		}
	}

	virtual unweighted_cnf_grammar<inpre_symbol,vocab_symbol>::cnf_binary_rule_set& binary_rules_with_c(
			inpre_symbol c_sym, my_uint i, my_uint k, my_uint j) {
		key_type y(i, k, j, c_sym);

		assert(cnf_binary_rule_c[y].size() <= 1);

		if (cnf_binary_rule_c[y].size() == 0) {
			return empty_set;
		}

		auto& s = cnf_binary_rule_c[y];

		//auto rule = *(s.begin());

		return s;
	}

	virtual unweighted_cnf_grammar<inpre_symbol,vocab_symbol>::cnf_binary_rule_set& binary_rules_with_ab(
			inpre_symbol a_sym, inpre_symbol b_sym, bool b, my_uint i,
			my_uint k, my_uint j) {
		key_type x(i, k, j, b_sym);

		assert(cnf_binary_rule_b[x].size() <= 1);

		if (cnf_binary_rule_b[x].size() == 0) {
			return empty_set;
		}

		auto& s = cnf_binary_rule_b[x];

		auto rule = *(s.begin());

		if ( (rule.c().is_pre() == b) && (rule.a() == a_sym)) {
			return s;
		} else {
			return empty_set;
		}
	}

	virtual unweighted_cnf_grammar<inpre_symbol,vocab_symbol>::cnf_binary_rule_set& binary_rules_with_b(
			inpre_symbol b_sym, my_uint i, my_uint k, my_uint j) {
		key_type x(i, k, j, b_sym);

		assert(cnf_binary_rule_b[x].size() <= 1);

		if (cnf_binary_rule_b[x].size() == 0) {
			return empty_set;
		}

		auto& s = cnf_binary_rule_b[x];

		//auto rule = *(s.begin());

		return s;
	}

	virtual const unweighted_cnf_grammar<inpre_symbol, vocab_symbol>::cnf_term_rule_set& preterminal_rules_with_term(
			vocab_symbol x1, my_uint s1, my_uint s2) {

		key_type_term z(s1, s2, x1);

		return unary_rule[z];
	}

	virtual unweighted_cnf_grammar<inpre_symbol,vocab_symbol>::cnf_binary_rule_set& binary_rules_with_nonterminals(
			inpre_symbol a_sym, my_uint i, my_uint j) {

		key_type_no_mid z(i, j, a_sym);

		return cnf_binary_rule_a[z];
	}

};

#endif /*SPAN_BAG_H_*/
