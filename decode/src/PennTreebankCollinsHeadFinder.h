/*
 * PennTreebankCollinsHeaderFinder.h
 *
 *  Created on: Apr 21, 2015
 *      Author: scohen
 */

#ifndef PENNTREEBANKCOLLINSHEADFINDER_H_
#define PENNTREEBANKCOLLINSHEADFINDER_H_

#include "AbstractHeadFinder.h"

class PennTreebankCollinsHeadFinder: public AbstractHeadFinder {
public:
	PennTreebankCollinsHeadFinder() {
		readLine("## Head rules for the Penn Treebank for Foresto");
		readLine("");
		readLine("## This version from Collins' dissertation (1999: 236-238). Adapted from the Stanford parser.");
		readLine("");
		readLine( "ADJP^left NNS QP NN $ ADVP JJ VBN VBG ADJP JJR NP JJS DT FW RBR RBS SBAR RB");
		readLine("ADVP^right RB RBR RBS FW ADVP TO CD JJR JJ IN NP JJS NN");
		readLine("CONJP^right CC RB IN");
		readLine("FRAG^right");
		readLine("INTJ^left");
		readLine("LST^right LS :");
		readLine( "NAC^left NN NNS NNP NNPS NP NAC EX $ CD QP PRP VBG JJ JJS JJR ADJP FW");
		readLine("NX^left");
		readLine("PP^right IN TO VBG VBN RP FW");
		readLine("PRN^left");
		readLine("PRT^right RP");
		readLine("QP^left $ IN NNS NN JJ RB DT CD NCD QP JJR JJS");
		readLine("RRC^right VP NP ADVP ADJP PP");
		readLine("S^left TO IN VP S SBAR ADJP UCP NP");
		readLine("SBAR^left WHNP WHPP WHADVP WHADJP IN DT S SQ SINV SBAR FRAG");
		readLine("SBARQ^left SQ S SINV SBARQ FRAG");
		readLine("SINV^left VBZ VBD VBP VB MD VP S SINV ADJP NP");
		readLine("SQ^left VBZ VBD VBP VB MD VP SQ");
		readLine("UCP^right");
		readLine( "VP^left TO VBD VBN MD VBZ VB VBG VBP AUX AUXG VP ADJP NN NNS NP");
		readLine("WHADJP^left CC WRB JJ ADJP");
		readLine("WHADVP^right CC WRB");
		readLine("WHNP^left WDT WP WP$ WHADJP WHPP WHNP");
		readLine("WHPP^right IN TO FW");
		readLine("X^right");
		readLine("NP^rightdis NN NNP NNPS NNS NX POS JJR^left NP^rightdis $ ADJP PRN^right CD^rightdis JJ JJS RB QP");
		readLine("TYPO^left");
		readLine("EDITED^left");
		readLine("XS^right IN");
		readLine("");
		readLine("## punctuation");
		readLine("punc `` ,  : '' . PUNC");
	}
};

#endif /* PENNTREEBANKCOLLINSHEADFINDER_H_ */
