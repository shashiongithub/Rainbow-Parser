#ifndef TREES_H_
#define TREES_H_

#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include "span_bag.h"

using namespace std;

class abstract_tree {

};

template<class L> class bracketed_tree: public abstract_tree {
protected:
	vector<bracketed_tree<L>*> children;
	bracketed_tree<L>* parent;

public:

	bracketed_tree<L> (bracketed_tree<L>* parent) {
		this->parent = parent;
	}

	virtual ~bracketed_tree<L> () {
		for (auto i = children.size() * 0; i < children.size(); i++) {
			delete children[i];
		}
	}

	int getDepth()
	{
		int d = -1;
		if (children.size() <= 1) { return 1; }

		for (int i=0; i < children.size(); i++) {
			if (children[i]->getDepth() > d) { d = children[i]->getDepth(); }
		}

		return d + 1;
	}

	bracketed_tree<L>* getParent() {
		return parent;
	}

	bracketed_tree<L>* getRoot() {
		if (parent == NULL) {
			return this;
		} else {
			return getParent()->getRoot();
		}
	}

	bracketed_tree<L>* get_child(unsigned int child_num) const {
		assert(child_num < children.size());

		return children[child_num];
	}

	void add_child(bracketed_tree<L>* tree) {
		assert(this == tree->getParent());
		children.push_back(tree);
	}

	virtual void analyzeNodeInformation(string nodeInfo) {
		// stub
	}

	virtual bracketed_tree<L>* get_new_empty_tree(bracketed_tree<L>* p) = 0;

	virtual bracketed_tree<L>* copy(bracketed_tree<L>*) = 0;

	int children_count() const {
		return children.size();
	}

	virtual void set_label(L& l) = 0;

	virtual L get_label() const = 0;

	virtual bool is_preterminal() const {
		return ((children_count() == 1) && (get_child(0) -> children_count()
				== 0));
	}

	virtual bool is_terminal() const {
		return (children_count() == 0);
	}
};

template<class L> class pennbank_tree: public bracketed_tree<L> {
protected:
	L label;

	int descendents_count;

public:
	pennbank_tree(pennbank_tree<L>* p, L& l) :
		bracketed_tree<L> (p) {
		this->label = l;
		descendents_count = 0;
	}

	pennbank_tree(pennbank_tree<L>* p) :
		bracketed_tree<L> (p) {
		descendents_count = 0;
	}

	bracketed_tree<L>* get_new_empty_tree(bracketed_tree<L>* parent) {
		return new pennbank_tree<L> ((pennbank_tree<L>*) parent);
	}

	bracketed_tree<L>* copy(bracketed_tree<L>* parent) {
		if (this->is_terminal()) {
			auto tree = get_new_empty_tree(parent);
			tree->set_label(label);

			return tree;
		} else {
			auto tree = get_new_empty_tree(parent);
			tree->set_label(label);

			for (auto i = 0 * (this->children_count()); i
					< this->children_count(); i++) {
				tree->add_child(this->get_child(i)->copy(tree));
			}

			return tree;
		}
	}

	vector<L> get_preterminal_yield() {
		vector<L> labels;

		get_preterminal_yield_helper(labels);

		return labels;
	}

	vector<L> get_terminal_yield() {
		vector<L> labels;

		get_terminal_yield_helper(labels, NULL);

		return labels;
	}

	vector<L> getOutsideTerminalYield() {
		vector<L> labels;

		((pennbank_tree<L>*) (this->getRoot()))->get_terminal_yield_helper(
				labels, this);

		return labels;
	}

	vector<L*> get_terminal_yield_ref() {
		vector<L*> labels;

		get_terminal_yield_helper_ref(labels);

		return labels;
	}

	void compute_descendent_counts() {
		if (this->children_count() == 0) {
			descendents_count = 1;
		} else {
			descendents_count = 0;

			for (int i = 0; i < this->children_count(); i++) {
				((pennbank_tree<L>*) (this->get_child(i)))->compute_descendent_counts();
				descendents_count
						+= ((pennbank_tree<L>*) (this->get_child(i)))->descendent_count();
			}
		}
	}

	int descendent_count() {
		return descendents_count;
	}

	void get_preterminal_yield_helper(vector<L>& v) {
		if (!this->is_preterminal()) {
			for (int i = 0; i < this->children_count(); i++) {
				((pennbank_tree<L>*) (this->get_child(i)))->get_preterminal_yield_helper(
						v);
			}
		} else {
			v.push_back(label);
		}
	}

	void get_terminal_yield_helper(vector<L>& v, bracketed_tree<L>* excluding) {
		if (this == excluding) {
			v.push_back("FOOTER");
			return;
		}

		if (this->children_count() > 1) {
			for (int i = 0; i < this->children_count(); i++) {
				((pennbank_tree<L>*) (this->get_child(i)))->get_terminal_yield_helper(
						v, excluding);
			}
		} else if (this->children_count() == 1) {
			((pennbank_tree<L>*) (this->get_child(0)))->get_terminal_yield_helper(
					v, excluding);
		} else {
			v.push_back(label);
		}
	}

	void get_terminal_yield_helper_ref(vector<L*>& v) {
		if (this->children_count() > 1) {
			for (int i = 0; i < this->children_count(); i++) {
				((pennbank_tree<L>*) (this->get_child(i)))->get_terminal_yield_helper_ref(
						v);
			}
		} else if (this->children_count() == 1) {
			((pennbank_tree<L>*) (this->get_child(0)))->get_terminal_yield_helper_ref(
					v);
		} else {
			v.push_back(&label);
		}
	}

	void set_label(L& l) {
		label = l;
	}

	L get_label() const {
		return label;
	}

	string to_string() const {
		ostringstream my_stream;

		to_string_helper(my_stream);

		return my_stream.str();
	}

	void to_string_helper(ostringstream& my_stream) const {
		if (this->children_count() == 0) {
			my_stream << get_label();

			return;
		}

		my_stream << "(" << get_label() << " ";

		for (int i = 0; i < this->children_count(); i++) {
			((pennbank_tree<L>*) this->get_child(i))->to_string_helper(
					my_stream);
		}
		my_stream << ") ";
	}

	void destroy_bag(cnf_span_bag* bag) {
		delete bag;
	}

	cnf_span_bag* create_bag(
			grammar_alphabet<inpre_symbol, vocab_symbol>* alphabet) {
		cnf_span_bag* bag = new cnf_span_bag();

		create_bag_helper(alphabet, 0, descendent_count(), bag, true);

		return bag;
	}

	void create_bag_helper(
			grammar_alphabet<inpre_symbol, vocab_symbol>* alphabet,
			my_uint left_corner, my_uint right_corner, cnf_span_bag* bag,
			bool is_root) {
		if (descendent_count() > 1) {
			assert(this->children_count() == 2);

			pennbank_tree<string>* left_child =
					(pennbank_tree<string>*) (this->get_child(0));
			pennbank_tree<string>* right_child =
					(pennbank_tree<string>*) (this->get_child(1));

			inpre_symbol b_sym, c_sym;

			if (left_child->children_count() == 1) {
				b_sym = alphabet->preterminal_lookup_string(
						left_child->get_label());
			} else {
				b_sym = alphabet->nonterminal_lookup_string(
						left_child->get_label());
			}

			if (right_child->children_count() == 1) {
				c_sym = alphabet->preterminal_lookup_string(
						right_child->get_label());
			} else {
				c_sym = alphabet->nonterminal_lookup_string(
						right_child->get_label());
			}

			cnf_binary_rule<inpre_symbol> rule(
					alphabet->nonterminal_lookup_string(get_label()), b_sym,
					c_sym);

			bag->add_binary_rule(rule, left_corner,
					left_corner + left_child->descendent_count(), right_corner,
					is_root);

			left_child->create_bag_helper(alphabet, left_corner,
					left_corner + left_child->descendent_count(), bag, false);
			right_child->create_bag_helper(alphabet,
					left_corner + left_child->descendent_count(), right_corner,
					bag, false);

		} else if (descendent_count() == 1) {
			assert(this->children_count() == 1);

			pennbank_tree<string>* child =
					(pennbank_tree<string>*) (this->get_child(0));

			bag->add_term_rule(
					alphabet->preterminal_lookup_string(get_label()),
					alphabet->vocab_lookup_string(child->get_label()),
					left_corner, right_corner);
		}
	}

	static pennbank_tree<string>* from_string(string bracket_str) {

		string empty_label = "";

		auto t = new pennbank_tree<string> (NULL, empty_label);

		auto t2 = from_string(bracket_str, t);

		delete t;

		return t2;
	}

	static pennbank_tree<string>* from_string(string bracket_str,
			pennbank_tree<string>* factory) {
		vector<pennbank_tree*> stack_of_trees;
		pennbank_tree<string>* current_tree = NULL;
		pennbank_tree<string>* final_tree = NULL;

		char leftP, rightP;

		if ((bracket_str.length() > 1) && (bracket_str[0] == '{')) {
			leftP = '{'; rightP = '}';
		} else {
			leftP = '('; rightP = ')';
		}

		if (bracket_str == "()") {
			L empty_label = "";
			auto p = factory->get_new_empty_tree(NULL);

			p->set_label(empty_label);

			return (pennbank_tree<string>*) p;
			//return new pennbank_tree<string> (NULL, empty_label);
		}

		for (auto i = 0 * bracket_str.length(); i < bracket_str.length(); i++) {
			char current_character = bracket_str[i];

			if (current_character == leftP) {
				bool last_was_bracket = true;

				vector<L> stack_of_labels;

				ostringstream current_label;

				do {
					i++;
					current_character = bracket_str[i];

					if ((current_character == ' ')
							|| (current_character == rightP) || (current_character
							== leftP)) {

						if (!last_was_bracket) {
							stack_of_labels.push_back(current_label.str());
						}

						current_label.clear();
						current_label.str(std::string());
					} else {
						last_was_bracket = false;

						current_label << current_character;
					}

				} while ((current_character != leftP) && (current_character
						!= rightP));

				i--;

				/*if (stack_of_labels.size() <= 1) {
				 //cerr<<"error in tree "<<stack_of_labels[0]<<endl;
				 // TODO error in tree
				 //return NULL;
				 }*/

				auto tree = (pennbank_tree<string>*)factory->get_new_empty_tree(current_tree);

				vector<string> tokens;

				bool splitByDash = false;

				if (splitByDash) {
					string_utils::split(tokens, stack_of_labels[0], "#");
				} else {
					tokens.push_back(stack_of_labels[0]);
				}

				tree->set_label(tokens[0]);

				if (tokens.size() > 1) {
					tree->analyzeNodeInformation(tokens[1]);
				}

				/*pennbank_tree* tree = new pennbank_tree<string> (current_tree,
				 stack_of_labels[0]);*/

				if (current_tree != NULL) {
					current_tree->add_child(tree);
				}

				stack_of_trees.push_back(tree);

				for (auto i = 1 + 0 * stack_of_labels.size(); i
						< stack_of_labels.size(); i++) {
					const char* st = stack_of_labels[i].c_str();

					bool no_space = false;

					while (*st) {
						if (*st != ' ') {
							no_space = true;
						}
						st++;
					}

					if (no_space) {
						auto p = factory->get_new_empty_tree(tree);

						vector<string> tokens;

						if (splitByDash) {
							string_utils::split(tokens, stack_of_labels[i], "#");
						} else {
							tokens.push_back(stack_of_labels[i]);
						}

						p->set_label(tokens[0]);

						if (tokens.size() > 1) {
							p->analyzeNodeInformation(tokens[1]);
						}

						tree->add_child(p);
					}
				}

				current_tree = tree;
			} else if (current_character == rightP) {
				assert(stack_of_trees.size()> 0);

				if (stack_of_trees.size() == 1) {
					final_tree = current_tree;
				}

				stack_of_trees.pop_back();

				if (stack_of_trees.size() == 0) {
					current_tree = NULL;
				} else {
					current_tree = stack_of_trees[stack_of_trees.size() - 1];
				}
			}
		}

		assert(final_tree != NULL);

		final_tree->compute_descendent_counts();

		return final_tree;
	}

};

#endif /*TREES_H_*/
