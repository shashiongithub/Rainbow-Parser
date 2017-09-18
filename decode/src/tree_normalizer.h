/*
 * tree_normalizer.h
 *
 *  Created on: Feb 2, 2015
 *      Author: scohen
 */

#ifndef TREE_NORMALIZER_H_
#define TREE_NORMALIZER_H_

#define RIGHT_NORMALIZATION false

#include "trees.h"

using namespace std;

template<class L> class TreeNormalizer {
public:

	bracketed_tree<L>* removeTraces(bracketed_tree<L>* mainTree) {
		bracketed_tree<L>* newTree = mainTree->get_new_empty_tree(NULL);

		auto l = mainTree->get_label();

		newTree->set_label(l);

		return removeTracesHelper(mainTree, newTree);
	}

	bracketed_tree<L>* removeTracesHelper(bracketed_tree<L>* inputTree,
			bracketed_tree<L>* outputTree) {

		if (inputTree->is_terminal()) {
			auto l = inputTree->get_label();

			outputTree->set_label(l);

			return outputTree;
		}

		for (int i = 0 * inputTree->children_count(); i
				< inputTree->children_count(); i++) {
			if (!isNodeTrace(inputTree->get_child(i))) {

				if (inputTree->get_child(i)->is_preterminal()) {
					auto emptyTree = inputTree->get_child(i)->copy(outputTree);

					outputTree->add_child(emptyTree);
				} else {
					auto emptyTree = inputTree->get_new_empty_tree(outputTree);

					auto l = inputTree->get_child(i)->get_label();

					emptyTree->set_label(l);

					outputTree->add_child(
							removeTracesHelper(inputTree->get_child(i),
									emptyTree));
				}
			}
		}

		return outputTree;
	}

	bool isNodeTrace(bracketed_tree<L>* inputTree) {
		if (inputTree->is_preterminal()) {
			if ((isCategoryEmpty(inputTree->get_child(0)->get_label()))
					|| (isCategoryEmpty(inputTree->get_label())))
				return true;
			else
				return false;
		}

		for (auto i = 0 * inputTree->children_count(); i
				< inputTree->children_count(); i++) {
			if (!isNodeTrace(inputTree->get_child(i))) {
				return false;
			}
		}

		return true;
	}

	virtual bool isCategoryEmpty(L label) = 0;

	bracketed_tree<L>* collapseUnaryRules(bracketed_tree<L>* mainTree) {
		bracketed_tree<L>* newTree = mainTree->get_new_empty_tree(NULL);

		return collapseUnaryRuleHelper(true, mainTree, mainTree->get_label(),
				newTree);
	}

	bracketed_tree<L>* collapseUnaryRuleHelper(bool isRoot,
			bracketed_tree<L>* inputTree, L label, bracketed_tree<L>* emptyTree) {

		if (inputTree->is_preterminal()) {
			emptyTree->set_label(label);

			L childLabel = inputTree->get_child(0)->get_label();

			emptyTree->add_child(emptyTree->get_new_empty_tree(emptyTree));
			emptyTree->get_child(0)->set_label(childLabel);

			return emptyTree;
		}

		if (isRoot) {
			bracketed_tree<L>* emptyTree2 = inputTree->get_new_empty_tree(emptyTree);

			emptyTree->add_child(emptyTree2);

			emptyTree->set_label(label);

			collapseUnaryRuleHelper(false, inputTree->get_child(0),
					inputTree->get_child(0)->get_label(), emptyTree2);

			return emptyTree;
		}

		if (inputTree->children_count() == 1) {
			L childLabel = inputTree->get_child(0)->get_label();
			return collapseUnaryRuleHelper(false, inputTree->get_child(0),
					concatLabel(label, childLabel), emptyTree);

			return emptyTree;
		}

		emptyTree->set_label(label);

		for (int i = 0; i < inputTree->children_count(); i++) {
			bracketed_tree<L>* newTree = inputTree->get_new_empty_tree(emptyTree);
			L childLabel = inputTree->get_child(i)->get_label();

			newTree = collapseUnaryRuleHelper(false, inputTree->get_child(i),
					childLabel, newTree);

			emptyTree->add_child(newTree);
		}

		return emptyTree;
	}

	bracketed_tree<L>* binarizeTree(bracketed_tree<L>* mainTree) {
		L headLabel = mainTree->get_label();

		vector<L> labelList;

		for (int i = 0; i < mainTree->children_count(); i++) {
			labelList.push_back(mainTree->get_child(i)->get_label());
		}

		bracketed_tree<L>* emptyTree = mainTree->get_new_empty_tree(NULL);

		auto t = binarizeTreeHelper(true, mainTree, 0, false, headLabel,
				labelList, emptyTree);

		return t;
	}

	virtual L convertHeadLabel(L& l, bool isIntermediate, bool isPre,
			bool removePostDash)
	= 0;

	virtual L concatLabel(L& l1, L& l2) = 0;

	bracketed_tree<L>* binarizeTreeHelper(bool isRoot,
			bracketed_tree<L>* inputTree, int childPtr, bool isIntermediate,
			L label, vector<L> labels, bracketed_tree<L>* outputTree) {

		if (inputTree->children_count() == 0) {
			L myLabel = inputTree->get_label();

			outputTree->set_label(myLabel);

			return outputTree;
		}

		if (isRoot) {
		 bracketed_tree<L>* emptyTree2 = inputTree->get_new_empty_tree(outputTree);

		 outputTree->add_child(emptyTree2);

		 outputTree->set_label(label);

		 vector<L> labels;

		 for (int i = 0; i < inputTree->get_child(0)->children_count(); i++) {

		 labels.push_back(
		 inputTree->get_child(0)->get_child(i)->get_label());
		 }

		 binarizeTreeHelper(false, inputTree->get_child(0), 0, false,
		 inputTree->get_child(0)->get_label(), labels, emptyTree2);

		 return outputTree;
		 }

		L headLabelNow = convertHeadLabel(label, isIntermediate,
				inputTree->is_preterminal(), true);

		outputTree->set_label(headLabelNow);

		if (labels.size() == 2) {
			bracketed_tree<L>* emptyTree1 = inputTree->get_new_empty_tree(outputTree);
			bracketed_tree<L>* emptyTree2 = inputTree->get_new_empty_tree(outputTree);

			vector<L> labels1;
			vector<L> labels2;

			for (int i = 0; i
					< inputTree->get_child(childPtr)->children_count(); i++) {
				labels1.push_back(
						inputTree->get_child(childPtr)->get_child(i)->get_label());
			}

			for (int i = 0; i
					< inputTree->get_child(childPtr + 1)->children_count(); i++) {
				labels2.push_back(
						inputTree->get_child(childPtr + 1)->get_child(i)->get_label());
			}

			auto myTree1 = binarizeTreeHelper(false,
					inputTree->get_child(childPtr), 0, false,
					inputTree->get_child(childPtr)->get_label(), labels1,
					emptyTree1);
			auto myTree2 = binarizeTreeHelper(false,
					inputTree->get_child(childPtr + 1), 0, false,
					inputTree->get_child(childPtr + 1)->get_label(), labels2,
					emptyTree2);

			outputTree->add_child(myTree1);
			outputTree->add_child(myTree2);

			return outputTree;
		}

		if (labels.size() == 1) {
			bracketed_tree<L>* emptyTree1;

			emptyTree1 = inputTree->get_new_empty_tree(outputTree);

			vector<L> labels1;

			for (int i = 0; i
					< inputTree->get_child(childPtr)->children_count(); i++) {
				labels1.push_back(
						inputTree->get_child(childPtr)->get_child(i)->get_label());
			}

			auto myTree1 = binarizeTreeHelper(false,
					inputTree->get_child(childPtr), 0, false,
					inputTree->get_child(childPtr)->get_label(), labels1,
					emptyTree1);

			outputTree->add_child(myTree1);

			return outputTree;
		}

		if (labels.size() > 2) {

			if (RIGHT_NORMALIZATION) {
				bracketed_tree<L>* emptyTree1 = inputTree->get_new_empty_tree(outputTree);
				bracketed_tree<L>* emptyTree2 = inputTree->get_new_empty_tree(outputTree);

				vector<L> labels1;

				for (int i = 0; i < inputTree->get_child(0)->children_count(); i++) {
					labels1.push_back(
							inputTree->get_child(0)->get_child(i)->get_label());
				}

				auto myTree1 = binarizeTreeHelper(false,
						inputTree->get_child(0), 0, true, labels[0], labels1,
						emptyTree1);

				labels.erase(labels.begin());

				auto myTree2 = binarizeTreeHelper(false, inputTree,
						childPtr + 1, true, label, labels, emptyTree2);

				outputTree->add_child(myTree1);
				outputTree->add_child(myTree2);

				return outputTree;
			} else {
				bracketed_tree<L>* emptyTree1 = inputTree->get_new_empty_tree(outputTree);
				bracketed_tree<L>* emptyTree2 = inputTree->get_new_empty_tree(outputTree);

				vector<L> labels1;

				int lastChildIdx = labels.size() - 1;

				for (int i = 0; i
						< inputTree->get_child(lastChildIdx)->children_count(); i++) {
					labels1.push_back(
							inputTree->get_child(lastChildIdx)->get_child(i)->get_label());
				}

				auto myTree1 = binarizeTreeHelper(false,
						inputTree->get_child(lastChildIdx), 0, true,
						labels[labels.size() - 1], labels1, emptyTree1);

				labels.erase(labels.end());

				auto myTree2 = binarizeTreeHelper(false, inputTree, childPtr,
						true, label, labels, emptyTree2);

				outputTree->add_child(myTree2);
				outputTree->add_child(myTree1);

				return outputTree;
			}
		}

		assert(0);

		return NULL;
	}

};

class StringTreeNormalizer: public TreeNormalizer<string> {
public:
	string convertHeadLabel(string& l, bool isIntermediate, bool isPre,
			bool removePostDash) {

		if (removePostDash) {
			if (l.find('-') != string::npos) {
				vector<string> output;

				string_utils::split(output, l, "-");

				if (output.size() == 0) {
					l = "EMPTYLABEL";
				} else {
					l = output[0];
				}
			}

			if (l.find('=') != string::npos) {
				vector<string> output;

				string_utils::split(output, l, "=");

				l = output[0];
			}
		}

		if (isPre) {
			return l;
		}

		if (l == "TOP") {
			return l;
		}

		if (isIntermediate) {
			return "@" + l + "^g";
		} else {
			return l + "^g";
		}
	}

	string concatLabel(string& l1, string& l2) {
		return l1 + "|" + l2;
	}

	bool isCategoryEmpty(string label) {
		if (label.find('*') == 0) {
			return true;
		}

		if (label == "-NONE") {
			return true;
		}

		return false;
	}
};

#endif /* TREE_NORMALIZER_H_ */
