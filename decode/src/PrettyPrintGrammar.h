#ifndef __pretty_print_grammar_h__
#define __pretty_print_grammar_h__

class GrammarBrowserCreator
{
public:
	vector<string> sentences;
	unordered_map<string, unordered_map< int, vector< tuple< int, int, int> > > > inMapHier;
	unordered_map<string, unordered_map< int, vector< tuple< int, int, int> > > > preMapHier;

	unordered_map<string, vector< tuple< int, int, int> > > inMap;
	unordered_map<string, vector< tuple< int, int, int> > > preMap;

	virtual int hitSentence(string sentence)
	{
		sentences.push_back(sentence);

		return sentences.size()-1;
	}

	virtual int getSentenceCount()
	{
		return sentences.size();
	}

	virtual string getSentence(int num)
	{
		assert(num < sentences.size());

		return sentences[num];
	}

	virtual void hit(string nonterminal, bool is_pre, int latent_state, int sentence, int left, int right)
	{
	}

	virtual void prettyPrintSortByStateCount(string file)
	{
	}

	virtual void prettyPrintSortByNonterminalCount(string file)
	{
	}
}

#endif
