%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Matlab code to run cluster style estimation of an L-PCFG parser, as described in
% Narayan and Cohen (2015). See README.md for more details.
%

% Start Fresh
clear

% Set the source folder
userpath('/disk/ocean/snarayan/dist/lpcfgparser/train/src/Source');

% Set Global Parameters and Objects
% See set_global_parameters.m for available options
set_global_parameters

% If you already ran the next three steps (preprocessing, inside_outside and svd) and the relevant
% files are in the output directory, you can skip these steps and jump directly to estimation.

% Preprocessing steps
preprocessing_steps

% Inside Outside 
inside_outside

% SVD
svd_nonterminals_parallel

% Start estimating the grammar [two options: default or grammar]

% Latent states using default cutoff 
clusterestimate_default_cutoff

% Latent state using some grammar
%grammarfile = '/gpfs/scratch/snarayan/Spectral-Tuning-Experiments/Swedish-SPMRL/swe_naacl-head_rare-19_t-pos_nt-pos_casing-true/Result-FirstStage-Tuning/Grammars-TrainWithClusters-kmeans/grammar-98411acd-0c72-4c2d-8d6d-8072fe5a99cc.txt';
%clusterestimate_with_grammar

% Close diary
diary off;
