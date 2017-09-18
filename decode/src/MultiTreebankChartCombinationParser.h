#ifndef MULTITREEBANKCHARTCOMBINATIONPARSER_H_
#define MULTITREEBANKCHARTCOMBINATIONPARSER_H_

class MultiTreebankChartCombinationParser {
private:
	cnf_vocab_reducer vocab;

public:

	void parse(string grammarFile, vector<string> chartFiles,
			string treebankFile, string vocabFile, int threshold, bool isPlus) {

		max_log_semiring<
				cnf_double_backpointer_pair<inpre_symbol, vocab_symbol> >
				viterbiSemiring;

		cnf_grammar_alphabet alphabet;

		tensor_semiring tensorSemiring;

		real_semiring realSemiring;

		latent_variable_cnf_grammar grammar(&tensorSemiring, &alphabet);

		grammar.parse_grammar_file(grammarFile);

		cnf_grammar<inpre_symbol, vocab_symbol, my_double> *pruneGrammar;

		pruneGrammar = grammar.create_double_grammar(&realSemiring);

		vocab.load(vocabFile);

		vocab.set_threshold(threshold);


		vector<ifstream*> charts;

		for (auto i = 0 * chartFiles.size(); i < chartFiles.size(); i++) {
			ifstream* tb = new ifstream(chartFiles[i]);

			if (tb == NULL) {
				cerr << "Could not open chart file " << chartFiles[i]
						<< ". Exiting.";

				return;
			}

			charts.push_back(tb);
		}

		penn_treebank my_tb;

		bool b = my_tb.open_file(treebankFile, true);

		if (!b) {
			cerr << "Could not open treebank file " << treebankFile
					<< ". Exiting.";

			return;
		}

		while (!charts[0]->eof()) {

			my_tb.next_tree();

			auto tree = my_tb.current_penn_tree();

			vector<string> chartLines;

			for (auto i = 0 * charts.size(); i < charts.size(); i++) {
				string myString;

				getline(*charts[i], myString);

				chartLines.push_back(myString);
			}

			if (tree->children_count() > 1) {
				vocab.modify_tree(tree);

				vector < string > tokens = tree->get_terminal_yield();

				vocab_symbol* sent_ptr = new vocab_symbol[tokens.size()];

				for (auto i = 0 * tokens.size(); i < tokens.size(); i++) {
					sent_ptr[i] = alphabet.vocab_lookup_string(tokens[i]);
				}

				array_1d<vocab_symbol> sent(sent_ptr, tokens.size());

				ParseCombinationChart<string> vit(&grammar, &viterbiSemiring, isPlus);

				for (auto i = 0 * chartLines.size(); i < chartLines.size(); i++) {
					vit.addChart(chartLines[i]);
				}

				time_t t1 = time(0);

				cerr << "Preparing pruner charts" << endl;

				vit.io_start_string(sent, NULL);

				cerr << "Calculating goal" << endl;

				vit.goal();

				time_t t2 = time(0);

				cerr << (t2 - t1) << " seconds with goal " << vit.goal()
						<< endl;

				string tree_str = vit.goal_string();

				cerr << "Output tree: " << tree_str << endl;

				cout << "(TOP " << tree_str << " )" << endl;

				sent.destroy();

				my_tb.destroy_current_tree(tree);
			} else {
				cerr << "Skipping" << endl;

				cout << "()" << endl;
			}
		}

		for (auto i = 0 * charts.size(); i < charts.size(); i++) {
			delete charts[i];
		}

		delete pruneGrammar;
	}
};

#endif /* MULTITREEBANKCOMBINATIONPARSER_H_ */
