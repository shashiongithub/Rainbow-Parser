/*
 * LPCFGSampler.h
 *
 *  Created on: Apr 16, 2015
 *      Author: scohen
 */

#ifndef LPCFGSAMPLER_H_
#define LPCFGSAMPLER_H_

class LPCFGSampler {
public:
	latent_variable_cnf_grammar* grammar;

	tensor_semiring tensor_sr;

	real_semiring real_sr;

	cnf_grammar_alphabet* alphabet;

	LPCFGSampler() {
		grammar = NULL;
		alphabet = NULL;
	}

	void sample(string grammar_file, my_uint num_samples,
			bool output_latent_states) {

		assert(grammar == NULL);

		assert(alphabet == NULL);

		alphabet = new cnf_grammar_alphabet();
		grammar = new latent_variable_cnf_grammar(&tensor_sr, alphabet);

		grammar->parse_grammar_file(grammar_file);

		cerr<<"Preparing grammar sampling."<<endl;
		grammar->prepareSamplingSlices();

		for (my_uint i = 0; i < num_samples; i++) {
			my_uint h;

			inpre_symbol a = grammar->sampleRoot(h);

			auto my_string = sampleHelper(a, h, output_latent_states);

			cerr << "Sampling tree #" << i << endl;
			cout << my_string << endl;

			cerr << "Output: " << my_string << endl;
		}

		delete grammar;
		delete alphabet;
	}

	string sampleHelper(inpre_symbol a, my_uint h, bool print_states) {

		if (a.is_pre()) {
			auto term = grammar->sampleTermRule(a, h);

			ostringstream my_str;

			my_str << " (" << alphabet->nonterminal_to_string(a) << " "
					<< alphabet->vocab_to_string(term.x()) << ") ";

			return my_str.str();
		} else {
			my_uint h_2, h_3;

			auto binary = grammar->sampleBinaryRule(a, h, h_2, h_3);

			ostringstream my_str;

			my_str << " (" << alphabet->nonterminal_to_string(a) << " "
					<< sampleHelper(binary.b(), h_2, print_states) << " " << sampleHelper(
					binary.c(), h_3, print_states) << ") ";

			return my_str.str();

		}
	}
};

#endif /* LPCFGSAMPLER_H_ */
