#ifndef LPCFG_FLAT_H_
#define LPCFG_FLAT_H_

#define FLIP_SIGN_TRUE true

class LPCFGFlattenGrammar {

public:

	void flattenGrammar(string grammarFile) {

		cnf_grammar_alphabet alphabet;

		tensor_semiring tensor_sr;

		real_semiring real_sr;

		latent_variable_cnf_grammar grammar(&tensor_sr, &alphabet);

		grammar.parse_grammar_file(grammarFile);

		ostringstream str;

		str<<grammarFile<<".flat";

		string grammarFileNew = str.str();

		grammar.save_grammar_file_flat(grammarFileNew);
	}
};

#endif /*LPCFG_FLAT_H_*/
