#ifndef DEPTH_PRINTER_H_
#define DEPTH_PRINTER_H_

class DepthPrinter {
private:
	cnf_vocab_reducer vocab;

public:
	void printDepth(penn_treebank* ptb) {

		ptb->reset_iterator();

		while (ptb->next_tree()) {
			auto tree = ptb->current_penn_tree();

			vector <string> orig_tokens = tree->get_terminal_yield();

			int depth = tree->getDepth();
			int len = orig_tokens.size();

			cout<<len<<" "<<depth<<endl;
		}
	}
};

#endif /*LPCFG_PARSER_H_*/
