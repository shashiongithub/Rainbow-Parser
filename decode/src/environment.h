#ifndef ENVIRONMENT_H_
#define ENVIRONMENT_H_

// contains an environment with all variable settings, alphabets, etc.

class semiring_environment
{
private:
	typedef max_log_semiring<cnf_double_backpointer_pair<inpre_symbol, vocab_symbol> > viterbi_semiring;
	
	real_semiring global_real_semiring;
	viterbi_semiring global_viterbi_semiring;
	tensor_semiring global_tensor_semiring;
	
public:
	real_semiring* g_real_semiring()
	{
		return &global_real_semiring;
	}
	
	viterbi_semiring* g_viterbi_semiring()
	{
		return &global_viterbi_semiring;
	}
	
	tensor_semiring* g_tensor_semiring()
	{
		return &global_tensor_semiring;
	}
};

class environment
{
private:
	cnf_grammar_alphabet alphabet;
	semiring_environment* sr_env;
	
public:
	environment(semiring_environment* sr)
	{
		sr_env = sr;
	}
	
	cnf_grammar_alphabet* env_alphabet()
	{
		return &alphabet;
	}
	
	semiring_environment* env_semiring()
	{
		return sr_env;
	}
};

#endif /*ENVIRONMENT_H_*/
