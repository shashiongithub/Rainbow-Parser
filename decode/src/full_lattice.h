#ifndef __full_lattice_h__
#define __full_lattice_h__

#include <set>

// format of lattice:
// NumberOfStates State,State:Symbol State,State:Symbol State,State:Symbol, etc.
// cannot use pruner with them

// need to change the span_length == 1 or > 1 to
// is_single_edge, etc. or span length = 1

// what is the definition of mid points? all states that are in between these two states?

// need to check if length() and everything that involves numStates makes sense

template <class E> class FullLattice : public abstract_lattice<E>
{
protected:
	int numStates;

	int left_idx, right_idx, current_len;

	bool iterating;

	array_2d<E> symbols;
	array_2d<double> weights;
	array_2d< vector < int > > midPointsForward;
	array_2d<bool> hasSpan;

	cnf_span_bag* constrain_bag;

	int iter_type;

	bool iterating_incoming_state;

        int current_incoming_state;
        int target_incoming_state;


public:
	void computeClosure(int i, int j, bool rightBranch, int lastState)
	{
		if ((j != lastState) && rightBranch) return;

		for (int k=i+1; k < j; k++)
		{
			computeClosure(i, k, rightBranch, lastState);
			computeClosure(k, j, rightBranch, lastState);

			vector<int> v = midPointsForward[i][k];
			midPointsForward[i][j].insert(midPointsForward[i][j].end(), v.begin(), v.end());

			v = midPointsForward[k][j];
			midPointsForward[i][j].insert(midPointsForward[i][j].end(), v.begin(), v.end());

			if (hasSpan[i][k]) {
				midPointsForward[i][j].push_back(k);
			} else if (hasSpan[k][j]) {
				midPointsForward[i][j].push_back(k);
			}

			vector<int>& vec = midPointsForward[i][j];
			set<int> s;
			unsigned size = vec.size();
			for( unsigned i = 0; i < size; ++i ) s.insert( vec[i] );
			vec.assign( s.begin(), s.end() );
		}
	}

	virtual E& symbol_at(my_uint s1, my_uint s2) { assert(hasSpan[s1][s2]); return symbols[s1][s2]; }

	virtual double weight_at(my_uint s1, my_uint s2) { assert(hasSpan[s1][s2]); return weights[s1][s2]; }

	FullLattice(cnf_span_bag* bag) {

	        this->constrain_bag = bag;

		iterating = false;
	}

	FullLattice(int n, string s = "", bool rightBranch = false) {

	        this->constrain_bag = NULL;

		this->numStates = n;

		iterating = false;

		E* symbolsPtr = new E[numStates*numStates];
		double* weightsPtr = new double[numStates*numStates];
		vector<int>* setsPtr = new vector<int>[numStates*numStates];
		bool* hasSpanPtr = new bool[numStates*numStates];

		this->weights = array_2d<double>(weightsPtr, numStates, numStates);
		this->symbols = array_2d<E>(symbolsPtr, numStates, numStates);
		this->midPointsForward = array_2d<vector< int > >(setsPtr, numStates, numStates);
		this->hasSpan = array_2d<bool>(hasSpanPtr, numStates, numStates);

		for (int i=0; i < numStates; i++) {
			for (int j=0; j < numStates; j++) {
				this->hasSpan[i][j] = false;
				this->weights[i][j] = 1.0;
			}
		}

		if (s != "" ) {
			myFromString(s, rightBranch);
		}
	}

	void myFromString(string latticeString, bool rightBranch = false)
	{
		vector<string> tokens;

		string_utils::split(tokens, latticeString, " ");

		assert(tokens[0] == "LPCFGLatticeMarker");

		numStates = stoi(tokens[1]);

		E* symbolsPtr = new E[numStates*numStates];
		vector<int>* setsPtr = new vector<int>[numStates*numStates];
		bool* hasSpanPtr = new bool[numStates*numStates];

		symbols = array_2d<E>(symbolsPtr, numStates, numStates);
		midPointsForward = array_2d<vector< int > >(setsPtr, numStates, numStates);
		hasSpan = array_2d<bool>(hasSpanPtr, numStates, numStates);

		for (int i=0; i < numStates; i++) {
			for (int j=0; j < numStates; j++) {
				hasSpan[i][j] = false;
			}
		}

		for (int i=2; i < tokens.size(); i++)
		{
			vector<string> stateTokens;

			string_utils::split(stateTokens, tokens[i], "^");

			int leftPoint = stoi(stateTokens[0]);
			int rightPoint = stoi(stateTokens[1]);
			//E symbol = alphabet->vocab_lookup_string(stateTokens[2]);
			//inpre_symbol tag = alphabet->preterminal_lookup_string(stateTokens[3]);

			//symbols[leftPoint][rightPoint] = symbol;
			//tags[leftPoint][rightPoint] = tag;

			hasSpan[leftPoint][rightPoint] = true;

			if (stateTokens.size() >= 5) {
				string midPointsStr = stateTokens[4];

				vector<string> midPointTokens;

				string_utils::split(midPointTokens, midPointsStr, ",");

				for (int j=0; j < midPointTokens.size(); j++)
				{
					midPointsForward[leftPoint][rightPoint].push_back(stoi(midPointTokens[j]));
				}

				/*for (int j=midPointTokens.size()-1; j >= 0; j--)
				{
					midPointsBackward[leftPoint][rightPoint].push_back(stoi(midPointTokens[j]));
				}*/
			}
		}

		computeClosure(0, numStates-1, rightBranch, numStates-1);
	}

	my_uint span_length(my_uint i, my_uint j) { return j - i; }

	void destroy_all()
	{
		symbols.destroy();
		midPointsForward.destroy();
		hasSpan.destroy();
		weights.destroy();
	}

	my_uint length() const
	{
		return numStates-1;
	}


	void reset_span_iterator(int iter_type)
	{
        	this->iter_type = iter_type;

        	if (iter_type == abstract_lattice<E>::iterator_forward) {
                	left_idx = -1;
                	right_idx = 0;
                	current_len = 1;

                	iterating = true;
       		 } else if (iter_type == abstract_lattice<E>::iterator_reverse) {
                	left_idx = -1;
                	right_idx = 0;
                	current_len = this->numStates;

               		iterating = true;
        	} else {
        	        assert(false);
        	}
	}

	bool next_span()
	{
	        // assert iterating

        	if (iter_type == abstract_lattice<E>::iterator_forward) {
                	left_idx++;
                	right_idx = left_idx + current_len;

               		 if (right_idx > this->numStates) {
                        	left_idx = 0;
                        	current_len++;
                        	right_idx = left_idx + current_len;

                        	if (right_idx >= this->numStates) {
                                	iterating = false;

                                	return false;
                        	}
                	}
        	} else if (iter_type == abstract_lattice<E>::iterator_reverse) {
                	left_idx++;
                	right_idx = left_idx + current_len;

                	if (right_idx > this->numStates) {
                	        left_idx = 0;
                	        current_len--;
                	        right_idx = left_idx + current_len;

                	        if (current_len == 0) {
                	                iterating = false;

                	                return false;
                	        }
                	}
        	} else {
                	assert(false);

              	  return false;
        	}

		if (hasSpan[left_state()][right_state()]) {
			return true;
		} else {
			return next_span();
		}
	}

        my_uint left_state()
	{
		return left_idx;
	}

        my_uint right_state()
	{
		return right_idx;
	}

        mid_pointer_iterator_struct reset_midstates_iterator(my_uint i, my_uint j, int typ, bool do_not_use_constrain_bag = false)
	{
		mid_pointer_iterator_struct s;

	        if ((constrain_bag == NULL) || (do_not_use_constrain_bag)) {
			s.is_constrained = false;

			s.constrained_points[0] = i;
			s.constrained_points[1] = j;

        	        if (typ == abstract_lattice<E>::midstate_iterator_no_end_inclusive) {
				s.mid_point = 0;
				s.last_mid_point = midPointsForward[i][j].size();
			} else if (typ == abstract_lattice<E>::midstate_iterator_first_inclusive) {
                        	s.mid_point = -1;
                        	s.last_mid_point = midPointsForward[i][j].size();
                	} else if (typ == abstract_lattice<E>::midstate_iterator_last_inclusive) {
                        	s.mid_point = 0;
                        	s.last_mid_point = midPointsForward[i][j].size() + 1;
                	} else if (typ == abstract_lattice<E>::midstate_iterator_both_inclusive) {
                	        s.mid_point = -1;
                	        s.last_mid_point = midPointsForward[i][j].size() + 1;
                	} else {
                	        assert(false);
                	}
        	} else {
                	s.is_constrained = true;

                	if (typ == abstract_lattice<E>::midstate_iterator_no_end_inclusive) {
                        	s.constrained_points[0] = constrain_bag->mid_point(i, j);
                        	s.last_mid_point = 1;
                	} else if (typ == abstract_lattice<E>::midstate_iterator_first_inclusive) {
                        	s.constrained_points[1] = i;
                        	s.constrained_points[0] = constrain_bag->mid_point(i, j);

                        	s.last_mid_point = 2;
                	} else if (typ == abstract_lattice<E>::midstate_iterator_last_inclusive) {
                	        s.constrained_points[1] = constrain_bag->mid_point(i, j);
                	        s.constrained_points[0] = j;

                	        s.last_mid_point = 2;
                	} else if (typ == abstract_lattice<E>::midstate_iterator_both_inclusive) {
                	        s.constrained_points[2] = i;
                	        s.constrained_points[1] = constrain_bag->mid_point(i, j);
                	        s.constrained_points[0] = j;

                	        s.last_mid_point = 3;
                	} else {
                	        assert(false);
                	}
        	}

        	return s;
	}

	bool next_midstate(mid_pointer_iterator_struct& num)
	{
		if (num.is_constrained) {
        	        num.last_mid_point--;

        	        if (num.last_mid_point < 0) {
        	                return false;
        	        }

        	        return true;
        	} else {
        	        num.mid_point++;

			int i = num.constrained_points[0];
			int j = num.constrained_points[1];

			if (i == j) { return false; }

			if (num.mid_point == 0) {
				num.constrained_points[2] = num.constrained_points[0];
			} else if ((num.mid_point == num.last_mid_point) && (midPointsForward[i][j].size() < num.last_mid_point)) {
				num.constrained_points[2] = num.constrained_points[1];
			} else if (num.mid_point > num.last_mid_point) {
				return false;
			} else {
				num.constrained_points[2] = midPointsForward[i][j][num.mid_point-1];
			}

			return true;
        	}
	}

	my_uint mid_state(mid_pointer_iterator_struct& num)
	{
        	if (num.is_constrained) {
                	return num.constrained_points[num.last_mid_point];
        	} else {
                	return num.constrained_points[2];
        	}
	}

	void set_span_bag(cnf_span_bag* bag) {
		this->constrain_bag = bag;
	}

        bool is_root_span(int i, int j) {
		return ((i == 0) && (j == numStates - 1));
	}

	void reset_incoming_states(
        	        my_uint target_state) {
        	iterating_incoming_state = true;
        	this->current_incoming_state = target_state;
		this->target_incoming_state = target_state;

        	if (target_state == 0) {
        	        iterating_incoming_state = false;
        	}
	}


	bool next_incoming_state() {
		this->current_incoming_state--;

		if (this->current_incoming_state < 0) { return false; }

		if (hasSpan[this->current_incoming_state][this->target_incoming_state]) {
			return true;
		} else {
			return next_incoming_state();
		}
	}

	my_uint incoming_state() {
	        return this->current_incoming_state;
	}


        my_uint first_state() const {
                return 0;
        }

        my_uint last_state() const {
                return numStates-1;
        }

        bool has_span_length_1(my_uint i, my_uint j)
	{
		return hasSpan[i][j];
	}

	bool has_span_length_1plus(my_uint i, my_uint j)
	{
		return ((midPointsForward[i][j].size() > 0) || (!hasSpan[i][j]));
	}
};



class FullLatticeString : public FullLattice<vocab_symbol>
{
public:
	array_2d<inpre_symbol> tags;
	grammar_alphabet<inpre_symbol, vocab_symbol>* alphabet;

	FullLatticeString(grammar_alphabet<inpre_symbol, vocab_symbol>* g_alphabet, string str, cnf_span_bag* bag, bool rightBranch):FullLattice(bag)
	{
		this->alphabet = g_alphabet;

		fromString(str, rightBranch);
	}

	void fromString(string latticeString, bool rightBranch = false)
	{
		cerr<<"Parsing lattice: "<<latticeString<<endl;

		vector<string> tokens;

		string_utils::split(tokens, latticeString, " ");

		assert(tokens[0] == "LPCFGLatticeMarker");

		numStates = stoi(tokens[1]);

		vocab_symbol* symbolsPtr = new vocab_symbol[numStates*numStates];
		double* weightsPtr = new double[numStates*numStates];
		inpre_symbol* tagsPtr = new inpre_symbol[numStates*numStates];
		vector<int>* setsPtr = new vector<int>[numStates*numStates];
		bool* hasSpanPtr = new bool[numStates*numStates];

		symbols = array_2d<vocab_symbol>(symbolsPtr, numStates, numStates);
		weights = array_2d<double>(weightsPtr, numStates, numStates);
		tags = array_2d<inpre_symbol>(tagsPtr, numStates, numStates);
		midPointsForward = array_2d<vector< int > >(setsPtr, numStates, numStates);
		hasSpan = array_2d<bool>(hasSpanPtr, numStates, numStates);

		for (int i=0; i < numStates; i++) {
			for (int j=0; j < numStates; j++) {
				hasSpan[i][j] = false;
			}
		}

		for (int i=2; i < tokens.size(); i++)
		{
			vector<string> stateTokens;

			string_utils::split(stateTokens, tokens[i], "^");

			int leftPoint = stoi(stateTokens[0]);
			int rightPoint = stoi(stateTokens[1]);
			vocab_symbol symbol = alphabet->vocab_lookup_string(stateTokens[2]);
			inpre_symbol tag = alphabet->preterminal_lookup_string(stateTokens[3]);
			double weight = 1.0;
			if (stateTokens.size() >= 5) {
				weight = stod(stateTokens[4]);
			}


			symbols[leftPoint][rightPoint] = symbol;
			weights[leftPoint][rightPoint] = weight;
			tags[leftPoint][rightPoint] = tag;

			hasSpan[leftPoint][rightPoint] = true;

			if (stateTokens.size() >= 6) {
				string midPointsStr = stateTokens[5];

				vector<string> midPointTokens;

				string_utils::split(midPointTokens, midPointsStr, ",");
				for (int j=0; j < midPointTokens.size(); j++)
				{
					midPointsForward[leftPoint][rightPoint].push_back(stoi(midPointTokens[j]));
				}

				/*for (int j=midPointTokens.size()-1; j >= 0; j--)
				{
					midPointsBackward[leftPoint][rightPoint].push_back(stoi(midPointTokens[j]));
				}*/
			}
		}

		cerr<<"Computing closure."<<endl;
		computeClosure(0, numStates-1, rightBranch, numStates-1);
		cerr<<"Done parsing lattice string."<<endl;
	}

	void destroy_all()
	{
		FullLattice<vocab_symbol>::destroy_all();
		tags.destroy();
	}
};

#endif
