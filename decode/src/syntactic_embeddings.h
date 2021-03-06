#ifndef SYNTACTIC_EMBEDDINGS_H_
#define SYNTACTIC_EMBEDDINGS_H_

class syntactic_embeddings_generator {
private:

	latent_variable_cnf_grammar* current_grammar;
	tensor_semiring* tensor_sr;
	real_semiring* real_sr;
	cnf_grammar_alphabet* alphabet;
	cnf_grammar<inpre_symbol, vocab_symbol, my_double> *prune_grammar;
	cnf_vocab_reducer* vocab;
	max_log_semiring< cnf_double_backpointer_pair<inpre_symbol, vocab_symbol> >
	* viterbi_sr;

public:
	syntactic_embeddings_generator() {
		alphabet = NULL;
		prune_grammar = NULL;
		vocab = NULL;

		tensor_sr = new tensor_semiring();
		real_sr = new real_semiring();
		viterbi_sr
		= new max_log_semiring< cnf_double_backpointer_pair<inpre_symbol, vocab_symbol> >();
	}

	virtual ~syntactic_embeddings_generator() {
		delete tensor_sr;
		delete real_sr;
		delete viterbi_sr;

		delete current_grammar;
		current_grammar = NULL;

		delete prune_grammar;
		prune_grammar = NULL;

		delete alphabet;
		alphabet = NULL;

		delete vocab;
		vocab = NULL;
	}

	void print_embeddings(penn_treebank* ptb, string base_grammar_file,
			string prune_grammar_file, string vocab_file, int threshold,
			double prune_threshold, bool constrain_chart, bool enforce_preterminals) {

		alphabet = new cnf_grammar_alphabet();

		current_grammar = new latent_variable_cnf_grammar(tensor_sr, alphabet);

		current_grammar->parse_grammar_file(base_grammar_file);

		vector<string> filenames;

		cnf_grammar<inpre_symbol, vocab_symbol, my_double> *prune_grammar;

		max_log_semiring<cnf_double_backpointer_pair<inpre_symbol, vocab_symbol> >
		viterbi_semiring;

		{
			latent_variable_cnf_grammar tmp_grammar(tensor_sr, alphabet);
			tmp_grammar.parse_grammar_file(prune_grammar_file);
			prune_grammar = tmp_grammar.create_double_grammar(real_sr);
		}

		vocab = new cnf_vocab_reducer();

		vocab->load(vocab_file);
		vocab->set_threshold(threshold);

		bool enforce_pre = enforce_preterminals;

		ptb->reset_iterator();

		while (ptb->next_tree()) {
			auto tree = ptb->current_penn_tree();

			if (tree->children_count()> 1) {
				vocab->modify_tree(tree);

				vector<string> pre = tree->get_preterminal_yield();

				vector<inpre_symbol> pre_symbols;

				for (auto i=0*pre.size(); i < pre.size(); i++) {
					pre_symbols.push_back(alphabet->prenonterminal_lookup_string(pre[i]));
				}

				tictoc tt_tree("tree", "tree");

				tt_tree.tic();

				vector<string> tokens = tree->get_terminal_yield();

				// Get words

				vocab_symbol* sent_ptr = new vocab_symbol[tokens.size()];

				cerr<<"sentence length: "<<tokens.size()<<endl;

				for (auto i=0*tokens.size(); i < tokens.size(); i++) {
					sent_ptr[i] = alphabet->vocab_lookup_string(tokens[i]);
					cerr<<tokens[i]<<" ";

					if (i > 0) { cout<<" "; }

					cout<<tokens[i];
				}

				cerr<<endl;
				cout<<endl;

				array_1d<vocab_symbol> sent(sent_ptr, tokens.size());

				// Read cnf file

				constituent_bag_interface<inpre_symbol, inpre_symbol>
				* lpcfg_pruner;

				cnf_io_simple_tensor latent_io(tensor_sr);
				cnf_io<my_double> pruner_io(real_sr);

				cnf_simple_pruner pruner(&pruner_io, 1e-5);
				pruner_io.io_start_string(sent, prune_grammar);

				latent_io.io_start_string(sent, current_grammar, &pruner,
						prune_grammar);

				lpcfg_pruner = new cnf_lpcfg_pruner(&latent_io, prune_threshold);

				if (enforce_pre) {
					lpcfg_pruner->enforce_tags(pre_symbols);
				}

				cerr<<"NoLatentStates pruner score: "<<pruner_io.goal()<<endl;

				// Prepare score io and parse

				bool flipSignTrue = true;

				cnf_io_mbr_lpcfg* score_io = new cnf_io_mbr_lpcfg(viterbi_sr, &latent_io, flipSignTrue, false);

				score_io->set_pruner(lpcfg_pruner);

				score_io->io_start_string(sent, NULL, lpcfg_pruner, NULL);

				string decoded_tree_string = score_io->goal_string();

				delete lpcfg_pruner;

				delete score_io;

				sent.destroy();

				tt_tree.toc();

				cerr<<decoded_tree_string<<endl;


				cout<<decoded_tree_string<<endl;

				auto cur_tree =
				pennbank_tree<string>::from_string(decoded_tree_string);
				assert(cur_tree != NULL);

				// Step 2: compute the embeddings				
				{
					//vocab->modify_tree(cur_tree);

					auto bag = cur_tree->create_bag(alphabet);

					vector<string> pre = cur_tree->get_preterminal_yield();

					vector<inpre_symbol> pre_symbols;

					for (auto i=0*pre.size(); i < pre.size(); i++) {
						pre_symbols.push_back(alphabet->prenonterminal_lookup_string(pre[i]));
					}

					tictoc tt_tree("tree", "tree");

					tt_tree.tic();

					vector<string> tokens = cur_tree->get_terminal_yield();

					// Get words

					vocab_symbol* sent_ptr = new vocab_symbol[tokens.size()];

					cerr<<"sentence length: "<<tokens.size()<<endl;

					for (auto i=0*tokens.size(); i < tokens.size(); i++) {
						sent_ptr[i] = alphabet->vocab_lookup_string(tokens[i]);
						cerr<<tokens[i]<<" ";
					}

					cerr<<endl;

					array_1d<vocab_symbol> sent(sent_ptr, tokens.size());

					// Read cnf file

					//constituent_bag_interface<inpre_symbol, inpre_symbol>
					//* lpcfg_pruner;

					cnf_io_simple_tensor latent_io(tensor_sr);
					cnf_io<my_double> pruner_io(real_sr);

					cnf_simple_pruner pruner(&pruner_io, 1e-5);
					pruner_io.io_start_string(sent, prune_grammar);

					latent_io.io_start_string(sent, current_grammar); //, &pruner, prune_grammar);

					/*lpcfg_pruner = new cnf_lpcfg_pruner(&latent_io, prune_threshold);

					if (enforce_pre) {
						lpcfg_pruner->enforce_tags(pre_symbols);
					}

					cerr<<"NoLatentStates pruner score: "<<pruner_io.goal()
					<<endl;

					// Prepare score io and parse

					cnf_io_mbr_lpcfg* score_io = new cnf_io_mbr_lpcfg(viterbi_sr, &latent_io);

					score_io->set_pruner(lpcfg_pruner);

					score_io->io_start_string(sent, NULL); //, lpcfg_pruner, NULL);

					score_io->io_constrain_bag(bag);

					string decoded_tree_string = score_io->goal_string();

					delete lpcfg_pruner;

					delete score_io;*/

					if (constrain_chart) {
		                        	latent_io.io_constrain_bag(bag);
					}

					auto chart = &latent_io;

					bag->reset_binary_rule_iterator();

					while (bag->next_binary_rule()) {
						cnf_binary_rule<inpre_symbol> rule = bag->current_binary_rule();

						int i = bag->current_binary_left_corner_state();
						int j = bag->current_binary_right_corner_state();

						simple_tensor v_a = chart->outside(rule.a(), i, j);
						simple_tensor v_a_in = chart->inside(rule.a(), i, j);

						cout<<alphabet->nonterminal_to_string(rule.a())<<" "<<i<<" "<<j<<" o";

						
						double s = 0.0;

						if (v_a.cols() > 0) {
							for (int i=0; i < v_a.cols(); i++)
							{
								s += v_a.get(i);
							}

							if (s > 0) {
								s = 1.0;

								for (int i=0; i < v_a.cols(); i++)
								{
									cout<<" "<<(v_a.get(i)/s);
								}
							} else {
								for (int i=0; i < v_a_in.cols(); i++)
								{
									cout<<" 0";
								}
							}
						} else {
							for (int i=0; i < v_a_in.cols(); i++)
							{
								cout<<" 0";
							}
						}

						s = 0.0;

						cout<<" i";
						if (v_a_in.cols() > 0) {
							for (int i=0; i < v_a_in.cols(); i++)
							{
								s += v_a_in.get(i);
							}

							if (s > 0) {
								s = 1.0;

								for (int i=0; i < v_a_in.cols(); i++)
								{
									cout<<" "<<(v_a_in.get(i) / s);
								}
							} else {
								for (int i=0; i < v_a.cols(); i++)
								{
									cout<<" 0";
								}
							}
						} else {
							for (int i=0; i < v_a.cols(); i++)
							{
								cout<<" 0";
							}
						}
						
						cout<<endl;
					}

					bag->reset_term_rule_iterator();

					while (bag->next_term_rule()) {
						cnf_term_rule<inpre_symbol, vocab_symbol> rule =
						bag->current_term_rule();
						int i = bag->current_term_left_corner_state();
						int j = bag->current_term_right_corner_state();

						simple_tensor v_a_in = chart->inside(rule.a(), i, j);
						simple_tensor v_a_out = chart->outside(rule.a(), i, j);

						cout<<alphabet->nonterminal_to_string(rule.a())<<" "<<i<<" "<<j<<" o";

						double s = 0.0;

						if (v_a_out.cols() > 0) {
							for (int i=0; i < v_a_out.cols(); i++)
							{
								s += v_a_out.get(i);
							}

							if (s > 0) {
								s = 1.0;

								for (int i=0; i < v_a_out.cols(); i++)
								{
									cout<<" "<<(v_a_out.get(i)/s);
								}
							} else {
								for (int i=0; i < v_a_in.cols(); i++)
								{
									cout<<" 0";
								}
							}
						}  else {
							for (int i=0; i < v_a_in.cols(); i++)
							{
								cout<<" 0";
							}
						}

						s = 0.0;

						cout<<" i";

						if (v_a_in.cols() > 0) {
							for (int i=0; i < v_a_in.cols(); i++)
							{
								s += v_a_in.get(i);
							}

							if (s > 0) {
								s = 1.0;

								for (int i=0; i < v_a_in.cols(); i++)
								{
									cout<<" "<<(v_a_in.get(i) / s);
								}
							} else {
								for (int i=0; i < v_a_out.cols(); i++)
								{
									cout<<" 0";
								}
							}
						} else {
							for (int i=0; i < v_a_out.cols(); i++)
							{
								cout<<" 0";
							}
						}

						cout<<endl;
					}

					sent.destroy();

					tt_tree.toc();
					cout<<endl;
				  }

				} else {
					cerr<<"Skipping"<<endl;

					cout<<"Word of length 1. Skipping";
					cout<<tree->to_string()<<endl;
					cout<<endl;

					cerr<<"()"<<endl;
				}
			}

			delete prune_grammar;
			prune_grammar = NULL;

			delete vocab;
			vocab = NULL;

			delete alphabet;
			alphabet = NULL;

			delete current_grammar;
			current_grammar = NULL;
		}

	};

#endif /*SYNTACTIC_EMBEDDINGS_H_*/
