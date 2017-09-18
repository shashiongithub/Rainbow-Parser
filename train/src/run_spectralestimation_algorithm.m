%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Matlab code to run cluster style estimation of an L-PCFG parser, as described in
% Cohen et al. (2013). See README.md for more details.
%

% Start Fresh
clear

% Set the source folder
userpath('/disk/ocean/snarayan/dist/lpcfgparser/train/src/Source');

% Set Global Parameters and Objects
set_global_parameters

% Other smoothing parameters
% See parameters description in Cohen et al. (2013)
% if isGrammarSmoothed == 0, then no smoothing is used, and the algorithm is essentially the vanilla algorithm of Cohen et al. (2014).
isGrammarSmoothed = 1;
parameter_c = 30;
parameter_v = 0.6;
interpolation_cutoff = 1000;

% If you already ran the next three steps (preprocessing, inside_outside and svd) and the relevant
% files are in the output directory, you can skip these steps and jump directly to estimation.

% Preprocessing steps
preprocessing_steps

% Inside Outside 
inside_outside

% SVD
svd_nonterminals_parallel

% Start estimating the grammar [two options: default or grammar]

% Latent state using a default cutoff
spectralestimate_default_cutoff

% Latent state using some grammar
% grammarfile = '/gpfs/scratch/snarayan/Spectral-Tuning-Experiments/English-PTB/eng_naacl_rare-25_t-pos_nt-pos/ACL-Grammars/grammar-4d1ed69c-ee32-434f-8dea-47e5b244bda2.txt-before';
% spectralestimate_with_grammar

% Close diary
diary off;
