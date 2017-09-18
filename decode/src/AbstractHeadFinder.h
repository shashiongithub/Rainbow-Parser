/*
 * AbstractHeadFinder.h
 *
 *  Created on: Apr 20, 2015
 *      Author: scohen
 */

#ifndef ABSTRACTHEADFINDER_H_
#define ABSTRACTHEADFINDER_H_

// Adapted from the Stanford parser

class AbstractHeadFinder {

private:
	vector<string> defaultLeftRule;
	vector<string> defaultRightRule;

	unordered_map<string, vector<vector<string> > > nonTerminalInfo;

	unordered_set<string> puncTable;

	vector<string> defaultRule;

public:

	virtual bool isPunctuationTag(string label) {
		return (puncTable.find(label) != puncTable.end());
	}

	string transformLabelIsLeaf(string treeLabel) {
		string transformedLabel = treeLabel;
		auto cutIndex_ = transformedLabel.find('-');
		auto cutIndex2_ = transformedLabel.find('=');
		auto cutIndex3_ = transformedLabel.find('^');

		int cutIndex = (int) cutIndex_;
		int cutIndex2 = (int) cutIndex2_;
		int cutIndex3 = (int) cutIndex3_;

		if (cutIndex_ == std::string::npos) {
			cutIndex = -1;
		}
		if (cutIndex2_ == std::string::npos) {
			cutIndex2 = -1;
		}
		if (cutIndex3_ == std::string::npos) {
			cutIndex3 = -1;
		}

		if (cutIndex3 > 0 && (cutIndex3 < cutIndex2 || cutIndex2 == -1))
			cutIndex2 = cutIndex3;
		if (cutIndex2 > 0 && (cutIndex2 < cutIndex || cutIndex <= 0))
			cutIndex = cutIndex2;
		if (cutIndex > 0 && true /* !tree.isLeaf() */) {
			transformedLabel = transformedLabel.substr(0, cutIndex);
		}

		// correct for unspliced nodes (different than Berkely parser), added by
		// Shay
		if (transformedLabel.find('@') == 0) {
			transformedLabel = transformedLabel.substr(1);
		}

		return transformedLabel;
	}

	string stripUnaryChain(string treeLabel) {
		string pos = "";

		vector < string > tokens;

		string_utils::split(tokens, treeLabel, "|");

		if (tokens.size() == 0) {
			return "EMPTYLABEL";
		}

		return tokens[tokens.size() - 1];
	}

	string basicTag(string tag) {
		return transformLabelIsLeaf(stripUnaryChain(tag));
	}

	// This is taken from TreeAnnotations of the Berkeley parser
	string transformLabel(bracketed_tree<string>* tree) {
		auto transformedLabel = tree->get_label();

		auto cutIndex_ = transformedLabel.find('-');
		auto cutIndex2_ = transformedLabel.find('=');
		auto cutIndex3_ = transformedLabel.find('^');

		int cutIndex = (int) cutIndex_;
		int cutIndex2 = (int) cutIndex2_;
		int cutIndex3 = (int) cutIndex3_;

		if (cutIndex_ == std::string::npos) {
			cutIndex = -1;
		}
		if (cutIndex2_ == std::string::npos) {
			cutIndex2 = -1;
		}
		if (cutIndex3_ == std::string::npos) {
			cutIndex3 = -1;
		}

		if (cutIndex3 > 0 && (cutIndex3 < cutIndex2 || cutIndex2 == -1))
			cutIndex2 = cutIndex3;
		if (cutIndex2 > 0 && (cutIndex2 < cutIndex || cutIndex <= 0))
			cutIndex = cutIndex2;
		if (cutIndex > 0 && !tree->is_terminal()) {
			transformedLabel = transformedLabel.substr(0, cutIndex);
		}

		// correct for unspliced nodes (different than Berkeley parser), added by
		// Shay
		if (transformedLabel.find('@') == 0) {
			transformedLabel = transformedLabel.substr(1);
		}

		return transformedLabel;
	}

	/**
	 * Set categories which, if it comes to last resort processing (i.e. none of
	 * the rules matched), will be avoided as heads. In last resort processing,
	 * it will attempt to match the leftmost or rightmost constituent not in
	 * this set but will fall back to the left or rightmost constituent if
	 * necessary.
	 *
	 * @param categoriesToAvoid
	 *            list of constituent types to avoid
	 */
	void setCategoriesToAvoid(vector<string> categoriesToAvoid) {
		// automatically build defaultLeftRule, defaultRightRule
		defaultLeftRule.clear();
		defaultRightRule.clear();

		defaultLeftRule.push_back("leftexcept");
		defaultRightRule.push_back("rightexcept");

		defaultLeftRule.insert(defaultLeftRule.end(),
				categoriesToAvoid.begin(), categoriesToAvoid.end());
		defaultRightRule.insert(defaultRightRule.end(),
				categoriesToAvoid.begin(), categoriesToAvoid.end());
	}

	/**
	 * A way for subclasses for corpora with explicit head markings to return
	 * the explicitly marked head
	 *
	 * @param t
	 *            a tree to find the head of
	 * @return the marked head-- null if no marked head
	 */
	// to be overridden in subclasses for corpora
	//
	virtual bracketed_tree<string>* findMarkedHead(bracketed_tree<string>* t) {
		return NULL;
	}

	/**
	 * A way for subclasses to fix any heads under special conditions The
	 * default does nothing.
	 *
	 * @param headIdx
	 *            the index of the proposed head
	 * @param daughterTrees
	 *            the array of daughter trees
	 * @return the new headIndex
	 */
	int postOperationFix(int headIdx,
			vector<bracketed_tree<string>*> daughterTrees) {
		if (headIdx >= 2) {
			// String prevLab = tlp.basicCategory(daughterTrees[headIdx -
			// 1].value());
			string prevLab = transformLabel(daughterTrees[headIdx - 1]);

			if (isConjunction(prevLab)) {
				int newHeadIdx = headIdx - 2;
				bracketed_tree<string>* t = daughterTrees[newHeadIdx];
				auto l = t->get_label();
				while (newHeadIdx >= 0 && t->is_preterminal()
						&& isPunctuationTag(l)) {
					newHeadIdx--;
				}
				if (newHeadIdx >= 0) {
					headIdx = newHeadIdx;
				}
			}
		}
		return headIdx;
	}

	virtual bool isConjunction(string label) {
		if ((label == "CC") || (label == "CONJP")) {
			return true;
		}

		return false;
	}

	/**
	 * These are built automatically from categoriesToAvoid and used in a fairly
	 * different fashion from defaultRule (above). These are used for categories
	 * that do have defined rules but where none of them have matched. Rather
	 * than picking the rightmost or leftmost child, we will use these to pick
	 * the the rightmost or leftmost child which isn't in categoriesToAvoid.
	 */

	// TODO: note that the old Java version had some caching for "headAbove"
	// once it was calculated, to avoid multiple calculations of the head.
	// the same thing can be done here, I removed it for now. This means
	// the code is less efficient (Shay)

	bracketed_tree<string>* determineHeadAbove(bracketed_tree<string>* t,
			bracketed_tree<string>* tHead) {

		bracketed_tree<string>* parent = t->getParent();
		bracketed_tree<string>* head = NULL;

		while (parent != NULL) {
			head = determinePercolatedHead(parent);

			if (head != tHead) {
				return head;
			}

			if (head == NULL) {
				break;
			}

			parent = parent->getParent();
		}

		return NULL;
	}

	/**
	 * Determine which daughter of the current parse tree is the head.
	 *
	 * @param t
	 *            The parse tree to examine the daughters of. If this is a leaf,
	 *            <code>null</code> is returned
	 * @return The daughter parse tree that is the head of <code>t</code>
	 * @see Tree#percolateHeads(HeadFinder) for a routine to call this and
	 *      spread heads throughout a tree
	 */
	bracketed_tree<string>* determineHead(bracketed_tree<string>* t) {
		return determineHead(t, NULL);
	}

	bracketed_tree<string>* determinePercolatedHead(
			bracketed_tree<string>* tree) {

		bracketed_tree<string>* head = tree;

		while ((head != NULL) && (!head->is_preterminal())) {
			head = determineHead(head);
		}

		return head;
	}

	/**
	 * Determine which daughter of the current parse tree is the head.
	 *
	 * @param t
	 *            The parse tree to examine the daughters of. If this is a leaf,
	 *            <code>null</code> is returned
	 * @param parent
	 *            The parent of t
	 * @return The daughter parse tree that is the head of <code>t</code>.
	 *         Returns null for leaf nodes.
	 * @see Tree#percolateHeads(HeadFinder) for a routine to call this and
	 *      spread heads throughout a tree
	 */
	bracketed_tree<string>* determineHead(bracketed_tree<string>* t,
			bracketed_tree<string>* parent) {
		if (t == NULL || t->is_terminal()) {
			return NULL;
		}

		if (t->is_preterminal()) {
			return t;
		}

		bracketed_tree<string>* theHead;
		// first check if subclass found explicitly marked head
		if ((theHead = findMarkedHead(t)) != NULL) {
			return theHead;
		}

		// if the node is a unary, then that kid must be the head
		// it used to special case preterminal and ROOT/TOP case
		// but that seemed bad (especially hardcoding string "ROOT")
		if (t->children_count() == 1) {
			return t->get_child(0);
		}

		return determineNonTrivialHead(t, parent);
	}

	/**
	 * Called by determineHead and may be overridden in subclasses if special
	 * treatment is necessary for particular categories.
	 */
	bracketed_tree<string>* determineNonTrivialHead(bracketed_tree<string>* t,
			bracketed_tree<string>* parent) {
		bracketed_tree<string>* theHead = NULL;

		string motherCat = transformLabel(t);

		// Look at label.
		// a total special case....
		// first look for POS tag at end
		// this appears to be redundant in the Collins case since the rule
		// already would do that
		// Tree lastDtr = t.lastChild();
		// if (tlp.basicCategory(lastDtr.label().value()).equals("POS")) {
		// theHead = lastDtr;
		// } else {
		vector < vector<string> > how = nonTerminalInfo[motherCat];
		if (how.size() == 0) {
			/*
			 if (DEBUG) {
			 System.err.println("Warning: No rule found for " + motherCat
			 + " (first char: " + motherCat.charAt(0) + ')');
			 System.err.println("Known nonterms are: "
			 + nonTerminalInfo.keySet());
			 }*/
			if (defaultRule.size() > 0) {

				vector<bracketed_tree<string>*> kids;

				for (int i = 0; i < t->children_count(); i++) {
					kids.push_back(t->get_child(i));
				}

				return traverseLocate(kids, defaultRule, true);
			} else {
				return NULL;
			}
		}

		vector<bracketed_tree<string>*> kids;

		for (int i = 0; i < t->children_count(); i++) {
			kids.push_back(t->get_child(i));
		}

		for (auto i = 0 * how.size(); i < how.size(); i++) {

			bool lastResort = (i == how.size() - 1);
			theHead = traverseLocate(kids, how[i], lastResort);
			if (theHead != NULL) {
				break;
			}
		}

		/*if (DEBUG) {
		 System.err.println("  Chose " + theHead.getLabel());
		 }*/

		return theHead;
	}

	/**
	 * Attempt to locate head daughter tree from among daughters. Go through
	 * daughterTrees looking for things from a set found by looking up the
	 * motherkey specifier in a hash map, and if you do not find one, take
	 * leftmost or rightmost thing iff lastResort is true, otherwise return
	 * <code>null</code>.
	 */
	bracketed_tree<string>* traverseLocate(
			vector<bracketed_tree<string>*>& daughterTrees,
			vector<string>& how, bool lastResort) {
		auto headIdx = 0 * daughterTrees.size();
		string childCat;
		bool found = false;

		if (how[0] == "left") {
			for (auto i = 1 + 0 * how.size(); i < how.size(); i++) {
				for (headIdx = 0; headIdx < daughterTrees.size(); headIdx++) {
					childCat = transformLabel(daughterTrees[headIdx]);
					if (how[i] == childCat) {
						found = true;
						goto endloop1;
					}
				}
			}

			endloop1: 1;
		} else if (how[0] == "leftdis") {
			for (headIdx = 0; headIdx < daughterTrees.size(); headIdx++) {
				childCat = transformLabel(daughterTrees[headIdx]);
				for (auto i = 1 + 0 * how.size(); i < how.size(); i++) {
					if (how[i] == childCat) {
						found = true;
						goto endloop2;
					}
				}
			}

			endloop2: 1;
		} else if (how[0] == "right") {
			// from right
			for (auto i = 1 + 0 * how.size(); i < how.size(); i++) {
				for (headIdx = daughterTrees.size() - 1; headIdx >= 0; headIdx--) {
					childCat = transformLabel(daughterTrees[headIdx]);
					if (how[i] == childCat) {
						found = true;
						goto endloop3;
					}
				}

				endloop3: 1;
			}
		} else if (how[0] == "rightdis") {
			// from right, but search for any, not in turn
			for (headIdx = daughterTrees.size() - 1; headIdx >= 0; headIdx--) {
				// childCat = tlp.basicCategory(daughterTrees[headIdx].label()
				// .value());
				childCat = transformLabel(daughterTrees[headIdx]);
				for (auto i = 1 + 0 * how.size(); i < how.size(); i++) {
					if (how[i] == childCat) {
						found = true;
						goto endloop4;
					}
				}

				endloop4: 1;
			}
		} else if (how[0] == "leftexcept") {
			for (headIdx = 0; headIdx < daughterTrees.size(); headIdx++) {
				childCat = transformLabel(daughterTrees[headIdx]);
				found = true;
				for (auto i = 1 + 0 * how.size(); i < how.size(); i++) {
					if (how[i] == childCat) {
						found = false;
					}
				}
				if (found) {
					break;
				}
			}
		} else if (how[0] == "rightexcept") {
			for (headIdx = daughterTrees.size() - 1; headIdx >= 0; headIdx--) {
				// childCat = tlp.basicCategory(daughterTrees[headIdx].label()
				// .value());
				childCat = transformLabel(daughterTrees[headIdx]);
				found = true;
				for (auto i = 1 + 0 * how.size(); i < how.size(); i++) {
					if (how[i] == childCat) {
						found = false;
					}
				}
				if (found) {
					break;
				}
			}
		} else {
			cerr << "ERROR: invalid direction type " << how[0]
					<< " to nonTerminalInfo map in AbstractCollinsHeadFinder."
					<< endl;
		}

		// what happens if our rule didn't match anything
		if (!found) {
			if (lastResort) {
				// use the default rule to try to match anything except
				// categoriesToAvoid
				// if that doesn't match, we'll return the left or rightmost
				// child (by
				// setting headIdx). We want to be careful to ensure that
				// postOperationFix
				// runs exactly once.
				vector < string > rule;
				if (!how[0].compare(0, 4, "left")) {
					headIdx = 0;
					rule = defaultLeftRule;
				} else {
					headIdx = daughterTrees.size() - 1;
					rule = defaultRightRule;
				}
				bracketed_tree<string>* child =
						(bracketed_tree<string>*) traverseLocate(daughterTrees,
								rule, false);
				if (child != NULL) {
					return child;
				}
			} else {
				// if we're not the last resort, we can return null to let the
				// next rule try to match
				return NULL;
			}
		}

		headIdx = postOperationFix(headIdx, daughterTrees);

		return daughterTrees[headIdx];
	}

	void readLine(string line) {

		if (line == "") {
			return;
		}

		if (line.find('#') == 0) {
			return;
		}

		if (!line.compare(0, 4, "punc")) {
			vector < string > tokens;

			string_utils::split(tokens, line, " ");

			if (tokens.size() == 1) {
				return;
			}

			for (auto i = 1 + 0 * tokens.size(); i < tokens.size(); i++) {
				puncTable.insert(tokens[i]);
			}

			return;
		}

		vector < string > tokens;

		string_utils::split(tokens, line, "^");

		if (tokens.size() < 2) {
			return;
		}

		string tag = tokens[0];

		vector < vector<string> > strs;

		for (auto i = 1 + 0 * tokens.size(); i < tokens.size(); i++) {
			vector < string > list;

			string_utils::split(list, tokens[i], " ");

			strs.push_back(list);
		}

		nonTerminalInfo[tag] = strs;
	}
};

#endif /* ABSTRACTHEADFINDER_H_ */
