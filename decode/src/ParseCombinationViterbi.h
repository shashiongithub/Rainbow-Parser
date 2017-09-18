#ifndef __parse_combination_h__
#define __parse_combination_h__

#include "grammar.h"
#include "trees.h"
#include <math.h>

template<class L>
class ParseCombinationViterbi: public cnf_io_viterbi {
protected:
	vector<cnf_span_bag*> parseSpanBags;
	vector<pennbank_tree<L>*> trees;
	unweighted_cnf_grammar<inpre_symbol, vocab_symbol>* grammar;

public:

	ParseCombinationViterbi(unweighted_cnf_grammar<inpre_symbol, vocab_symbol>* grammar, max_log_semiring< cnf_double_backpointer_pair<inpre_symbol, vocab_symbol> >* sr) : cnf_io_viterbi(sr) {
		this->grammar = grammar;
	}

	void addTree(pennbank_tree<L>* tree) {
		parseSpanBags.push_back(
				tree->create_bag(current_unweighted_grammar()->alphabet()));
		trees.push_back(tree);
	}

	virtual ~ParseCombinationViterbi() {
		for (auto i = 0 * trees.size(); i < trees.size(); i++) {
			trees[i]->destroy_bag(parseSpanBags[i]);
			//delete trees[i];
		}
	}

	unweighted_cnf_grammar<inpre_symbol, vocab_symbol>* current_unweighted_grammar() {
		return grammar;
	}

	cnf_double_backpointer_pair<inpre_symbol, vocab_symbol> inside_span_weight(
			const cnf_term_rule<inpre_symbol, vocab_symbol>& rule, my_uint s1,
			my_uint s2) {

		my_double marg = NEG_INF;

		for (auto i = 0 * parseSpanBags.size(); i < parseSpanBags.size(); i++) {
			auto& my_set = parseSpanBags[i]->preterminal_rules_with_term(
					rule.x(), s1, s2);

			if (my_set.find(rule) != my_set.end()) {
				if (marg == NEG_INF) { marg = 0; }
				marg++;
			}
		};

		return cnf_double_backpointer_pair<inpre_symbol, vocab_symbol> (
				rule.a(), s1, s2, s2, marg, true);
	}

	cnf_double_backpointer_pair<inpre_symbol, vocab_symbol> root_span_weight(inpre_symbol a, my_uint s1, my_uint s2) {

		double marg = NEG_INF;

		for (auto i = 0 * parseSpanBags.size(); i < parseSpanBags.size(); i++) {
			auto& my_set = parseSpanBags[i]->roots();

			if (my_set.find(a) != my_set.end()) {
				if (marg == NEG_INF) { marg = 0; }
				marg++;
			}
		}

		return cnf_double_backpointer_pair<inpre_symbol, vocab_symbol> (
				a, s1, s2, s2, marg, false);
	}

	void _create_io_tables_further(my_uint num_symbols, my_uint num_states) {

	}

	cnf_double_backpointer_pair<inpre_symbol, vocab_symbol> inside_span_weight(
			const cnf_binary_rule<inpre_symbol>& rule, my_uint i, my_uint k,
			my_uint j) {

		my_double marg = NEG_INF;

		for (auto l = 0 * parseSpanBags.size(); l < parseSpanBags.size(); l++) {
			auto& my_set = parseSpanBags[l]->binary_rules_with_ab(rule.a(),
					rule.b(), rule.c().is_pre(), i, k, j);

			if (my_set.find(rule) != my_set.end()) {
				if (marg == NEG_INF) { marg = 0; }
				marg++;
			}
		}

		return cnf_double_backpointer_pair<inpre_symbol, vocab_symbol> (
				rule.a(), i, k, j, marg, false);
	}

};

#endif
