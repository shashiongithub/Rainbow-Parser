#ifndef LPCFG_PARSER_H_
#define LPCFG_PARSER_H_

#define FLIP_SIGN_TRUE true

class lpcfg_parser {
private:
	cnf_vocab_reducer vocab;

public:
	void parseLattice(string lattice_file, string grammar_file,
			string prune_grammar_file, string vocab_filename, int threshold,
			bool enforce_pre, bool shouldPrintChart, bool multiplyOrAdd, double addVal, bool rightBranch) {
		cnf_grammar_alphabet alphabet;

		tensor_semiring tensor_sr;

		real_semiring real_sr;

		latent_variable_cnf_grammar grammar(&tensor_sr, &alphabet);

		grammar.parse_grammar_file(grammar_file);

		cnf_grammar<inpre_symbol, vocab_symbol, my_double> *prune_grammar;

		max_log_semiring<
				cnf_double_backpointer_pair<inpre_symbol, vocab_symbol> >
				viterbi_semiring;

		{
			latent_variable_cnf_grammar tmp_grammar(&tensor_sr, &alphabet);
			tmp_grammar.parse_grammar_file(prune_grammar_file);
			prune_grammar = tmp_grammar.create_double_grammar(&real_sr);
		}

		vocab.load(vocab_filename);

		vocab.set_threshold(threshold);

		ifstream myfile(lattice_file.c_str());

                if (myfile.is_open()) {
			debug::msg("lattice file opened");
                } else {
                        debug::msg("could not read lattice file");
			return;
		}

		string latticeString;

		while (getline(myfile, latticeString)) {
			cnf_io_simple_tensor io(&tensor_sr);

			cnf_io<my_double> pruner_io(&real_sr);

			cnf_simple_pruner pruner(&pruner_io, 1e-10);

			io.set_pruner(&pruner);

			time_t t1 = time(0);

			cerr << "Preparing pruner charts" << endl;

			pruner_io.io_start_lattice(latticeString, multiplyOrAdd, addVal, prune_grammar, NULL, NULL, rightBranch);

			cerr << "Calculating goal" << endl;

			pruner_io.goal();

			time_t t2 = time(0);

			cerr << (t2 - t1) << " seconds with goal " << pruner_io.goal()
				<< endl;

			cerr << "Preparing LPCFG charts" << endl;

			//io.io_start_lattice(latticeString, &grammar, &pruner, prune_grammar);
			io.io_start_lattice(latticeString, multiplyOrAdd, addVal, &grammar, &pruner, prune_grammar, rightBranch);

			t1 = time(0);

			cerr << t1 - t2 << " seconds" << endl;

			cerr << "MBR decoding" << endl;

			cnf_io_mbr_lpcfg mbr_io(&viterbi_semiring, &io, FLIP_SIGN_TRUE,
				shouldPrintChart);

			//mbr_io.io_start_lattice(latticeString, NULL, &pruner, prune_grammar);
			mbr_io.io_start_lattice(latticeString, multiplyOrAdd, addVal, NULL, NULL, NULL, rightBranch);

			string tree = mbr_io.goal_string();

			t2 = time(0);

			cerr << t2 - t1 << " seconds" << endl;

			cerr << "Output raw tree: " << tree << endl;

			auto my_tree = pennbank_tree<string>::from_string(tree);

			auto terminal_tokens = my_tree->get_terminal_yield_ref();

			tree = my_tree->to_string();

			if (shouldPrintChart) {
				mbr_io.printChart();

				cerr << "(TOP " << tree << " )" << endl;
			} else {
				double g_numeric = mbr_io.goal_numeric();

				cout << "(TOP " << tree << " )" << endl;
				cerr << "Weight: "<< g_numeric << endl;
			}
		}

		myfile.close();

		delete prune_grammar;
	}

	void parse(penn_treebank* ptb, string grammar_file,
			string prune_grammar_file, string vocab_filename, int threshold,
			bool enforce_pre, bool shouldPrintChart, int max_m) {
		cnf_grammar_alphabet alphabet;

		tensor_semiring tensor_sr;

		real_semiring real_sr;

		latent_variable_cnf_grammar grammar(&tensor_sr, &alphabet);

		grammar.parse_grammar_file(grammar_file, max_m);

		cnf_grammar<inpre_symbol, vocab_symbol, my_double> *prune_grammar;

		max_log_semiring<
				cnf_double_backpointer_pair<inpre_symbol, vocab_symbol> >
				viterbi_semiring;

		{
			latent_variable_cnf_grammar tmp_grammar(&tensor_sr, &alphabet);
			tmp_grammar.parse_grammar_file(prune_grammar_file, max_m);
			prune_grammar = tmp_grammar.create_double_grammar(&real_sr);
		}

		vocab.load(vocab_filename);

		vocab.set_threshold(threshold);

		ptb->reset_iterator();

		while (ptb->next_tree()) {
			auto tree = ptb->current_penn_tree();

			if (tree->children_count() > 1) {
				vector <string> orig_tokens = tree->get_terminal_yield();

				vocab.modify_tree(tree);

				vector < string > pre = tree->get_preterminal_yield();

				vector<inpre_symbol> pre_symbols;

				for (auto i = 0 * pre.size(); i < pre.size(); i++) {
					pre_symbols.push_back(
							alphabet.prenonterminal_lookup_string(pre[i]));
				}

				cnf_io_simple_tensor io(&tensor_sr);

				cnf_io<my_double> pruner_io(&real_sr);

				cnf_simple_pruner pruner(&pruner_io, 1e-5);

				io.set_pruner(&pruner);

				vector < string > tokens = tree->get_terminal_yield();

				vocab_symbol* sent_ptr = new vocab_symbol[tokens.size()];

				for (auto i = 0 * tokens.size(); i < tokens.size(); i++) {
					sent_ptr[i] = alphabet.vocab_lookup_string(tokens[i]);
				}

				if (enforce_pre) {
					cerr << "Enforcing tags:";
					for (auto i = 0 * pre.size(); i < pre.size(); i++) {
						cerr << " " << pre[i];
					}
					cerr << endl;
					pruner.enforce_tags(pre_symbols);
				}

				array_1d<vocab_symbol> sent(sent_ptr, tokens.size());

				time_t t1 = time(0);

				cerr << "Preparing pruner charts" << endl;

				pruner_io.io_start_string(sent, prune_grammar);

				cerr << "Calculating goal" << endl;

				pruner_io.goal();

				time_t t2 = time(0);

				cerr << (t2 - t1) << " seconds with goal " << pruner_io.goal()
						<< endl;

				cerr << "Preparing LPCFG charts" << endl;

				io.io_start_string(sent, &grammar, &pruner, prune_grammar);

				t1 = time(0);

				cerr << t1 - t2 << " seconds" << endl;

				cerr << "MBR decoding" << endl;

				cnf_io_mbr_lpcfg mbr_io(&viterbi_semiring, &io, FLIP_SIGN_TRUE,
						shouldPrintChart);

				mbr_io.io_start_string(sent, NULL, &pruner, prune_grammar);

				string tree = mbr_io.goal_string();

				t2 = time(0);

				cerr << t2 - t1 << " seconds" << endl;

				cerr << "Output raw tree: " << tree << endl;

				auto my_tree = pennbank_tree<string>::from_string(tree);

				auto terminal_tokens = my_tree->get_terminal_yield_ref();

				if (terminal_tokens.size() != orig_tokens.size()) {
					cerr << "Warning: wrong number of tokens in tree" << endl;
					cerr << "Actual number: "<<terminal_tokens.size()<<endl;
					cerr << "Expected: "<<orig_tokens.size()<<endl;
				} else {
					for (auto i = 0 * terminal_tokens.size(); i
							< terminal_tokens.size(); i++) {
						*(terminal_tokens[i]) = orig_tokens[i];
					}

					tree = my_tree->to_string();
				}

				if (shouldPrintChart) {
					mbr_io.printChart();

					cerr << "(TOP " << tree << " )" << endl;
				} else {
					double g_numeric = mbr_io.goal_numeric();

					cout << "(TOP " << tree << " )" << endl;
					cerr << "Weight: "<< g_numeric << endl;
				}
				sent.destroy();
			} else {
				cerr << "Skipping" << endl;

				double g_numeric = 0.0;

				cout << "()" << endl;
				cerr << "Weight: "<< g_numeric << endl;
			}
		}

		delete prune_grammar;
	}
};

#endif /*LPCFG_PARSER_H_*/
