#include "cnf_io.h"
#include "pruner.h"

template <class P> void cnf_io<P>::_create_io_tables(my_uint num_symbols,
		my_uint num_states) {
	/*debug::msg(string("creating io tables of size ")
	 + to_string(num_symbols) + string("x") + to_string(num_states)
	 + string("x") + to_string(num_states));*/

	is_lattice = false;

	on_demand = true;

	i_table = memutils::create_3d_array<P>(num_symbols, num_states,
			num_states, sr->zero());

	o_table = memutils::create_3d_array<P>(num_symbols, num_states,
			num_states, sr->zero());

	if (on_demand)
	{
		i_demand_table = memutils::create_3d_array<bool>(num_symbols, num_states,
				num_states, false);

		o_demand_table = memutils::create_3d_array<bool>(num_symbols, num_states,
				num_states, false);
	}

	array_1d<sym2double> arr = memutils::create_1d_array<sym2double>(num_states, sym2double());

	o_preterminal_table = new iterable_string_lattice<sym2double>(arr);

	_create_io_tables_further(num_symbols, num_states);
}

template <class P> void cnf_io<P>::io_start_string(
		array_1d<vocab_symbol> sentence, io_cnf_grammar* grammar,
		constituent_bag_interface<inpre_symbol, inpre_symbol>* pruner,
		cnf_grammar<inpre_symbol, vocab_symbol, my_double>* prune_grammar) {
	assert(grammar == NULL || sr->type() == grammar->semiring()->type());

	lattice = new iterable_string_lattice<vocab_symbol>(sentence);
	this->grammar = grammar;

	this->calc_goal = false;

	this->span_bag = NULL;

	_create_io_tables(current_unweighted_grammar()->alphabet()->nonterminal_count(), lattice->length()+1);

	if (pruner != NULL) {
		//pruner->io()->io_start_string(sentence, prune_grammar);
		set_pruner(pruner);
		pruner->setRightBranch(false, 0);

		this->rightBranch = false;

		//pruner->io()->io_do_inside();

		//pruner->io()->io_do_outside();
	} else {
		set_pruner(NULL);
		this->rightBranch = false;
	}
}

template <class P> void cnf_io<P>::io_start_lattice(
		string latticeString, bool multiplyOrAdd, double add, io_cnf_grammar* grammar,
		constituent_bag_interface<inpre_symbol, inpre_symbol>* pruner,
		cnf_grammar<inpre_symbol, vocab_symbol, my_double>* prune_grammar, bool rightBranch) {
	assert(grammar == NULL || sr->type() == grammar->semiring()->type());

	is_lattice = true;

	this->multiplyLatticeWeight = multiplyOrAdd;

	this->addVal = add;

	this->grammar = grammar;

	lattice = new FullLatticeString(current_unweighted_grammar()->alphabet(), latticeString, NULL, rightBranch);

	int num_states = lattice->length() + 1;
	int num_symbols = current_unweighted_grammar()->alphabet()->inpreterminal_count();

	this->calc_goal = false;

	this->span_bag = NULL;

	//_create_io_tables(current_unweighted_grammar()->alphabet()->nonterminal_count(), lattice->length()+1);

	on_demand = true;

	cerr<<"Creating lattice with "<<num_states<<" states and "<<num_symbols<<" nonterminals"<<endl;

	i_table = memutils::create_3d_array<P>(num_symbols, num_states, num_states, sr->zero());

	o_table = memutils::create_3d_array<P>(num_symbols, num_states, num_states, sr->zero());

	if (on_demand)
	{
		i_demand_table = memutils::create_3d_array<bool>(num_symbols, num_states, num_states, false);

		o_demand_table = memutils::create_3d_array<bool>(num_symbols, num_states, num_states, false);
	}

	o_preterminal_table = new FullLattice<sym2double>(num_states, latticeString, rightBranch);

	_create_io_tables_further(num_symbols, num_states);

	if (pruner != NULL) {
		pruner->setRightBranch(rightBranch, lattice->length());
		set_pruner(pruner);
	} else {
		set_pruner(NULL);
	}


	cerr<<"Setting right branch to "<<rightBranch<<endl;
	this->rightBranch = rightBranch;

	// TODO VERY IMPORTANT: No pruner is used if it is in lattice mode!!!! TODO TODO
	//set_pruner(NULL);
}

template<class P> void cnf_io<P>::set_pruner(constituent_bag_interface<inpre_symbol, inpre_symbol>* p) {
	this->pruner = p;
}

template<class P> bool cnf_io<P>::pruned(inpre_symbol a, my_uint s1, my_uint s2) {

	if (rightBranch) {
		if (!a.is_pre()) {
			if (s2 != lattice->length()) {
				return true;
			}
		}
	}

	if (pruner == NULL) {
		return false;
	}

	bool b;
	if (a.is_pre()) {
		b = !pruner->has_preterminal_constituent(a, s1, s2);
	} else {
		b = !pruner->has_nonterminal_constituent(a, s1, s2);
	}

	return b;
}

template <class P> void cnf_io<P>::_calc_inside_on_demand(inpre_symbol a_sym,
		my_uint i, my_uint j) {

	my_uint len = lattice->span_length(i, j);

	if (pruned(a_sym, i, j)) {
		return;
	}

	my_uint a = current_unweighted_grammar()->alphabet()->nonterminal_lookup(a_sym);

	// check whether len=1, in which case we need to use the preterminal rules
	if (len == 1) {
		// just skip length 1 ones, since we directly pull off below the length 1 ones from the
		// term rules a -> x, according to four cases (whether i,k and k,j have length larger than 1
		// or just 1.
		assert(false);
	} else {
		auto k_iter = lattice->reset_midstates_iterator(i, j,
				abstract_lattice<vocab_symbol>::midstate_iterator_no_end_inclusive);

		while (lattice->next_midstate(k_iter)) {
			my_uint k = lattice->mid_state(k_iter);

			if ((lattice->has_span_length_1(i, k))
					&& (lattice->has_span_length_1(k, j))) {

				// TODO make sure there is only one symbol between the two
				vocab_symbol x1 = lattice->symbol_at(i, k);
				vocab_symbol x2 = lattice->symbol_at(k, j);

				// get all preterminal rules that rewrite to x (a->x)
				auto& unary_rules1 = _rule_iter_preterminal(x1, i, k);

				// iterate over all these rules
				for (auto iter = unary_rules1.begin(); iter
						!= unary_rules1.end(); iter++) {

					io_cnf_term_rule unary_rule1 = (*iter);

					//auto v_b = grammar->weight(unary_rule1);
					auto v_b = inside_span_weight(unary_rule1, i, k);

					inpre_symbol b_sym = unary_rule1.a();

					auto& rule_set = binary_rules_with_ab(a_sym, b_sym, true,
							i, k, j);

					for (auto iter = rule_set.begin(); iter != rule_set.end(); iter++) {
						inpre_symbol c = (*iter).c();

						if (c.is_pre()) {
							//auto v_c = grammar->weight(cnf_term_rule<
							//		inpre_symbol, vocab_symbol>(c, x2));

							auto v_c = inside_span_weight(cnf_term_rule<
									inpre_symbol, vocab_symbol>(c, x2), k, j);

							if (!equals_zero(v_c)) {
								//auto v_abc = grammar->weight(*iter);

								auto v_abc = inside_span_weight(*iter, i, k, j);

								//cerr<<"v_abc = "<<v_abc<<endl;

								sr->plus_mul_yz(v_b, v_c, v_abc,
										i_table[a][i][j]);

								//sr->set_mul(v_b, v_c, v_abc, v_abc);

								//sr->set_plus(i_table[a][i][j], v_abc,
								//		i_table[a][i][j]);
							}
						}
					}
				}
			}

			if ((lattice->has_span_length_1(i, k))
					&& (lattice->has_span_length_1plus(k, j))) {


				// TODO make sure there is only one symbol between the two
				vocab_symbol x1 = lattice->symbol_at(i, k);

				// get all preterminal rules that rewrite to x (a->x)
				auto& unary_rules1 = _rule_iter_preterminal(x1, i, k);

				// iterate over all these rules
				for (auto iter = unary_rules1.begin(); iter
						!= unary_rules1.end(); iter++) {
					io_cnf_term_rule unary_rule1 = (*iter);

					//auto v_b = grammar->weight(unary_rule1);
					auto v_b = inside_span_weight(unary_rule1, i, k);

					if (!equals_zero(v_b)) {
						inpre_symbol b_sym = unary_rule1.a();

						auto& rule_set = binary_rules_with_ab(a_sym, b_sym,
								false, i, k, j);

						for (auto iter = rule_set.begin(); iter
								!= rule_set.end(); iter++) {
							inpre_symbol c = (*iter).c();

							//my_uint c_idx = grammar->alphabet()->nonterminal_lookup((*iter).c());

							if (!c.is_pre()) {
								auto v_c = _inside(c, k, j);

								if (!equals_zero(v_c)) {
									//auto v_abc = grammar->weight(*iter);
									auto v_abc = inside_span_weight(*iter, i,
											k, j);

									//sr->set_mul(v_b, v_c, v_abc, v_abc);

									//sr->set_plus(i_table[a][i][j], v_abc,
									//		i_table[a][i][j]);

									sr->plus_mul_yz(v_b, v_c, v_abc,
											i_table[a][i][j]);
								}
							}
						}
					}
				}
			}

			if ((lattice->has_span_length_1plus(i, k)) && (lattice->has_span_length_1(k, j))) {
				// TODO make sure there is only one symbol between the two

				vocab_symbol x2 = lattice->symbol_at(k, j);

				// get all preterminal rules that rewrite to x (a->x)
				auto& unary_rules2 = _rule_iter_preterminal(x2, k, j);

				// iterate over all these rules
				for (auto iter = unary_rules2.begin(); iter
						!= unary_rules2.end(); iter++) {
					io_cnf_term_rule unary_rule2 = (*iter);

					//auto v_c = grammar->weight(unary_rule2);
					auto v_c = inside_span_weight(unary_rule2, k, j);

					if (!equals_zero(v_c)) {

						inpre_symbol c_sym = unary_rule2.a();

						auto& rule_set = binary_rules_with_ac(a_sym, c_sym,
								false, i, k, j);

						for (auto iter = rule_set.begin(); iter
								!= rule_set.end(); iter++) {
							inpre_symbol b = (*iter).b();

							//my_uint b_idx = grammar->alphabet()->nonterminal_lookup((*iter).b());

							if (!b.is_pre()) {
								auto v_b = _inside(b, i, k);

								if (!equals_zero(v_b)) {
									//auto v_abc = grammar->weight(*iter);
									auto v_abc = inside_span_weight(*iter, i,
											k, j);

									//sr->set_mul(v_b, v_c, v_abc, v_abc);

									//sr->set_plus(i_table[a][i][j], v_abc,
									//		i_table[a][i][j]);

									sr->plus_mul_yz(v_b, v_c, v_abc,
											i_table[a][i][j]);
								}
							}
						}
					}
				}
			}

			if ((lattice->has_span_length_1plus(i, k)) && (lattice->has_span_length_1plus(k, j))) {

				auto& rule_set = binary_rules_with_nonterminals(a_sym, i, j);

				for (auto iter = rule_set.begin(); iter != rule_set.end(); iter++) {

					inpre_symbol b_sym = (*iter).b();

					auto v_b = _inside(b_sym, i, k);

					if (!equals_zero(v_b)) {

						//my_uint c = grammar->alphabet()->nonterminal_lookup((*iter).c());

						//auto v_c = i_table[c][k][j];
						auto v_c = _inside((*iter).c(), k, j);

						if (!equals_zero(v_c)) {

							if (!equals_zero(v_c)) {
								//auto v_abc = grammar->weight(*iter);
								auto v_abc = inside_span_weight(*iter, i, k, j);

								//sr->set_mul(v_b, v_c, v_abc, v_abc);

								//sr->set_plus(i_table[a][i][j], v_abc,
								//		i_table[a][i][j]);

								sr->plus_mul_yz(v_b, v_c, v_abc,
										i_table[a][i][j]);
							}
						}
					}
				}
			}
		}
	}
}

template <class P> void cnf_io<P>::_calc_inside(my_uint symbol, my_uint state1,
		my_uint state2) {

	lattice->reset_span_iterator(abstract_lattice<vocab_symbol>::iterator_forward);

	while (lattice->next_span()) {
		my_uint i = lattice->left_state();
		my_uint j = lattice->right_state();
		my_uint len = lattice->span_length(i, j);

		bool not_pruned = false;

		for (auto iter = current_unweighted_grammar()->alphabet()->nonterminals().begin(); iter != current_unweighted_grammar()->alphabet()->nonterminals().end(); iter++) {
			if (!pruned(*iter, i, j)) {
				not_pruned = true;
				break;
			}
		}

		if (not_pruned) {

			// check whether len=1, in which case we need to use the preterminal rules
			if (len == 1) {
				// just skip length 1 ones, since we directly pull off below the length 1 ones from the
				// term rules a -> x, according to four cases (whether i,k and k,j have length larger than 1
				// or just 1.
			} else {
				auto k_iter = lattice->reset_midstates_iterator(i, j,
						abstract_lattice<vocab_symbol>::midstate_iterator_no_end_inclusive);

				while (lattice->next_midstate(k_iter)) {
					my_uint k = lattice->mid_state(k_iter);

					if ((lattice->has_span_length_1(i, k))
							&& (lattice->has_span_length_1(k, j))) {
						// TODO make sure there is only one symbol between the two
						vocab_symbol x1 = lattice->symbol_at(i, k);
						vocab_symbol x2 = lattice->symbol_at(k, j);

						// get all preterminal rules that rewrite to x (a->x)
						auto& unary_rules1 = _rule_iter_preterminal(x1, i, k);

						// iterate over all these rules
						for (auto iter = unary_rules1.begin(); iter
								!= unary_rules1.end(); iter++) {
							io_cnf_term_rule unary_rule1 = (*iter);

							//auto v_b = grammar->weight(unary_rule1);
							auto v_b = inside_span_weight(unary_rule1, i, k);

							inpre_symbol b_sym = unary_rule1.a();

							auto& rule_set =
									binary_rules_with_b(b_sym, i, k, j);

							for (auto iter = rule_set.begin(); iter
									!= rule_set.end(); iter++) {
								inpre_symbol c = (*iter).c();
								inpre_symbol a_sym = (*iter).a();

								if (!pruned(a_sym, i, j)) {
									if (c.is_pre()) {
										//auto v_c = grammar->weight(cnf_term_rule<
										//		inpre_symbol, vocab_symbol>(c, x2));

										auto v_c = inside_span_weight(
												cnf_term_rule< inpre_symbol,
														vocab_symbol>(c, x2),
												k, j);

										if (!equals_zero(v_c)) {
											//auto v_abc = grammar->weight(*iter);

											auto v_abc = inside_span_weight(
													*iter, i, k, j);

											my_uint
													a =
															current_unweighted_grammar()->alphabet()->nonterminal_lookup(a_sym);

											sr->plus_mul_yz(v_b, v_c, v_abc,
													i_table[a][i][j]);

											//sr->set_mul(v_b, v_c, v_abc, v_abc);

											//sr->set_plus(i_table[a][i][j], v_abc,
											//		i_table[a][i][j]);
										}
									}
								}
							}
						}
					}

					if ((lattice->has_span_length_1(i, k))
							&& (lattice->has_span_length_1plus(k, j))) {
						// TODO make sure there is only one symbol between the two
						vocab_symbol x1 = lattice->symbol_at(i, k);

						// get all preterminal rules that rewrite to x (a->x)
						auto& unary_rules1 = _rule_iter_preterminal(x1, i, k);

						// iterate over all these rules
						for (auto iter = unary_rules1.begin(); iter
								!= unary_rules1.end(); iter++) {
							io_cnf_term_rule unary_rule1 = (*iter);

							//auto v_b = grammar->weight(unary_rule1);
							auto v_b = inside_span_weight(unary_rule1, i, k);

							if (!equals_zero(v_b)) {

								inpre_symbol b_sym = unary_rule1.a();

								auto& rule_set = binary_rules_with_b(b_sym, i,
										k, j);

								for (auto iter = rule_set.begin(); iter
										!= rule_set.end(); iter++) {
									inpre_symbol c = (*iter).c();
									inpre_symbol a_sym = (*iter).a();

									if (!pruned(a_sym, i, j)) {

										//my_uint c_idx = grammar->alphabet()->nonterminal_lookup((*iter).c());

										if (!c.is_pre()) {
											auto v_c = _inside(c, k, j);

											if (!equals_zero(v_c)) {
												//auto v_abc = grammar->weight(*iter);
												auto v_abc =
														inside_span_weight(
																*iter, i, k, j);

												//sr->set_mul(v_b, v_c, v_abc, v_abc);

												my_uint
														a =
																current_unweighted_grammar()->alphabet()->nonterminal_lookup(a_sym);

												//sr->set_plus(i_table[a][i][j], v_abc,
												//		i_table[a][i][j]);

												sr->plus_mul_yz(v_b, v_c,
														v_abc, i_table[a][i][j]);
											}
										}
									}
								}
							}
						}
					}

					if ((lattice->has_span_length_1plus(i, k))
							&& (lattice->has_span_length_1(k, j))) {
						// TODO make sure there is only one symbol between the two
						vocab_symbol x2 = lattice->symbol_at(k, j);

						// get all preterminal rules that rewrite to x (a->x)
						auto& unary_rules2 = _rule_iter_preterminal(x2, k, j);

						// iterate over all these rules
						for (auto iter = unary_rules2.begin(); iter
								!= unary_rules2.end(); iter++) {
							io_cnf_term_rule unary_rule2 = (*iter);

							//auto v_c = grammar->weight(unary_rule2);
							auto v_c = inside_span_weight(unary_rule2, k, j);

							if (!equals_zero(v_c)) {

								inpre_symbol c_sym = unary_rule2.a();

								auto& rule_set = binary_rules_with_c(c_sym, i,
										k, j);

								for (auto iter = rule_set.begin(); iter
										!= rule_set.end(); iter++) {
									inpre_symbol b = (*iter).b();
									inpre_symbol a_sym = (*iter).a();

									if (!pruned(a_sym, i, j)) {

										//my_uint b_idx = grammar->alphabet()->nonterminal_lookup((*iter).b());

										if (!b.is_pre()) {
											auto v_b = _inside(b, i, k);

											if (!equals_zero(v_b)) {
												//auto v_abc = grammar->weight(*iter);
												auto v_abc =
														inside_span_weight(
																*iter, i, k, j);

												//sr->set_mul(v_b, v_c, v_abc, v_abc);

												my_uint
														a =
																current_unweighted_grammar()->alphabet()->nonterminal_lookup(a_sym);

												//sr->set_plus(i_table[a][i][j], v_abc,
												//		i_table[a][i][j]);

												sr->plus_mul_yz(v_b, v_c,
														v_abc, i_table[a][i][j]);
											}
										}
									}
								}
							}
						}
					}

					if ((lattice->has_span_length_1plus(i, k))
							&& (lattice->has_span_length_1plus(k, j))) {

						for (my_uint b=0; b<i_table.slices(); b++) {
							inpre_symbol b_sym = current_unweighted_grammar()->alphabet()->reverse_nonterminal_lookup(b);

							auto v_b = _inside(b_sym, i, k);

							if (!equals_zero(v_b)) {
								auto& rule_set = binary_rules_with_b(b_sym, i,
										k, j);

								for (auto iter = rule_set.begin(); iter
										!= rule_set.end(); iter++) {
									//my_uint c = grammar->alphabet()->nonterminal_lookup((*iter).c());

									//auto v_c = i_table[c][k][j];
									auto v_c = _inside((*iter).c(), k, j);

									inpre_symbol a_sym = (*iter).a();

									if ((!pruned(a_sym, i, j))
											&& (!equals_zero(v_c))) {

										if (!equals_zero(v_c)) {
											//auto v_abc = grammar->weight(*iter);
											auto v_abc = inside_span_weight(
													*iter, i, k, j);

											//sr->set_mul(v_b, v_c, v_abc, v_abc);

											my_uint
													a =
															current_unweighted_grammar()->alphabet()->nonterminal_lookup(a_sym);

											//sr->set_plus(i_table[a][i][j], v_abc,
											//		i_table[a][i][j]);

											sr->plus_mul_yz(v_b, v_c, v_abc,
													i_table[a][i][j]);
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

template <class P> P cnf_io<P>::_inside(inpre_symbol a, my_uint i, my_uint j) {

	if (i == j) {
		return sr->one();
	}

	assert(i<j);

	if (lattice->has_span_length_1(i, j)) {
		if ((!a.is_pre()) || (pruned(a, i, j))) {
			return sr->zero();
		}

		vocab_symbol x = lattice->symbol_at(i, j);

		auto u = inside_span_weight(cnf_term_rule<inpre_symbol, vocab_symbol>(
				a, x), i, j);
		
		return u;
	} 
	if (lattice->has_span_length_1plus(i,j)) {
		if (a.is_pre()) {
			return sr->zero();
		}

		my_uint a_idx = current_unweighted_grammar()->alphabet()->nonterminal_lookup(a);

		if (on_demand && !i_demand_table[a_idx][i][j]) {
			_calc_inside_on_demand(a, i, j);
			i_demand_table[a_idx][i][j] = true;
		}

		return i_table[a_idx][i][j];
	}
}

template <class P> P cnf_io<P>::_outside(inpre_symbol a, my_uint i, my_uint j) {

	if (i == j) {
		return sr->one();
	}

	assert(i<j);

	if (lattice->has_span_length_1(i, j)) {

		sym2double& map = o_preterminal_table->symbol_at(i, j);

		if (map.find(a) == map.end()) {
			if (on_demand) {
				map[a] = sr->zero();

				_calc_outside_on_demand(a, i, j);

				return _outside(a, i, j);
			}

			return sr->zero();
		} else {
			return map[a];
		}
	}
	if (lattice->has_span_length_1plus(i,j)) {
		my_uint a_idx = current_unweighted_grammar()->alphabet()->nonterminal_lookup(a);

		if (on_demand && !o_demand_table[a_idx][i][j]) {
			_calc_outside_on_demand(a, i, j);
			o_demand_table[a_idx][i][j] = true;
		}

		return o_table[a_idx][i][j];
	}
}

template <class P> P cnf_io<P>::goal() {

	if (calc_goal) {
		return calculated_goal;
	} else {
		my_uint first = lattice->first_state();
		my_uint last = lattice->last_state();

		auto& roots_ = roots();

		calculated_goal = sr->zero();

		for (auto iter = roots_.begin(); iter != roots_.end(); iter++) {
			sr->set_plus(calculated_goal, sr->mul(_inside(*iter, first, last),
					_outside(*iter, first, last)), calculated_goal);
		}

		calc_goal = true;

		return calculated_goal;
	}
}

template <class P> P cnf_io<P>::inside_goal() {
	my_uint first = lattice->first_state();
	my_uint last = lattice->last_state();

	auto& roots_ = roots();

	P calculated_goal = sr->zero();

	for (auto iter = roots_.begin(); iter != roots_.end(); iter++) {
		sr->set_plus(calculated_goal, sr->mul(_inside(*iter, first, last), root_span_weight(*iter, first, last)),
				calculated_goal);
	}

	return calculated_goal;
}

template <class P> void cnf_io<P>::_calc_outside_on_demand(inpre_symbol my_sym,
		my_uint i, my_uint j) {

	my_uint len = lattice->span_length(i, j);

	if (pruned(my_sym, i, j)) {
		return;
	}

	// check whether length is full, in which case we need to use the preterminal rules
	if (lattice->is_root_span(i, j)) {
		// TODO set roots correctly
		if (len == 1) {
			assert(false);
		} else {
			auto a_idx = current_unweighted_grammar()->alphabet()->nonterminal_lookup(my_sym);

			//o_table[a_idx][lattice->first_state()][lattice->last_state()] = grammar->weight(my_sym);
			o_table[a_idx][lattice->first_state()][lattice->last_state()] = root_span_weight(my_sym, 0, len);
		}
	} else {
		// first term of outside
		auto k_iter = lattice->reset_midstates_iterator(lattice->first_state(),
				i, abstract_lattice<vocab_symbol>::midstate_iterator_first_inclusive, true);

		while (lattice->next_midstate(k_iter)) {
			my_uint k = lattice->mid_state(k_iter);

			// first, handle the case  in which we are doing outside for a preterminal
			// the only differences from length > 1 are: (a) we need to iterate over a preterminal_set
			// instead of nonterminal_set; (b) we need to update o_preterminal_table instead of o_table.
			if (lattice->has_span_length_1(i,j)) {
				inpre_symbol c_sym = my_sym;

				auto& rule_set = binary_rules_with_c(c_sym, k, i, j);

				for (auto iter = rule_set.begin(); iter != rule_set.end(); iter++) {

					/*
					if (lattice->has_span_length_1(k, i)) {
						if (iter->b().is_pre()) {
							continue;
						}
					}
					if (lattice->has_span_length_1plus(k, i)) {
						if (iter->b().is_pre()) {
							continue;
						}
					}*/

					auto v_b = _inside(iter->b(), k, i);

					if (!equals_zero(v_b)) {
						auto v_a = _outside(iter->a(), k, j);
						if (!equals_zero(v_a)) {
							auto v_abc = grammar->weight(*iter);

							/*sr->set_mul(v_a, v_b, v_abc, v_abc);

							 sr->set_plus(
							 o_preterminal_table->symbol_at(i,j)[c_sym], v_abc,
							 o_preterminal_table->symbol_at(i,j)[c_sym]);*/

							sr->plus_mul_xy(v_a, v_b, v_abc,
									o_preterminal_table->symbol_at(i,j)[c_sym]);
						}
					}
				}
			} 

			if (lattice->has_span_length_1plus(i,j)) {
				inpre_symbol c_sym = my_sym;

				my_uint c_idx = current_unweighted_grammar()->alphabet()->nonterminal_lookup(c_sym);

				auto& rule_set = binary_rules_with_c(c_sym, k, i, j);

				for (auto iter = rule_set.begin(); iter != rule_set.end(); iter++) {

					/*
					if (lattice->span_length(k, i) == 1) {
						if (!iter->b().is_pre()) {
							continue;
						}
					} else {
						if (iter->b().is_pre()) {
							continue;
						}
					}*/

					auto v_b = _inside(iter->b(), k, i);

					if (!equals_zero(v_b)) {
						auto v_a = _outside(iter->a(), k, j);
						if (!equals_zero(v_a)) {
							auto v_abc = grammar->weight(*iter);

							/*sr->set_mul(v_a, v_b, v_abc, v_abc);

							 sr->set_plus(o_table[c_idx][i][j],
							 v_abc, o_table[c_idx][i][j]);*/

							sr->plus_mul_xy(v_a, v_b, v_abc,
									o_table[c_idx][i][j]);
						}
					}
				}
			}
		}

		// second term of outside
		k_iter = lattice->reset_midstates_iterator(j, lattice->last_state(),
				abstract_lattice<vocab_symbol>::midstate_iterator_last_inclusive, true);

		while (lattice->next_midstate(k_iter)) {
			my_uint k = lattice->mid_state(k_iter);

			//if (len == 1) {
			if (lattice->has_span_length_1(i,j)) {
				inpre_symbol b_sym = my_sym;

				auto& rule_set = binary_rules_with_b(b_sym, i, j, k);

				for (auto iter = rule_set.begin(); iter != rule_set.end(); iter++) {

					/*
					if (lattice->span_length(j, k) == 1) {
						if (!iter->c().is_pre()) {
							continue;
						}
					} else {
						if (iter->c().is_pre()) {
							continue;
						}
					}*/

					auto v_c = _inside(iter->c(), j, k);

					if (!equals_zero(v_c)) {
						auto v_a = _outside(iter->a(), i, k);

						if (!equals_zero(v_a)) {
							auto v_abc = grammar->weight(*iter);

							/*sr->set_mul(v_a, v_c, v_abc, v_abc);

							 sr->set_plus(
							 o_preterminal_table->symbol_at(i,j)[b_sym], v_abc,
							 o_preterminal_table->symbol_at(i,j)[b_sym]);*/

							sr->plus_mul_xz(v_a, v_c, v_abc,
									o_preterminal_table->symbol_at(i,j)[b_sym]);
						}
					}
				}
			}

			if (lattice->has_span_length_1plus(i,j)) {
				inpre_symbol b_sym = my_sym;

				my_uint b_idx = current_unweighted_grammar()->alphabet()->nonterminal_lookup(b_sym);

				auto& rule_set = binary_rules_with_b(b_sym, i, j, k);

				for (auto iter = rule_set.begin(); iter != rule_set.end(); iter++) {

					/*
					if (lattice->span_length(j, k) == 1) {
						if (!iter->c().is_pre()) {
							continue;
						}
					} else {
						if (iter->c().is_pre()) {
							continue;
						}
					}*/

					auto v_c = _inside(iter->c(), j, k);

					if (!equals_zero(v_c)) {
						auto v_a = _outside(iter->a(), i, k);

						if (!equals_zero(v_a)) {
							auto v_abc = grammar->weight(*iter);

							/*sr->set_mul(v_a, v_c, v_abc, v_abc);

							 sr->set_plus(o_table[b_idx][i][j],
							 v_abc, o_table[b_idx][i][j]);*/

							sr->plus_mul_xz(v_a, v_c, v_abc,
									o_table[b_idx][i][j]);
						}
					}
				}
			}
		}
	}
}

template <class P> void cnf_io<P>::_calc_outside(my_uint symbol,
		my_uint state1, my_uint state2) {

	lattice->reset_span_iterator(abstract_lattice<vocab_symbol>::iterator_reverse);

	while (lattice->next_span()) {
		my_uint i = lattice->left_state();
		my_uint j = lattice->right_state();
		my_uint len = lattice->span_length(i, j);

		bool not_pruned = false;

		if (lattice->has_span_length_1(i,j)) {
			for (auto iter = current_unweighted_grammar()->alphabet()->nonterminals().begin(); iter != current_unweighted_grammar()->alphabet()->nonterminals().end(); iter++) {
				if (!pruned(*iter, i, j)) {
					not_pruned = true;
					break;
				}
			}
		}

		if (lattice->has_span_length_1plus(i,j)) {
			for (auto iter = current_unweighted_grammar()->alphabet()->preterminals().begin(); iter != current_unweighted_grammar()->alphabet()->nonterminals().end(); iter++) {
				if (!pruned(*iter, i, j)) {
					not_pruned = true;
					break;
				}
			}
		}

		if (true) {
			// check whether length is full, in which case we need to use the preterminal rules
			if (lattice->is_root_span(i, j)) {
				// TODO set roots correctly
				if (len == 1) {
					assert(false);
				} else {
					auto& root_set = roots();

					for (auto iter = root_set.begin(); iter != root_set.end(); iter++) {
						auto a_idx = current_unweighted_grammar()->alphabet()->nonterminal_lookup(*iter);

						//o_table[a_idx][lattice->first_state()][lattice->last_state()] = grammar->weight(*iter);
						o_table[a_idx][lattice->first_state()][lattice->last_state()] = root_span_weight(*iter, 0, len);
					}
				}
			} else {
				// first term of outside
				auto k_iter = lattice->reset_midstates_iterator(
						lattice->first_state(), i,
						abstract_lattice<vocab_symbol>::midstate_iterator_first_inclusive);

				while (lattice->next_midstate(k_iter)) {
					my_uint k = lattice->mid_state(k_iter);

					// first, handle the case  in which we are doing outside for a preterminal
					// the only differences from length > 1 are: (a) we need to iterate over a preterminal_set
					// instead of nonterminal_set; (b) we need to update o_preterminal_table instead of o_table.
					if (len == 1) {

						for (auto iter = current_unweighted_grammar()->alphabet()->preterminals().begin(); iter != current_unweighted_grammar()->alphabet()->preterminals().end(); iter++) {
							inpre_symbol c_sym = *iter;

							if (!pruned(c_sym, i, j)) {

								auto& rule_set = binary_rules_with_c(c_sym, i,
										k, j);

								for (auto iter = rule_set.begin(); iter
										!= rule_set.end(); iter++) {

									auto v_b = _inside(iter->b(), k, i);

									if (!equals_zero(v_b)) {
										auto v_a = _outside(iter->a(), k, j);
										if (!equals_zero(v_a)) {
											auto v_abc = grammar->weight(*iter);

											/*sr->set_mul(v_a, v_b, v_abc, v_abc);

											 sr->set_plus(
											 o_preterminal_table->symbol_at(i,j)[c_sym], v_abc,
											 o_preterminal_table->symbol_at(i,j)[c_sym]);*/

											sr->plus_mul_xy(
													v_a,
													v_b,
													v_abc,
													o_preterminal_table->symbol_at(i,j)[c_sym]);
										}
									}
								}
							}
						}
					} else {
						for (auto iter = current_unweighted_grammar()->alphabet()->nonterminals().begin(); iter != current_unweighted_grammar()->alphabet()->nonterminals().end(); iter++) {
							inpre_symbol c_sym = *iter;

							my_uint c_idx = current_unweighted_grammar()->alphabet()->nonterminal_lookup(c_sym);

							if (!pruned(c_sym, i, j)) {

								auto& rule_set = binary_rules_with_c(c_sym, i,
										k, j);

								for (auto iter = rule_set.begin(); iter
										!= rule_set.end(); iter++) {

									auto v_b = _inside(iter->b(), k, i);

									if (!equals_zero(v_b)) {
										auto v_a = _outside(iter->a(), k, j);
										if (!equals_zero(v_a)) {
											auto v_abc = grammar->weight(*iter);

											/*sr->set_mul(v_a, v_b, v_abc, v_abc);

											 sr->set_plus(o_table[c_idx][i][j],
											 v_abc, o_table[c_idx][i][j]);*/

											sr->plus_mul_xy(v_a, v_b, v_abc,
													o_table[c_idx][i][j]);
										}
									}
								}
							}
						}
					}
				}

				// second term of outside
				k_iter = lattice->reset_midstates_iterator(j,
						lattice->last_state(),
						abstract_lattice<vocab_symbol>::midstate_iterator_last_inclusive);

				while (lattice->next_midstate(k_iter)) {
					my_uint k = lattice->mid_state(k_iter);

					if (len == 1) {
						for (auto iter = current_unweighted_grammar()->alphabet()->preterminals().begin(); iter != current_unweighted_grammar()->alphabet()->preterminals().end(); iter++) {
							inpre_symbol b_sym = *iter;

							if (!pruned(b_sym, i, j)) {

								auto& rule_set = binary_rules_with_b(b_sym, i,
										k, j);

								for (auto iter = rule_set.begin(); iter
										!= rule_set.end(); iter++) {

									auto v_c = _inside(iter->c(), j, k);

									if (!equals_zero(v_c)) {
										auto v_a = _outside(iter->a(), i, k);

										if (!equals_zero(v_a)) {
											auto v_abc = grammar->weight(*iter);

											/*sr->set_mul(v_a, v_c, v_abc, v_abc);

											 sr->set_plus(
											 o_preterminal_table->symbol_at(i,j)[b_sym], v_abc,
											 o_preterminal_table->symbol_at(i,j)[b_sym]);*/

											sr->plus_mul_xz(
													v_a,
													v_c,
													v_abc,
													o_preterminal_table->symbol_at(i,j)[b_sym]);
										}
									}
								}
							}
						}
					} else {
						for (auto iter = current_unweighted_grammar()->alphabet()->nonterminals().begin(); iter != current_unweighted_grammar()->alphabet()->nonterminals().end(); iter++) {
							inpre_symbol b_sym = *iter;

							if (!pruned(b_sym, i, j)) {

								my_uint b_idx = current_unweighted_grammar()->alphabet()->nonterminal_lookup(b_sym);

								auto& rule_set = binary_rules_with_b(b_sym, i,
										k, j);

								for (auto iter = rule_set.begin(); iter
										!= rule_set.end(); iter++) {

									auto v_c = _inside(iter->c(), j, k);

									if (!equals_zero(v_c)) {
										auto v_a = _outside(iter->a(), i, k);

										if (!equals_zero(v_a)) {
											auto v_abc = grammar->weight(*iter);

											/*sr->set_mul(v_a, v_c, v_abc, v_abc);

											 sr->set_plus(o_table[b_idx][i][j],
											 v_abc, o_table[b_idx][i][j]);*/

											sr->plus_mul_xz(v_a, v_c, v_abc,
													o_table[b_idx][i][j]);
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

void cnf_io_simple_tensor::destroy_tables() {
	for (int i=0; i < i_table.rows(); i++)
	{
		for (int j=0; j < i_table.cols(); j++)
		{
			i_table[i][j].destroy();
		}
	}
	
	for (int i=0; i < o_table.rows(); i++)
	{
		for (int j=0; j < o_table.cols(); j++)
		{
			o_table[i][j].destroy();
		}
	}
	
	// need to destruct o_preterminal_table
	o_preterminal_table->reset_span_iterator(abstract_lattice<vocab_symbol>::iterator_forward);

	while (o_preterminal_table->next_span())
	{
		auto table = o_preterminal_table->symbol_at(o_preterminal_table->left_state(), o_preterminal_table->right_state());

		for (auto iter = table.begin(); iter != table.end(); iter++)
		{
			iter->second.destroy();
		}
	}
	
	//o_preterminal_table->destroy_all();
}


template class cnf_io<my_double>;

template class cnf_io<simple_tensor>;

template class cnf_io<cnf_double_backpointer_pair<inpre_symbol, vocab_symbol> >;
