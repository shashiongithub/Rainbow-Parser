/*
 * ParseCombinationChart.h
 *
 *  Created on: Mar 19, 2015
 *      Author: scohen
 */

#ifndef PARSECOMBINATIONCHART_H_
#define PARSECOMBINATIONCHART_H_

#include "grammar.h"
#include "trees.h"
#include <math.h>

template<class L>
class ParseCombinationChart: public cnf_io_viterbi {
protected:

	typedef unordered_map<inpre_symbol, unordered_map<int, unordered_map<int,
			unordered_map<int, double> > > > InChart;
	typedef unordered_map<inpre_symbol, unordered_map<int, unordered_map<int,
			double> > > PreChart;
	typedef unordered_map<inpre_symbol, double> RootChart;

	vector<InChart> ins;
	vector<PreChart> pres;
	vector<RootChart> roots;

	bool isPlus;

	unweighted_cnf_grammar<inpre_symbol, vocab_symbol>* grammar;

public:

	ParseCombinationChart(
			unweighted_cnf_grammar<inpre_symbol, vocab_symbol>* grammar,
			max_log_semiring<cnf_double_backpointer_pair<inpre_symbol,
					vocab_symbol> >* sr, bool plusOrNot) :
		cnf_io_viterbi(sr) {
		this->grammar = grammar;
		isPlus = plusOrNot;
	}

	void addChart(string chartString) {
		PreChart pre;
		InChart in;
		RootChart root;

		vector<string> tokens;

		string_utils::split(tokens, chartString, ";");

		for (auto i = 0 * tokens.size(); i < tokens.size(); i++) {
			vector<string> localTokens;

			string_utils::split(localTokens, tokens[i], " ");

			if (localTokens[0] == "in") {
				inpre_symbol a =
						grammar->alphabet()->prenonterminal_lookup_string(
								localTokens[1]);
				int i = stoi(localTokens[2]);
				int k = stoi(localTokens[3]);
				int j = stoi(localTokens[4]);
				double d = stod(localTokens[5]);

				in[a][i][k][j] = d;
			} else if (localTokens[0] == "pre") {
				inpre_symbol a =
						grammar->alphabet()->prenonterminal_lookup_string(
								localTokens[1]);
				int i = stoi(localTokens[2]);
				int j = stoi(localTokens[3]);
				double d = stod(localTokens[4]);

				pre[a][i][j] = d;
			} else if (localTokens[0] == "rt") {
				inpre_symbol a =
						grammar->alphabet()->prenonterminal_lookup_string(
								localTokens[1]);
				double d = stod(localTokens[4]);

				root[a] = d;
			}
		}

		ins.push_back(in);
		pres.push_back(pre);
		roots.push_back(root);
	}

	virtual ~ParseCombinationChart() {
	}

	unweighted_cnf_grammar<inpre_symbol, vocab_symbol>* current_unweighted_grammar() {
		return grammar;
	}

	cnf_double_backpointer_pair<inpre_symbol, vocab_symbol> inside_span_weight(
			const cnf_term_rule<inpre_symbol, vocab_symbol>& rule, my_uint s1,
			my_uint s2) {

		my_double marg = NEG_INF;

		auto a = rule.a();

		for (auto l = 0 * pres.size(); l < pres.size(); l++) {
			auto& my_set = pres[l];

			if (my_set.find(a) != my_set.end()) {
				if (my_set[a].find(s1) != my_set[a].end()) {
					if (my_set[a][s1].find(s2) != my_set[a][s1].end()) {
						if (marg == NEG_INF) {
							marg = my_set[a][s1][s2];
						} else {
							if (isPlus) {
								marg = marg + my_set[a][s1][s2];
							} else {
								marg = marg + (log(my_set[a][s1][s2]) + 1000000);
							}
						}
					}
				}
			}
		}

		return cnf_double_backpointer_pair<inpre_symbol, vocab_symbol> (
				rule.a(), s1, s2, s2, marg, true);
	}

	cnf_double_backpointer_pair<inpre_symbol, vocab_symbol> root_span_weight(
			inpre_symbol a, my_uint s1, my_uint s2) {

		double marg = NEG_INF;

		for (auto i = 0 * roots.size(); i < roots.size(); i++) {
			auto& my_set = roots[i];

			if (my_set.find(a) != my_set.end()) {
				if (marg == NEG_INF) {
					marg = my_set[a];
				} else {
					if (isPlus) {
						marg = marg + my_set[a];
					} else {
						marg = marg + (log(my_set[a]) + 1000000);
					}
				}
			}
		}

		return cnf_double_backpointer_pair<inpre_symbol, vocab_symbol> (a, s1,
				s2, s2, marg, false);
	}

	void _create_io_tables_further(my_uint num_symbols, my_uint num_states) {

	}

	cnf_double_backpointer_pair<inpre_symbol, vocab_symbol> inside_span_weight(
			const cnf_binary_rule<inpre_symbol>& rule, my_uint i, my_uint k,
			my_uint j) {

		my_double marg = NEG_INF;

		auto a = rule.a();

		for (auto l = 0 * ins.size(); l < ins.size(); l++) {
			auto& my_set = ins[l];

			if (my_set.find(a) != my_set.end()) {
				if (my_set[a].find(i) != my_set[a].end()) {
					if (my_set[a][i].find(k) != my_set[a][i].end()) {
						if (my_set[a][i][k].find(j) != my_set[a][i][k].end()) {
							if (marg == NEG_INF) {
								marg = my_set[a][i][k][j];
							} else {
								if (isPlus) {
									marg = marg + my_set[a][i][k][j];
								} else {
									marg = marg + (log(my_set[a][i][k][j]) + 1000000);
								}
							}
						}
					}
				}
			}
		}

		return cnf_double_backpointer_pair<inpre_symbol, vocab_symbol> (
				rule.a(), i, k, j, marg, false);
	}

};

#endif /* PARSECOMBINATIONCHART_H_ */
