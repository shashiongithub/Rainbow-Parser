#ifndef TREEBANKS_H_
#define TREEBANKS_H_

#include <iostream>

using namespace std;

template<class L> class treebank {
public:

	virtual bool next_tree(bool parse_tree = true) = 0;

	virtual bracketed_tree<L>* current_tree() = 0;

	virtual void destroy_current_tree(bracketed_tree<L>* tree) = 0;

};

class penn_treebank: public treebank<string> {
private:
	pennbank_tree<string>* cur_tree;
	pennbank_tree<string>* factory;

	ifstream* myfile;
	bool destroy_iter;
	string my_filename;

public:
	penn_treebank() {
		cur_tree = NULL;
		myfile = NULL;
		destroy_iter = false;
		factory = NULL;
	}

	penn_treebank(pennbank_tree<string>* f) {
		cur_tree = NULL;
		myfile = NULL;
		destroy_iter = false;

		string empty_label = "";
		factory = (pennbank_tree<string>*)f->get_new_empty_tree(NULL);
	}

	virtual ~penn_treebank() {
		cleanup();
	}

	void cleanup() {
		delete factory;
		factory = NULL;

		if (destroy_iter) {
			delete cur_tree;
			cur_tree = NULL;
		}

		if (myfile != NULL) {
			myfile->close();

			delete myfile;
			myfile = NULL;
		}
	}

	void reset_iterator() {
		open_file(my_filename, destroy_iter);
	}

	bool open_file(string filename, bool destroy_on_iter = true) {
		cleanup();

		my_filename = filename;

		destroy_iter = destroy_on_iter;

		myfile = new ifstream(filename.c_str());

		if (myfile->is_open()) {
			return true;
		} else {
			return false;
		}
	}

	bool next_tree(bool parse_tree = true) {
		assert(myfile != NULL);

		if (destroy_iter) {
			delete cur_tree;
			cur_tree = NULL;
		}

		string line;

		if (getline(*myfile, line)) {
			if (parse_tree) {
				if (factory != NULL) {
					cur_tree = pennbank_tree<string>::from_string(line, factory);
				} else {
					cur_tree = pennbank_tree<string>::from_string(line);
				}
				assert(cur_tree != NULL);
			}

			return true;
		} else {
			return false;
		}
	}

	bracketed_tree<string>* current_tree() {
		return cur_tree;
	}

	pennbank_tree<string>* current_penn_tree() {
		return (pennbank_tree<string>*) cur_tree;
	}

	void destroy_current_tree(bracketed_tree<string>* tree) {
		delete cur_tree;
		cur_tree = NULL;
	}

	bracketed_tree<string>* getXYTree(bracketed_tree<string>* treeSoFar,
			vector<string>& tokens, int i) {

		string Xlabel = "X";
		string Ylabel = "Y";

		if (i == 1) {
			auto myTree1 = treeSoFar->get_new_empty_tree(treeSoFar);
			auto myTree2 = treeSoFar->get_new_empty_tree(treeSoFar);

			myTree1->set_label(Ylabel);
			myTree1->add_child(myTree1->get_new_empty_tree(myTree1));
			myTree1->get_child(0)->set_label(tokens[i - 1]);

			myTree2->set_label(Ylabel);
			myTree2->add_child(myTree1->get_new_empty_tree(myTree2));
			myTree2->get_child(0)->set_label(tokens[i]);

			treeSoFar->add_child(myTree1);
			treeSoFar->add_child(myTree2);

			return treeSoFar;
		} else {
			if (i < 2) {
				return treeSoFar;
			}

			auto myTree = treeSoFar->get_new_empty_tree(treeSoFar);

			auto newXtree = treeSoFar->get_new_empty_tree(treeSoFar);

			newXtree->set_label(Xlabel);

			myTree->set_label(Ylabel);
			myTree->add_child(myTree->get_new_empty_tree(myTree));
			myTree->get_child(0)->set_label(tokens[i]);

			treeSoFar->add_child(getXYTree(newXtree, tokens, i - 1));
			treeSoFar->add_child(myTree);

			return treeSoFar;
		}
	}

	bool writeXYTreebank(string filename, string outFilename) {

		string Xlabel = "X";

		auto myfile = new ifstream(filename.c_str());

		auto outfile = new ofstream(outFilename.c_str());

		if (!myfile->is_open()) {
			return false;
		}

		string line;

		while (getline(*myfile, line)) {
			vector<string> vec;

			string_utils::split(vec, line, " ");

			pennbank_tree<string>* tree = new pennbank_tree<string> (NULL);
			tree->set_label(Xlabel);

			tree
					= (pennbank_tree<string>*) getXYTree(tree, vec,
							vec.size() - 1);

			(*outfile) << tree->to_string() << endl;

			delete tree;
		}

		myfile->close();

		delete myfile;

		outfile->close();

		delete outfile;

		cerr << "Done preparing treebank from " << filename << " (into "
				<< outFilename << ")" << endl;

		return true;
	}
};

#endif /*TREEBANKS_H_*/
