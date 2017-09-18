#ifndef __TensorGrammar_H__
#define __TensorGrammar_H__

#include "hashes.h"

#include "grammar.h"

#include <tr1/unordered_set>
#include <tr1/unordered_map>

using namespace std;
using namespace std::tr1;

class TensorGrammar
{
	unordered_map<tuple<inpre_symbol, inpre_symbol, inpre_symbol>, double> tensorValues;

	unordered_map<string, my_uint> tensorSymbols;

	unordered_map<my_uint, string> tensorSymbolsReverse;

	my_uint lastIndex;

	latent_variable_cnf_grammar* currentGrammar;


public:
	TensorGrammar()
	{
		lastIndex = 0;
	}

	void translateBinaryRules(latent_variable_cnf_grammar& grammar)
	{
		auto& binaryRules = grammar.all_binary_rules();

		for (auto i = binaryRules.begin(); i != binaryRules.end(); i++)
		{
			auto& rules = (*i).second;

			for (auto j = rules.begin(); j != rules.end(); j++)
			{
				auto rule = *j;

				tuple<inpre_symbol, inpre_symbol, inpre_symbol> r(rule.a(), rule.b(), rule.c());

				tensorValues[r] = grammar.weight(rule).get(0,0,0);
			}
		}

		currentGrammar = &grammar;
       }

	my_uint getSymbolIndex(string symbolName)
	{
		if (tensorSymbols.find(symbolName) == tensorSymbols.end()) {
			tensorSymbols[symbolName] = lastIndex;
			tensorSymbolsReverse[lastIndex] = symbolName;

			lastIndex++;

			return lastIndex-1;
		} else {
			return tensorSymbols[symbolName];
		}
	}

	inpre_symbol getSymbol(my_uint idx)
	{
		string s = tensorSymbolsReverse[idx];

		return currentGrammar->alphabet()->nonterminal_lookup_string(s);
	}

	bool printBinaryRules(string filename, string symFilename)
	{
		ofstream myfile(filename.c_str());

                if (!myfile.is_open()) {
                        return false;
                }

		for (auto i = tensorValues.begin(); i != tensorValues.end(); i++)
		{
			auto& r = (*i).first;

			auto a = get<0>(r);
			auto b = get<1>(r);
			auto c = get<2>(r);

			auto a_i = getSymbolIndex(currentGrammar->alphabet()->nonterminal_to_string(a));
			auto b_i = getSymbolIndex(currentGrammar->alphabet()->nonterminal_to_string(b));
			auto c_i = getSymbolIndex(currentGrammar->alphabet()->nonterminal_to_string(c));

			a_i = a_i + 1;
			b_i = b_i + 1;
			c_i = c_i + 1;

			myfile<<a_i<<" "<<b_i<<" "<<c_i<<" "<<(*i).second<<endl;
		}

		myfile.close();

		return saveSymbols(symFilename);
	}

	bool loadBinaryRules(string filename, string symFilename)
	{
		loadSymbols(symFilename);

		ifstream myfile(filename.c_str());

		if (!myfile.is_open()) {
			return false;
		}

		string s;

                while (getline(myfile, s)) {
                        vector<string> out;

                        string_utils::split(out, s, " ");

			int a_idx = stoi(out[0])-1;
			int b_idx = stoi(out[1])-1;
			int c_idx = stoi(out[2])-1;

			double v = stod(out[3]);

			tuple<inpre_symbol, inpre_symbol, inpre_symbol> t(getSymbol(a_idx), getSymbol(b_idx), getSymbol(c_idx));

			if (tensorValues.find(t) != tensorValues.end()) {
				tensorValues[t] = v;
			}
		}

		myfile.close();

		return true;
	}

	bool saveGrammar(string filename)
	{
		ofstream myfile(filename.c_str());

                if (!myfile.is_open()) {
                        return false;
                }

		for (auto i = tensorValues.begin(); i != tensorValues.end(); i++)
		{
			auto& r = (*i).first;

			auto a = get<0>(r);
			auto b = get<1>(r);
			auto c = get<2>(r);

			auto rule = cnf_binary_rule<inpre_symbol>(a, b, c);

			auto& w = currentGrammar->weight_ref(rule, 1,1,1);

			w.set(0,0,0, (*i).second);
		}

		myfile.close();

		currentGrammar->save_grammar_file(filename);

		return true;
	}

	bool saveSymbols(string filename)
	{
		ofstream myfile(filename.c_str());

                if (!myfile.is_open()) {
                        return false;
                }

		for (auto i = tensorSymbols.begin(); i != tensorSymbols.end(); i++)
		{
			myfile<<(*i).first<<" "<<(*i).second<<endl;
		}

		myfile.close();

		return true;
	}

	bool loadSymbols(string filename)
	{
  		ifstream myfile(filename.c_str());

                if (!myfile.is_open()) {
			cerr<<"Could not load symbol file "<<filename<<endl;

                        return false;
                }

		string s;

                while (getline(myfile, s)) {
                        vector<string> out;

                        string_utils::split(out, s, " ");

			string token = out[0];
			my_uint idx = stoi(out[1]);

			tensorSymbols[token] = idx;
			tensorSymbolsReverse[idx] = token;
		}

		myfile.close();

		return true;
	}
};

#endif
