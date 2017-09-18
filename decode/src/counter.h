#ifndef COUNTER_H_
#define COUNTER_H_

template <class L> class counter {
public:
	unordered_map<L, my_uint> count_table;

public:
	my_uint count(L label) {
		if (count_table.find(label) == count_table.end()) {
			return 0;
		} else {
			return count_table[label];
		}
	}

	void inc_count(L label, int c = 1) {
		count_table[label] = count(label) + c;
	}
};

class vocab_reducer {
protected:
	counter<string> vocab_table;
	int threshold;

public:

	vocab_reducer()
	{
		threshold = 0;
	}
	
	bool load(string filename, int threshold = 0) {
		ifstream myfile(filename.c_str());

		if (myfile.is_open()) {

			string line;

			while (getline(myfile, line)) {
				vector<string> tokens;

				string_utils::split(tokens, line, " ");

				assert(tokens.size() == 2);

				int count = stoi(tokens[1]);

				assert(count >= 0);

				if (count >= threshold) {
					vocab_table.inc_count(tokens[0], count);
				}
			}

			myfile.close();

			return true;
		} else {
			return false;
		}
	}
	
	bool save(string filename)
	{
		ofstream myfile(filename.c_str());
		
		if (myfile.is_open())
		{
			for (auto iter = vocab_table.count_table.begin(); iter != vocab_table.count_table.end(); iter++)
			{
				myfile<<iter->first<<" "<<iter->second<<endl;
			}
			
			myfile.close();
			
			return true;			
		} else {
			return false;
		}
	}

	void set_threshold(int t) {
		threshold = t;
	}

	string reduce_token(string token, string alt) {
		if (vocab_table.count(token) > threshold) {
			return token;
		} else {
			return alt;
		}
	}
};

class cnf_vocab_reducer : public vocab_reducer
{
public:
	void hit_tree(pennbank_tree<string>* tree)
	{
		vector<string> words;
		
		words = tree->get_terminal_yield();

		for (auto i=0*words.size(); i < words.size(); i++)
		{
			vocab_table.inc_count(words[i]);
		}
	}
	
	void modify_tree(pennbank_tree<string>* tree)
	{
		vector<string*> words;
		vector<string> pre;
		
		words = tree->get_terminal_yield_ref();
		
		pre = tree->get_preterminal_yield();

		for (auto i=0*words.size(); i < words.size(); i++)
		{
			vector<string> v;
			
			// TODO may need a different split
			string_utils::split(v, pre[i], "|");

			*(words[i]) = reduce_token(*(words[i]), v[v.size()-1]);
		}		
	}

	void modifyTreeFromTree(pennbank_tree<string>* tree, pennbank_tree<string>* second)
	{
		vector<string*> words;
		vector<string> pre;
		
		words = tree->get_terminal_yield_ref();
		
		pre = second->get_terminal_yield();

		for (auto i=0*words.size(); i < words.size(); i++)
		{
			vector<string> v;
			
			// TODO may need a different split
			if (second != NULL) {
				*(words[i]) = reduce_token(*(words[i]), pre[i]);
			} else {
				string_utils::split(v, pre[i], "|");
				*(words[i]) = reduce_token(*(words[i]), v[v.size()-1]);
			}

		}		
	}
};

#endif /*COUNTER_H_*/
