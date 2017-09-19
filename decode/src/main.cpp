#include <stdlib.h>
#include "lattice.h"
#include "grammar.h"
#include "semiring.h"
#include "cnf_io.h"
#include <iostream>
#include <string>
#include "trees.h"
#include <ctime>
#include "debug.h"
#include "environment.h"
#include "span_bag.h"
#include "treebanks.h"
#include "counter.h"
#include "lpcfg_em.h"
#include "lpcfg_parser.h"
#include "signature.h"
#include "syntactic_embeddings.h"
#include "tree_normalizer.h"
#include "marginals.h"

#include "table_variable.h"
#include "ParseCombinationViterbi.h"
#include "ParseCombinationChart.h"
#include "MultiTreebankCombinationParser.h"
#include "MultiTreebankChartCombinationParser.h"
#include "LPCFGSampler.h"

#include "PrintRawMarginals.h"

#include "AbstractHeadFinder.h"
#include "PennTreebankCollinsHeadFinder.h"

#include "TensorGrammar.h"

#include "GrammarCommands.h"

#include "ChartFilter.h"

#include "lpcfg_flatten.h"

#include "EditDistance.h"

#include "full_lattice.h"

#include "DepthPrinter.h"

//#include "pruner.h"

//#include "lang_tokenizer.h"

using namespace std;

template class FullLattice<cnf_io<my_double>::sym2double>;
template class FullLattice<cnf_io_simple_tensor::sym2double>;
template class FullLattice<cnf_io_viterbi::sym2double>;

int main(int argc, char** argv) {

	signature::state_signature(argc, argv);

	string command = signature::vars.getStringValue("command", "none", true,
			"No command specified.");

	if (string(command) == string("printdepth")) {
		DepthPrinter printDepth;

		penn_treebank tb;

		bool b = tb.open_file(
                                signature::vars.getStringValue("treebankInput", "none", true,
                                                "No input treebank (sentences) specified"));

		if (!b) {
			cerr << "Could not open treebank file. Exiting.";

			return 0;
		}


		printDepth.printDepth(&tb);
	}

	if (string(command) == string("editdistance")) {
		EditDistanceCalculator editDist;

		editDist.doEditDistance(signature::vars.getStringValue("tabSeparatedStringFile", "none", true,
                                                "No tab separated file specified"), signature::vars.getStringValue("dictionaryFile", "none", true,
                                                "No dictionary file specified"), signature::vars.getBooleanValue("useDictionaryFile", true, true,
                                                "Compare string against all dictionary file strings"), signature::vars.getBooleanValue("outputEditDistanceBacktrace", true, true,
                                                "Output the two aligned strings from the tab separated file"),
						signature::vars.getBooleanValue("doTransposition", true, true,
                                                "Do transposition"));
	}

	if (string(command) == string("lpcfgflatten")) {
		LPCFGFlattenGrammar grammarFlatter;
	
		grammarFlatter.flattenGrammar(signature::vars.getStringValue("grammarInput", "none", true,
                                                "No grammar file specified"));
	}

	if (string(command) == string("lpcfgem")) {
		lpcfg_em em;

		penn_treebank tb;

		bool b = tb.open_file(
				signature::vars.getStringValue("treebankInput", "none", true,
						"No input treebank (sentences) specified"));

		if (!b) {
			cerr << "Could not open treebank file. Exiting.";

			return 0;
		}

		em.run_em(&tb, signature::vars.getIntValue("m", 0, true,
                                                "Number of latent states"),  signature::vars.getStringValue("vocabFile", "vocab.txt", true, "No vocab file specified."), signature::vars.getIntValue("vocabThreshold", 0, true,
                                                "No vocabulary threshold specified"), signature::vars.getStringValue("grammarOutputPrefix", "gr", true, "No grammar output prefix specified."),
signature::vars.getDoubleValue("scalingFactor", 1.0, true, "No scaling factor specified."), signature::vars.getIntValue("iterationsNumber", 30, true, "Number of iterations for EM"));
	}

	if (string(command) == string("filterchart")) {
		ChartFilter filter;

		filter.filter(
				signature::vars.getStringValue("grammarFile", "none", true,
						"Grammar file to be loaded is missing"),
				signature::vars.getStringValue("chartFile", "none", true,
						"Chart file to be filtered is missing."),
				signature::vars.getStringValue("treebankFile", "none", true,
						"Treebank file to be used as a filter is missing."));
	}

	/*
	if (string(command) == string("lrtrain")) {
		BilinearFeatureLogisticRegression lr;

		lr.train(
				signature::vars.getStringValue("inputTrainFile", "none", true,
						"No input file for training data specified."),
				signature::vars.getStringValue("outputModelFile", "none", true,
						"No output file for saving model parameters specified."));
	}

	if (string(command) == string("lrpredict")) {
		BilinearFeatureLogisticRegression lr;

		lr.predict(
				signature::vars.getStringValue("inputTestFile", "none", true,
						"No input file for test data specified."),
				signature::vars.getStringValue("inputModelFile", "none", true,
						"No input file for model parameters specified."),
				signature::vars.getStringValue("outputPredictionFile", "none",
						true,
						"No output file for saving predictions specified."));
	}
	*/

	// Not needed for parser release, and uses eigen
	/*
	if (string(command) == string("hmmtrain")) {

		penn_treebank tb;

		tb.writeXYTreebank(
				signature::vars.getStringValue("corpusInput", "none", true,
						"No input treebank (sentences) specified"),
				signature::vars.getStringValue("tmpTreebankOutput", "none",
						true, "No temporary treebank specified"));

		bool b = tb.open_file(
				signature::vars.getStringValue("tmpTreebankOutput", "none",
						true, "No temporary treebank specified"));

		if (!b) {
			cerr << "Could not open treebank file. Exiting.";

			return 0;
		}

		OOMSimpleFeatureGenerator fg;
		HiddenMarkovModelOOM oomHMM(fg);

		oomHMM.train(
				signature::vars.getIntValue("rankX", 1, true,
						"No rankX specified (integer)"),
				signature::vars.getIntValue("rankY", 1, true,
						"No rankY specified (integer)"), &tb);

		oomHMM.save(
				signature::vars.getStringValue("outputModelFile", "none", true,
						"No output file for saving model parameters specified."));
		//oomHMM.printParameters();

		//oomHMM.read(&tb);
	}

	if (string(command) == string("hmmread")) {

		penn_treebank tb;

		tb.writeXYTreebank(
				signature::vars.getStringValue("corpusInput", "none", true,
						"No input treebank (sentences) specified"),
				signature::vars.getStringValue("tmpTreebankOutput", "none",
						true, "No temporary treebank specified"));

		bool b = tb.open_file(
				signature::vars.getStringValue("tmpTreebankOutput", "none",
						true, "No temporary treebank specified"));

		if (!b) {
			cerr << "Could not open treebank file. Exiting.";

			return 0;
		}

		OOMSimpleFeatureGenerator fg;
		HiddenMarkovModelOOM oomHMM(fg);

		oomHMM.load(
				signature::vars.getStringValue("inputModelFile", "none", true,
						"No input file for loading model parameters specified."));
		//oomHMM.printParameters();

		oomHMM.read(
				&tb,
				signature::vars.getStringValue("outputReadFile", "none", true,
						"No file specified to write out the reading output."));
	}*/

	if (string(command) == string("printmarginals")) {

		penn_treebank tb;

		bool b = tb.open_file(
				signature::vars.getStringValue("treebankInput", "none", true,
						"No input treebank (sentences) specified"));

		if (!b) {
			cerr << "Could not open treebank file. Exiting.";

			return 0;
		}

		penn_treebank tb2;
		penn_treebank *v_tb = NULL;

		string s = signature::vars.getStringValue("treebankInputForVocabModify", "none", true,
							"No input treebank (for vocab modify) specified. Can be 'null'.");

		if (s != "null") {

		b = tb2.open_file(
				signature::vars.getStringValue("treebankInputForVocabModify", "none", true,
						"No input treebank (for vocab modify) specified. Can be 'null'."));

		if (!b) {
			cerr << "Could not open modifyvocab treebank file. Exiting.";

			return 0;
		}

			v_tb = &tb2;
		} else {
			cerr<<"Not loading vocab modification treebank. Specified as null."<<endl;
		}


		PrintMarginals maxMarg;

		maxMarg.print_max_marginals(
				&tb, v_tb,
				signature::vars.getStringValue("grammar", "none", true,
						"No grammar file specified"),
				signature::vars.getStringValue("pruneGrammar", "none", true,
						"No prune grammar file specified"),
				signature::vars.getStringValue("vocabFile", "none", true,
						"No vocabulary file specified"),
				signature::vars.getIntValue("vocabThreshold", 0, true,
						"No vocabulary threshold specified"),
				signature::vars.getDoubleValue("pruneThreshold", 0, true,
						"No prune threshold specified"),
				signature::vars.getBooleanValue("constrainTree", true, true,
						"Did not specify whether to constrain tree"),
				signature::vars.getBooleanValue("enforcePreterminals", false,
						true, "Did not specify whether to enforce preterminals"),
				signature::vars.getBooleanValue("doParse", false,
                                                true, "Actually do the parsing"));
	}
	if (string(command) == string("maxmarginals")) {

		penn_treebank tb;

		bool b = tb.open_file(
				signature::vars.getStringValue("treebankInput", "none", true,
						"No input treebank (sentences) specified"));

		if (!b) {
			cerr << "Could not open treebank file. Exiting.";

			return 0;
		}

		MaxMarginalParser maxMarg;

		maxMarg.print_max_marginals(
				&tb,
				signature::vars.getStringValue("grammar", "none", true,
						"No grammar file specified"),
				signature::vars.getStringValue("pruneGrammar", "none", true,
						"No prune grammar file specified"),
				signature::vars.getStringValue("vocabFile", "none", true,
						"No vocabulary file specified"),
				signature::vars.getIntValue("vocabThreshold", 0, true,
						"No vocabulary threshold specified"),
				signature::vars.getDoubleValue("pruneThreshold", 0, true,
						"No prune threshold specified"),
				signature::vars.getBooleanValue("constrainTree", true, true,
						"Did not specify whether to constrain tree"),
				signature::vars.getBooleanValue("enforcePreterminals", false,
						true, "Did not specify whether to enforce preterminals"),
				signature::vars.getBooleanValue("doParse", false,
                                                true, "Actually do the parsing"));
	}

	if (string(command) == string("embeddings")) {

		penn_treebank tb;

		bool b = tb.open_file(
				signature::vars.getStringValue("treebankInput", "none", true,
						"No input treebank (sentences) specified"));

		if (!b) {
			cerr << "Could not open treebank file. Exiting.";

			return 0;
		}

		syntactic_embeddings_generator embedder;

		embedder.print_embeddings(
				&tb,
				signature::vars.getStringValue("grammar", "none", true,
						"No grammar file specified"),
				signature::vars.getStringValue("pruneGrammar", "none", true,
						"No prune grammar file specified"),
				signature::vars.getStringValue("vocabFile", "none", true,
						"No vocabulary file specified"),
				signature::vars.getIntValue("vocabThreshold", 0, true,
						"No vocabulary threshold specified"),
				signature::vars.getDoubleValue("pruneThreshold", 0, true,
						"No prune threshold specified"),
				signature::vars.getBooleanValue("constrainTree", true, true,
						"Did not specify whether to constrain tree"),
				signature::vars.getBooleanValue("enforcePreterminals", false,
						true, "Did not specify whether to enforce preterminals"));
	}

	if (string(command) == string("lpcfgsample")) {

		LPCFGSampler sampler;

		sampler.sample(
				signature::vars.getStringValue("grammar", "none", true,
						"No grammar file specified"),
				signature::vars.getIntValue("numsamples", 10, false,
						"Number of samples not specified"), false);
	}
	if (string(command) == string("recombparse")) {

		auto numTreebanks = signature::vars.getIntValue("treebankCount", 0,
				true, "No treebank count specified");

		vector<string> filenames;

		MultiTreebankCombinationParser parser;

		for (auto i = 1 + 0 * numTreebanks; i <= numTreebanks; i++) {
			ostringstream my_str;

			my_str << "treebank" << i;

			filenames.push_back(
					signature::vars.getStringValue(my_str.str(), "none", true,
							"No treebank count specified"));
		}

		parser .parse(
				signature::vars.getStringValue("grammar", "none", true,
						"No grammar file specified"),
				filenames,
				signature::vars.getStringValue("vocabFile", "none", true,
						"No vocabulary file specified."),
				signature::vars.getIntValue("vocabThreshold", 0, true,
						"No vocabulary threshold specified"));
	}

	if (string(command) == string("sumrecombparse")) {

		auto numTreebanks = signature::vars.getIntValue("treebankCount", 0,
				true, "No treebank count specified");

		vector<string> filenames;

		MultiTreebankChartCombinationParser parser;

		for (auto i = 1 + 0 * numTreebanks; i <= numTreebanks; i++) {
			ostringstream my_str;

			my_str << "treebank" << i;

			filenames.push_back(
					signature::vars.getStringValue(my_str.str(), "none", true,
							"Charts not specified for all indices"));
		}

		parser.parse(
				signature::vars.getStringValue("grammar", "none", true,
						"No grammar file specified"),
				filenames,
				signature::vars.getStringValue("treebankInput", "none", true,
						"No treebank specified"),
				signature::vars.getStringValue("vocabFile", "none", true,
						"No vocabulary file specified"),
				signature::vars.getIntValue("vocabThreshold", 0, true,
						"No vocabulary threshold specified"),
				signature::vars.getBooleanValue("doPlus", false, true,
						"Did not specify whether to do plus or times"));
	}

	if (string(command) == string("lpcfgparselattice")) {
		lpcfg_parser parser;

		parser.parseLattice(
				signature::vars.getStringValue("latticeInput", "none", true,
                                                "No input lattice (sentences) specified"),
				signature::vars.getStringValue("grammar", "none", true,
						"No grammar file specified"),
				signature::vars.getStringValue("pruneGrammar", "none", true,
						"No prune grammar file specified"),
				signature::vars.getStringValue("vocabFile", "none", true,
						"No vocabulary file specified"),
				signature::vars.getIntValue("vocabThreshold", 0, true,
						"No vocabulary threshold specified"),
				signature::vars.getBooleanValue("enforcePreterminals", false,
						true, "Did not specify whether to enforce preterminals"),
				signature::vars.getBooleanValue("printChart", false, true,
						"Did not specify whether to print chart or not"),
				signature::vars.getBooleanValue("multiplyWeights", false, true, "Did not specify how to combine lattice weights"),
				signature::vars.getDoubleValue("addWeight", false, true, "Did not specify the add factor"),
				signature::vars.getBooleanValue("rightBranch", false, true, "Is it right branching trees?"));
	}

	if (string(command) == string("lpcfgparse")) {
		lpcfg_parser parser;

		penn_treebank tb;

		bool b = tb.open_file(
				signature::vars.getStringValue("treebankInput", "none", true,
						"No input treebank (sentences) specified"));

		if (!b) {
			cerr << "Could not open treebank file. Exiting.";

			return 0;
		}

		parser.parse(
				&tb,
				signature::vars.getStringValue("grammar", "none", true,
						"No grammar file specified"),
				signature::vars.getStringValue("pruneGrammar", "none", true,
						"No prune grammar file specified"),
				signature::vars.getStringValue("vocabFile", "none", true,
						"No vocabulary file specified"),
				signature::vars.getIntValue("vocabThreshold", 0, true,
						"No vocabulary threshold specified"),
				signature::vars.getBooleanValue("enforcePreterminals", false,
						true, "Did not specify whether to enforce preterminals"),
				signature::vars.getBooleanValue("printChart", false, true,
						"Did not specify whether to print chart or not"),
				signature::vars.getIntValue("maxLatentState", 10000, false,
                                                "Maximal m"));
	}

	if (string(command) == string("lpcfgcommands")) {

		GrammarCommandExecuter exec;

		if (signature::vars.getBooleanValue("outputTensor", false, true,
				"Did not specify whether to output tensor")) {
			exec.outputTensor(
					signature::vars.getStringValue("grammarFile", "none", true,
							"No grammar file specified"),
					signature::vars.getStringValue("outputTensorFile", "none",
							true, "No output specified for tensor file"),
					signature::vars.getStringValue("outputSymbolFile", "none",
							true, "No output specified for symbol file"));
		}

		if (signature::vars.getBooleanValue("updateGrammar", false, true,
				"Did not specify whether to output tensor")) {
			exec.updateGrammar(
					signature::vars.getStringValue("grammarFile", "none", true,
							"No grammar file specified"),
					signature::vars.getStringValue("inputTensorFile", "none",
							true, "No input specified for tensor file"),
					signature::vars.getStringValue("inputSymbolFile", "none",
							true, "No input specified for symbol file"),
					signature::vars.getStringValue("outputGrammarFile", "none",
							true, "No output file specified for grammar"));
		}
	}

	if (string(command) == string("normalize")) {

		penn_treebank tb;

		bool b = tb.open_file(
				signature::vars.getStringValue("treebankInput", "none", true,
						"No input treebank (sentences) specified"));

		if (!b) {
			cerr << "Could not read input treebank" << endl;
			exit(0);
		}

		tb.reset_iterator();

		int count = 0;

		while (tb.next_tree()) {
			count++;

			auto tree = tb.current_penn_tree();

			StringTreeNormalizer bin;

			if (signature::vars.getBooleanValue("removeTraces", true, true,
					"Did not specify the removeTraces flag as true/false")) {
				auto tree2 = (pennbank_tree<string>*) bin.removeTraces(tree);

				//delete tree;

				tree = tree2;
			}

			if (signature::vars.getBooleanValue("collapseUnary", true, true,
					"Did not specify the unary collapse flag as true/false")) {
				auto tree2 = (pennbank_tree<string>*) bin.collapseUnaryRules(
						tree);

				//delete tree;

				tree = tree2;
			}

			if (signature::vars.getBooleanValue("binarize", true, true,
					"Did not specify the binarize flag as true/false")) {
				auto tree2 = (pennbank_tree<string>*) bin.binarizeTree(tree);

				//delete tree;

				tree = tree2;
			}

			//delete tree

			cout << tree->to_string() << endl;
		}

		cerr << "Normalized " << count << " trees" << endl;
	}

	/*
	 if (string(cname) == string("train_perceptron")) {
	 if (iflag == 0) {
	 cerr<<"[train_perceptron] Missing -i option"<<endl;
	 return 0;
	 }

	 if (gflag == 0) {
	 cerr<<"[train_perceptron] Missing -g option"<<endl;
	 return 0;
	 }

	 if (pflag == 0) {
	 cerr<<"[train_perceptron] Missing -p option"<<endl;
	 return 0;
	 }

	 if (xflag == 0) {
	 cerr<<"[train_perceptron] Missing -x option"<<endl;
	 return 0;
	 }

	 if (rflag == 0) {
	 //		cerr<<"[train_perceptron] Missing -r option"<<endl;
	 //		return 0;
	 }

	 if (lflag == 0) {
	 cerr<<"[train_perceptron] Missing -l option"<<endl;
	 return 0;
	 }

	 if (vflag == 0) {
	 cerr<<"[train_perceptron] Missing -v option"<<endl;
	 return 0;
	 }

	 if (oflag == 0) {
	 cerr<<"[train_perceptron] Missing -o option"<<endl;
	 return 0;
	 }

	 if (tflag == 0) {
	 cerr<<"[train_perceptron] Missing -t option"<<endl;
	 return 0;
	 }

	 if (vflag == 0) {
	 cerr<<"[train_perceptron] Missing -v option"<<endl;
	 return 0;
	 }

	 if (hflag == 0) {
	 cerr<<"[train_perceptron] Missing -h option"<<endl;
	 return 0;
	 }

	 if (Dflag == 0) {
	 cerr<<"[train_perceptron] Missing -D option"<<endl;
	 return 0;
	 }

	 if (mflag == 0) {
	 cerr<<"[train_perceptron] Missing -m option"<<endl;
	 return 0;
	 }

	 if (aflag == 0) {
	 cerr<<"[train_perceptron] Missing -a option"<<endl;
	 return 0;
	 }

	 penn_treebank tb;

	 bool b = tb.open_file(iname);

	 if (!b) {
	 cerr<<"[train_perceptron] Could not open "<<iname<<endl;
	 }

	 cnf_rbf_features rbf;

	 rbf.perceptron(&tb, gname, pname, xname, stoi((string)lname), stoi((string)vname), hname, oname, stod((string)tname), stoi((string)Dname), false, stod(mname), stod(aname), (sflag == 1), rname, (eflag == 1), bname, yflag);
	 }



	 if (string(cname) == string("parse_lpcfg")) {
	 if (iflag == 0) {
	 cerr<<"[parse_perceptron] Missing -i option"<<endl;
	 return 0;
	 }

	 if (gflag == 0) {
	 cerr<<"[parse_perceptron] Missing -g option"<<endl;
	 return 0;
	 }

	 if (pflag == 0) {
	 cerr<<"[parse_perceptron] Missing -p option"<<endl;
	 return 0;
	 }

	 if (vflag == 0) {
	 cerr<<"[parse_perceptron] Missing -v option"<<endl;
	 return 0;
	 }

	 if (xflag == 0) {
	 cerr<<"[parse_perceptron] Missing -x option"<<endl;
	 return 0;
	 }

	 lpcfg_parser parser;

	 penn_treebank tb;

	 bool b = tb.open_file(iname);

	 parser.parse(&tb, gname, pname, xname, stod((string)vname));
	 }

	 if (string(cname) == string("parse_perceptron")) {
	 if (iflag == 0) {
	 cerr<<"[parse_perceptron] Missing -i option"<<endl;
	 return 0;
	 }

	 if (gflag == 0) {
	 cerr<<"[parse_perceptron] Missing -g option"<<endl;
	 return 0;
	 }

	 if (pflag == 0) {
	 cerr<<"[parse_perceptron] Missing -p option"<<endl;
	 return 0;
	 }

	 if (xflag == 0) {
	 cerr<<"[parse_perceptron] Missing -x option"<<endl;
	 return 0;
	 }

	 if (vflag == 0) {
	 cerr<<"[parse_perceptron] Missing -v option"<<endl;
	 return 0;
	 }

	 if (oflag == 0) {
	 cerr<<"[parse_perceptron] Missing -o option"<<endl;
	 return 0;
	 }

	 if (tflag == 0) {
	 cerr<<"[parse_perceptron] Missing -t option"<<endl;
	 return 0;
	 }

	 if (mflag == 0) {
	 cerr<<"[parse_perceptron] Missing -m option"<<endl;
	 return 0;
	 }

	 if (aflag == 0) {
	 cerr<<"[parse_perceptron] Missing -a option"<<endl;
	 return 0;
	 }


	 penn_treebank tb;

	 bool b = tb.open_file(iname);

	 if (!b) {
	 cerr<<"[parse_perceptron] Could not open "<<iname<<endl;
	 }

	 cnf_rbf_features rbf;

	 rbf.parse(&tb, gname, pname, xname, mname, stoi((string)vname), stod((string)tname), stod((string)aname), (sflag == 1), (eflag == 1));
	 }
	 */

	/*if (argc > 3) {
	 lpcfg_parser parser;

	 parser.parse(&tb, argv[2], argv[3], argv[4], 25);
	 } else {
	 lpcfg_em em;

	 em.run_em(&tb, 24, 25, argv[2]);
	 }*/
	/*	semiring_environment sr_env;
	 
	 environment env(&sr_env);
	 
	 cnf_weighted_forest cnf(&env);
	 
	 cnf.load_from_file("/Users/scohen/GRAMMAR_MLE.TXT");
	 */
	/*pennbank_tree<string>
	 * tree =
	 pennbank_tree<string>::from_string("(X (T t) (Z y)  )");

	 cerr<<((pennbank_tree<string>*)tree->get_child(0))->to_string()<<endl;
	 
	 cnf_grammar_alphabet* a = new cnf_grammar_alphabet();
	 
	 cerr<<"Creating span bag"<<endl;

	 tree->create_bag(a);

	 cerr<<"Done creating bag"<<endl;
	 
	 cerr<<tree->to_string()<<endl;

	 vector<string> s = tree->get_preterminal_yield();
	 
	 for (int i=0; i < s.size(); i++)
	 {
	 cerr<<s[i]<<endl;
	 }
	 
	 parser_interface interface;

	 vector<string> strings;
	 
	 string line;
	 
	 ifstream myfile(argv[1]);

	 if (myfile.is_open()) {
	 line = "";
	 
	 while (getline(myfile, line) ) {
	 strings.push_back(line);
	 }

	 myfile.close();
	 } else
	 debug::msg("could not read grammar file");

	 vector<string> strings2;
	 
	 string line2;
	 
	 ifstream myfile2(argv[2]);

	 if (myfile2.is_open()) {
	 line2 = "";
	 
	 while (getline(myfile2, line2) ) {
	 strings2.push_back(line2);
	 }

	 myfile2.close();
	 } else
	 debug::msg("could not read grammar file");
	 */
	/*strings.clear();
	 strings.push_back("The man anticipated the food");*/

	//interface.parse_sentence(strings, strings2, argv[2], argv[3], 1e-4);
	//interface.parse_sentence(strings, strings, "/Users/scohen/GRAMMAR_MLE.TXT", "/Users/scohen/GRAMMAR_MLE.TXT", 0);
}

// Need to clean up code... Pruning with larger numbers. Interface for parsing. Memory release.

/*
 * 
 #define ONE 1.0

 #define _P(x) alphabet.preterminal_lookup_string(x)
 #define _V(x) alphabet.vocab_lookup_string(x)
 #define _N(x) alphabet.nonterminal_lookup_string(x)
 //#define _R(x, y) cnf_double_backpointer_pair<inpre_symbol, vocab_symbol>(y, &x)
 //#define _Rb(x, y) cnf_double_backpointer_pair<inpre_symbol, vocab_symbol>(y, &x)
 #define _R(x, y) simple_tensor(ONE, false)
 #define _Rb(x, y) simple_tensor(ONE, true)

 arbitrary_cfg_rule<vocab_symbol> binary_rule(vocab_symbol a, vocab_symbol b,
 vocab_symbol c) {
 vocab_symbol* p = new vocab_symbol[2];

 p[0] = b;
 p[1] = c;

 return arbitrary_cfg_rule<vocab_symbol>(a, array_1d<vocab_symbol>(p, 2));
 }

 arbitrary_cfg_rule<vocab_symbol> term_rule(vocab_symbol a, vocab_symbol x) {
 vocab_symbol* p = new vocab_symbol[1];

 p[0] = x;

 return arbitrary_cfg_rule<vocab_symbol>(a, array_1d<vocab_symbol>(p, 1));
 }
 */
