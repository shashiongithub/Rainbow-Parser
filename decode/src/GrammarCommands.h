#ifndef GRAMMAR_COMMANDS_H
#define GRAMMAR_COMMANDS_H

#define FLIP_SIGN_TRUE true

class GrammarCommandExecuter {

public:

	void outputTensor(string grammar_file, string outputTensorFile, string outputSymbolFile) {

		cnf_grammar_alphabet alphabet;

		tensor_semiring tensor_sr;

		real_semiring real_sr;

		latent_variable_cnf_grammar grammar(&tensor_sr, &alphabet);

		grammar.parse_grammar_file(grammar_file);

		TensorGrammar tensorGrammar;

		tensorGrammar.translateBinaryRules(grammar);

		tensorGrammar.printBinaryRules(outputTensorFile, outputSymbolFile);
	}

	void updateGrammar(string grammar_file, string inputTensorFile, string inputSymbolFile, string outputGrammarFile) {

		cnf_grammar_alphabet alphabet;

		tensor_semiring tensor_sr;

		real_semiring real_sr;

		latent_variable_cnf_grammar grammar(&tensor_sr, &alphabet);

		grammar.parse_grammar_file(grammar_file);

		TensorGrammar tensorGrammar;

		tensorGrammar.translateBinaryRules(grammar);

		tensorGrammar.loadBinaryRules(inputTensorFile, inputSymbolFile);

		tensorGrammar.saveGrammar(outputGrammarFile);
	}
};

#endif
