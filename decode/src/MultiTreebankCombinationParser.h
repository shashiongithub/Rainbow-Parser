/*
 * MultiTreebankCombinationParser.h
 *
 *  Created on: Feb 24, 2015
 *      Author: scohen
 */

#ifndef MULTITREEBANKCOMBINATIONPARSER_H_
#define MULTITREEBANKCOMBINATIONPARSER_H_

/*
 * MultiTreebankCombinationParser.cpp
 *
 *  Created on: Feb 24, 2015
 *      Author: scohen
 */

class MultiTreebankCombinationParser {
private:
	cnf_vocab_reducer vocab;

public:

	void parse(string grammarFile,
			vector<string> treebankFiles, string vocabFile, int threshold) {

		max_log_semiring<
				cnf_double_backpointer_pair<inpre_symbol, vocab_symbol> >
				viterbiSemiring;

		cnf_grammar_alphabet alphabet;

		tensor_semiring tensorSemiring;

		vocab.load(vocabFile);

		vocab.set_threshold(threshold);

		real_semiring realSemiring;

		latent_variable_cnf_grammar grammar(&tensorSemiring, &alphabet);

		grammar.parse_grammar_file(grammarFile);

		cnf_grammar<inpre_symbol, vocab_symbol, my_double> *pruneGrammar;

		pruneGrammar = grammar.create_double_grammar(&realSemiring);

		vector<penn_treebank*> treebanks;

		for (auto i = 0 * treebankFiles.size(); i < treebankFiles.size(); i++) {
			penn_treebank* tb = new penn_treebank();

			bool b = tb->open_file(treebankFiles[i]);

			if (!b) {
				cerr << "Could not open treebank file " << treebankFiles[i]
						<< ". Exiting.";

				return;
			}

			treebanks.push_back(tb);

			tb->reset_iterator();
		}

		while (treebanks[0]->next_tree()) {

			for (auto i=1 + 0*treebanks.size(); i < treebanks.size(); i++)
			{
				treebanks[i]->next_tree();

			}

			auto tree = treebanks[0]->current_penn_tree();

			if (tree->children_count() > 1) {

				for (auto i=0*treebanks.size(); i < treebanks.size(); i++) {
					vocab.modify_tree(treebanks[i]->current_penn_tree());
				}

				vector < string > tokens = tree->get_terminal_yield();

				vocab_symbol* sent_ptr = new vocab_symbol[tokens.size()];

				for (auto i = 0 * tokens.size(); i < tokens.size(); i++) {
					sent_ptr[i] = alphabet.vocab_lookup_string(tokens[i]);
				}

				array_1d<vocab_symbol> sent(sent_ptr, tokens.size());


				cerr<<"Parsing sentence: ";
				for (auto i = 0 * tokens.size(); i < tokens.size(); i++) {
                                	cerr << " " << alphabet.vocab_to_string(sent_ptr[i]);
                                }
                                cerr << endl;

				ParseCombinationViterbi<string> vit(&grammar, &viterbiSemiring);

				vit.addTree(tree);

				for (auto i = 1 + 0 * treebanks.size(); i < treebanks.size(); i++) {
					vit.addTree(treebanks[i]->current_penn_tree());
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

				for (auto i = 0 * treebanks.size(); i < treebanks.size(); i++) {
					treebanks[i]->destroy_current_tree(
							treebanks[i]->current_penn_tree());
				}
			} else {
				cerr << "Skipping" << endl;

				cout << "()" << endl;
			}
		}

		delete pruneGrammar;
	}
};

#endif /* MULTITREEBANKCOMBINATIONPARSER_H_ */
