#ifndef LPCFG_EM_H_
#define LPCFG_EM_H_

#include "tictoc.h"

class lpcfg_em : public latent_variable_cnf_state_mapper {
private:
	latent_variable_cnf_grammar* current_grammar;
	tensor_semiring* tensor_sr;
	real_semiring* real_sr;
	cnf_grammar_alphabet* alphabet;
	cnf_grammar<inpre_symbol, vocab_symbol, my_double> *prune_grammar;
	cnf_vocab_reducer vocab;

	unordered_map<my_uint, my_uint> ms;

	int corpus_size;

public:
	lpcfg_em() {
		current_grammar = NULL;
		tensor_sr = NULL;
		alphabet = NULL;
	}

	virtual ~lpcfg_em() {
		delete current_grammar;
		delete tensor_sr;
		delete alphabet;
		delete real_sr;
		delete prune_grammar;
	}

	int get_m(inpre_symbol sym, int m) {
		my_uint u = alphabet->nonterminal_lookup(sym);

		if (ms.find(u) == ms.end())
		{
			ms[u] = m;
		}

		return ms[u];
	}

	int m(inpre_symbol sym) {
		my_uint u = alphabet->nonterminal_lookup(sym);

		assert(ms.find(u) != ms.end());

		return ms[u];
	}

	void init(penn_treebank* ptb, int m, string grammar_file_prefix) {
		delete alphabet;
		delete tensor_sr;
		delete current_grammar;

		current_grammar = NULL;
		tensor_sr = NULL;
		alphabet = NULL;

		// set up the alphabet and the semiring
		alphabet = new cnf_grammar_alphabet();

		tensor_sr = new tensor_semiring();

		//current_grammar = new latent_variable_cnf_grammar(tensor_sr, alphabet);
		current_grammar = NULL;

		latent_variable_cnf_grammar* tmp_grammar =
				new latent_variable_cnf_grammar(tensor_sr, alphabet);

		assert(ptb!=NULL);

		tictoc tt_init("Initializing EM", "initEM");

		tt_init.tic(corpus_size);

		ptb->reset_iterator();

		// This is Matsuzaki MLE + noise initialization
		while (ptb->next_tree()) {
			cerr<<"1"<<endl;
			auto tree = ptb->current_penn_tree();
			cerr<<"2"<<endl;

			tt_init.update();

			vocab.modify_tree(tree);

			cerr<<"3"<<endl;
			if (tree->children_count() == 1) {
				continue;
			}

			//cerr<<tree->to_string()<<endl;

			auto bag = tree->create_bag(alphabet);
			cerr<<"4"<<endl;

			// scan binary rules
			bag->reset_binary_rule_iterator();

			while (bag->next_binary_rule()) {
				auto r = bag->current_binary_rule();
				get_m(r.a(), m);
				get_m(r.b(), m);
				get_m(r.c(), m);
				//current_grammar->inc_weight(r, get_m(r.a(), m), get_m(r.b(), m),
				//		get_m(r.c(), m), 1.0);
				tmp_grammar->inc_weight(bag->current_binary_rule(), 1, 1, 1,
						1.0);

				//auto r = bag->current_binary_rule();
				//cerr<<alphabet->nonterminal_to_string(r.a())<<" "<<alphabet->nonterminal_to_string(r.b())<<" "<<alphabet->nonterminal_to_string(r.c())<<endl;
			}

			// scan term rules
			bag->reset_term_rule_iterator();

			while (bag->next_term_rule()) {
				auto r = bag->current_term_rule();
				get_m(r.a(), m);
				//current_grammar->inc_weight(r, get_m(r.a(), m), 1.0);
				tmp_grammar->inc_weight(bag->current_term_rule(), 1, 1.0);

				//auto r = bag->current_term_rule();
				//cerr<<alphabet->nonterminal_to_string(r.a())<<" "<<alphabet->vocab_to_string(r.x())<<endl;
			}

			// root
			//current_grammar->inc_weight(bag->get_root(), get_m(bag->get_root(), m), 1.0);
			tmp_grammar->inc_weight(bag->get_root(), 1, 1.0);
			get_m(bag->get_root(), m);
			//cerr<<alphabet->nonterminal_to_string(bag->get_root());				

			tree->destroy_bag(bag);
		}

		cerr<<"Saving files"<<endl;

		ostringstream str;

		str<<grammar_file_prefix<<".vocab";

		vocab.save(str.str());

		real_sr = new real_semiring();

		tmp_grammar->normalize_grammar();

		current_grammar = tmp_grammar->create_latent_pcfg_grammar(tensor_sr, this);

		current_grammar->normalize_grammar();
		current_grammar->noise_matsuzaki();
		current_grammar->normalize_grammar();

		prune_grammar = tmp_grammar->create_double_grammar(real_sr);

		ostringstream str2;

		str2<<grammar_file_prefix<<".prune";

		tmp_grammar->save_grammar_file(str2.str());

		delete tmp_grammar;

		tt_init.toc();
	}

	void em_iter(penn_treebank* ptb, int m, string grammar_file_prefix, int iter, double scalingFactor) {

		latent_variable_cnf_grammar* next_iter_grammar =
				new latent_variable_cnf_grammar(tensor_sr, alphabet);


		tictoc tt_em("Running EM iteration", "EMiter");

		tt_em.tic(corpus_size);

		ptb->reset_iterator();

		double ll = 0.0;

		int count = 0;

		int skipped = 0;

		int tryAgain = 0;

		// tryAgain is a flag that tells whether the tree should be repeated, at most 15 times

		while (((tryAgain > 0) && (tryAgain < 15)) || ptb->next_tree()) {
			auto tree = ptb->current_penn_tree();

			if (tryAgain >= 15) { tryAgain = 0; }

			if (tryAgain == 0) {
				cerr<<"Tree: "<<tree->to_string()<<endl;
			}

			tt_em.update();

			if (tree->children_count() == 1) {
				cerr<<"Skipping tree because it has only one terminal."<<endl;
				continue;
			}

			vocab.modify_tree(tree);

			auto bag = tree->create_bag(alphabet);

			cnf_io_simple_tensor latent_io(tensor_sr);

			//cnf_io<my_double> pruner_io(real_sr);

			//cnf_simple_pruner pruner(&pruner_io, 1e-5);

			//latent_io.set_pruner(&pruner);

			vector<string> tokens = tree->get_terminal_yield();

			vocab_symbol* sent_ptr = new vocab_symbol[tokens.size()];

			for (auto i=0*tokens.size(); i < tokens.size(); i++) {
				sent_ptr[i] = alphabet->vocab_lookup_string(tokens[i]);
			}

			array_1d<vocab_symbol> sent(sent_ptr, tokens.size());

			//pruner_io.io_start_string(sent, prune_grammar);

			//pruner_io.io_constrain_bag(bag);

			latent_io.io_start_string(sent, current_grammar); //, &pruner, prune_grammar);

			latent_io.io_constrain_bag(bag);

			//latent_io.goal_string();

			//delete [] vocab_symbol;

			// Do root
			auto root_in = latent_io.inside(bag->get_root(), 0, tokens.size());

			auto root_out =
					latent_io.outside(bag->get_root(), 0, tokens.size());

			if (root_in.cols() > 0) {
				simple_tensor& v_ = next_iter_grammar->weight_ref(
						bag->get_root(), root_in.cols());

				double p = 0.0;

				for (int i=0; i < root_in.cols(); i++) {
					/*cerr<<"root_in:"<<root_in.get(i)<<endl;
					cerr<<"root_out:"<<root_out.get(i)<<endl;*/
					p += root_in.get(i) * root_out.get(i);
				}

				cerr<<"ll: "<<p<<endl;

				if ((p > 1e300) || (isnan(p))) {
					cerr<<"ll is too large or is nan. multiplying by 0.1"<<endl;
					tryAgain++;
					current_grammar->apply_underflow_prevention_factor(0.1);

				} else if (p > 0) {
					v_.plus_vector(root_in, root_out, p);

					ll += log(p);
					count++;

					// scan binary rules
					bag->reset_binary_rule_iterator();

					while (bag->next_binary_rule()) {
						auto r = bag->current_binary_rule();

						auto i = bag->current_binary_left_corner_state();

						auto j = bag->current_binary_right_corner_state();

						auto k = bag->mid_point(i, j);

						auto a_vec = latent_io.outside(r.a(), i, j);
						auto b_vec = latent_io.inside(r.b(), i, k);
						auto c_vec = latent_io.inside(r.c(), k, j);

						simple_tensor r_w = current_grammar->weight(r);

						simple_tensor& t_ = next_iter_grammar->weight_ref(r,
								r_w.slices(), r_w.rows(), r_w.cols());

						t_.plus_tensor(current_grammar->weight(r), a_vec,
								b_vec, c_vec, p);
					}

					// scan term rules
					bag->reset_term_rule_iterator();

					while (bag->next_term_rule()) {
						auto r = bag->current_term_rule();

						auto i = bag->current_term_left_corner_state();

						auto j = bag->current_term_right_corner_state();

						auto a_vec_out = latent_io.outside(r.a(), i, j);

						auto a_vec_in = latent_io.inside(r.a(), i, j);

						assert((j - i) == 1);

						if ((a_vec_out.cols () > 0) && (a_vec_in.cols() > 0)) {
							assert(a_vec_out.cols() == a_vec_in.cols());

							simple_tensor& v_ = next_iter_grammar->weight_ref(r,
									a_vec_in.cols());

							v_.plus_vector(a_vec_in, a_vec_out, p);
						}
					}

					tryAgain = 0;
				} else {
					current_grammar->apply_underflow_prevention_factor(10);
					tryAgain++;

					if (tryAgain == 1) {
						skipped++;
					}

					cerr<<"ll is 0. multiplying by 10"<<endl;
				}
			} else {
				skipped++;
			}

			tree->destroy_bag(bag);

			sent.destroy();
		}

		tt_em.toc();


		delete current_grammar;

		current_grammar = next_iter_grammar;

		current_grammar->normalize_grammar();

		cerr<<"Log-likelihood: "<< (ll / count) << " Skips: "<<skipped
				<<" Used: "<<count<<endl;

		ostringstream str;

		str<<grammar_file_prefix<<"."<<iter<<".gra";

		cerr<<"Saving grammar to "<<str.str()<<endl;

		current_grammar->save_grammar_file(str.str());

		current_grammar->apply_underflow_prevention_factor(scalingFactor);
	}

	void run_em(penn_treebank* ptb, int m, string vocabFile, int threshold, string grammar_prefix, double scalingFactor, int iterationsNumber) {

		if (vocabFile == string("@compute")) {

		cerr<<"Computing vocabulary"<<endl;

		vocab.set_threshold(threshold);

		tictoc tt_vocab("Computing vocabulary", "vocab");

		tt_vocab.tic();

		ptb->reset_iterator();

		corpus_size = 0;

		while (ptb->next_tree()) {
			auto tree = ptb->current_penn_tree();

			vocab.hit_tree(tree);

			corpus_size++;
		}

		tt_vocab.toc();
		} else {
			cerr<<"Reading vocabulary file "<<vocabFile<<endl;
			vocab.load(vocabFile);
                	vocab.set_threshold(threshold);
		}


		init(ptb, m, grammar_prefix);

		cerr<<"Saving init grammar"<<endl;

		ostringstream str2;

		str2<<grammar_prefix<<".init_matsuzaki";

		current_grammar->save_grammar_file(str2.str());

		for (int i=0; i < iterationsNumber; i++) {
			em_iter(ptb, m, grammar_prefix, i, scalingFactor);
		}
	}
};

#endif /*LPCFG_EM_H_*/
