/*
 * ChartFilter.h
 *
 *  Created on: Jul 1, 2015
 *      Author: scohen
 */

#ifndef CHARTFILTER_H_
#define CHARTFILTER_H_

class ChartFilter {

public:
	void filter(string grammarFile, string chartFilename, string treebankFile) {

		max_log_semiring<
				cnf_double_backpointer_pair<inpre_symbol, vocab_symbol> >
				viterbiSemiring;

		cnf_grammar_alphabet alphabet;

		tensor_semiring tensorSemiring;

		latent_variable_cnf_grammar grammar(&tensorSemiring, &alphabet);

		grammar.parse_grammar_file(grammarFile);

		ifstream chartFile(chartFilename.c_str());

		if (!chartFile.is_open()) {
			cerr << "Could not open chart file " << chartFilename
					<< ". Exiting.";

			return;
		}

		penn_treebank my_tb;

		bool b = my_tb.open_file(treebankFile, true);

		if (!b) {
			cerr << "Could not open treebank file " << treebankFile
					<< ". Exiting.";

			return;
		}

		int treeNumber = 0;

		while (my_tb.next_tree()) {

			treeNumber++;


			auto tree = my_tb.current_penn_tree();

			string chartLine;

			getline(chartFile, chartLine);

			cerr<<"Tree "<<treeNumber<<": "<<tree->to_string()<<endl;

			if (tree->children_count() > 1) {

				ParseCombinationChart<string> vit(&grammar, &viterbiSemiring,
						true);

				vit.addChart(chartLine);

				ostringstream str;

				cerr<<"Creating bag"<<endl;
				cnf_span_bag* bag = tree->create_bag(grammar.alphabet());

				bag->reset_binary_rule_iterator();

				cerr<<"Iterating over binary rules"<<endl;
				while (bag->next_binary_rule()) {
					auto rule = bag->current_binary_rule();

					auto i = bag->current_binary_left_corner_state();
					auto j = bag->current_binary_right_corner_state();
					auto k = bag->mid_point(i, j);

					auto margVit = vit.inside_span_weight(rule, i, k, j);

					auto marg = margVit.v();

					str << "in " << grammar.alphabet()->nonterminal_to_string(
							rule.a()) << " " << i << " " << k << " " << j
							<< " " << marg<<";";
				}

				bag->reset_term_rule_iterator();

				cerr<<"Iterating over terminal rules"<<endl;
				while (bag->next_term_rule()) {
					auto rule = bag->current_term_rule();

					auto i = bag->current_term_left_corner_state();
					auto j = bag->current_term_right_corner_state();

					auto margVit = vit.inside_span_weight(rule, i, j);

					auto marg = margVit.v();

					str << "pre " << grammar.alphabet()->nonterminal_to_string(
							rule.a()) << " " << i << " " << j << " " << marg<<";";
				}

				cerr<<"Printing root"<<endl;
				auto margVit = vit.inside_span_weight(bag->get_root_rule(),
						bag->get_root_left_corner(),
						bag->mid_point(bag->get_root_left_corner(), bag->get_root_right_corner()),
						bag->get_root_right_corner());

				auto marg = margVit.v();

				str << "rt " << grammar.alphabet()->nonterminal_to_string(
						bag->get_root()) << " " << bag->get_root_left_corner()
						<< " " << bag->get_root_right_corner() << " " << marg;

				cout<<str.str()<<endl;

				tree->destroy_bag(bag);

				//my_tb.destroy_current_tree(tree);
			} else {
				cout<<"## Tree skipped here because is of length 1"<<endl;
			}
		}

		cerr<<"Finished filtering out charts"<<endl;
	}
};

#endif /* CHARTFILTER_H_ */
