%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Parameters file for spectral/cluster estimation of L-PCFGs. See README.md.
% By Shashi Narayan and Shay Cohen.

% % ========== Dynamic path Setup ============
% javaaddpath('lpcfg-smoothclusterestimation-universal.jar');

% ========== Parallel Pool Size ======================
% This is the number of parallel threads that are used, one per nonterminal.
% Used with caution, as a large number of parallel threads can be a burden
% on the machine. See README.md on instructions on how to cancel the use of
% multithreading if you detect problems with it (happens on some machines).
Parallel_Pool_Size = 10;

% ========== Intialize GlobalParameters Object =============
obj_globalparam = lpcfgsmoothclusterestimationuniversal.GlobalParameters;

% ========== Language : Training Data ================
% Can be one of:
% unknown, eng-ptb, german-negra, swe-spmrl, pol-spmrl, bas-spmrl, fre-spmrl, ger-spmrl, heb-spmrl, hun-spmrl, kor-spmrl
% If not using for the goal of syntactic parsing, use unknown. The rest are markers appropriate for specific treebanks.
% See Narayan and Cohen (2016).
% The difference is mainly in the way the data is loaded and processed.
obj_globalparam.languageAndTrainingData = java.lang.String('eng-ptb');

% The treebank used for training in Penn Treebank format.
% Each tree also has to have a top level TOP.
% For example: (TOP (S (NP (DT The) (NN dog)) (VP (VBD barked))))
obj_globalparam.corpusOriginal = '/disk/ocean/snarayan/dist/lpcfgparser/example-data/ptb-example-train.txt';

% Is Casing Preserved
% Do you want to lower case the vocabulary (0) or not (1)? Note that
% if you lowercase the vocabulary, you also have to feed into the parser
% lowercased sentences.
% Set to 1 by default, or if unsure.
obj_globalparam.isPreserveCasing = 1; % 1 for true and 0 for false

% Features used and their cutoffs (naacl refers to features in Cohen et al. (2013) in NAACL)
% -head means without head features, +morph means with morphological features
obj_globalparam.featuresUsed = 'naacl'; % naacl-head, naacl, naacl+morph
obj_globalparam.frequencyCutoffNAACL = 0;
	
% Rare word Cutoff 
% Any word that appears less than that number in the training data will be replaced
% with its part-of-speech tag. Use 0 if you do not want to use a rare word cutoff.
% Note that this word cutoff needs to be used in conjunction with lpcfgparser.py
% as vocab-threshold for the grammar and vocabulary file you create.
obj_globalparam.rarewordCutoff = 5; % (<= x)

% Rare word and preterminal handling
% See Narayan and Cohen (2016)
obj_globalparam.rarewordHandling = 'pos';	% pos, morph, pos+morph
obj_globalparam.preterminalHandling = 'pos'; % pos,  pos+morph 

% Tuning Setup
% For the k-means "spectral" algorithm of Narayan and Cohen (2015)
obj_globalparam.clusteringMethod = 'kmeans'; % kmeans, kmeans-fuzzy, cmeans
obj_globalparam.fuzziness = 3; % Proportional to 1/d^n (kmeans-fuzzy), 1.1 for cmeans, NOT used for kmeans

% Used for hyperparameter tuning (number of latent states per nonterminal), but is not necessary in most cases if vanilla
% training is used without a search for the optimal number of latent states as described in
% Narayan and Cohen (2016)
% This needs to be in the same treebank format as the training set.
% There are two options: either to use a gold stadard development set or one with hypothesized tags.
obj_globalparam.developmentTaggedCorpus = '';
obj_globalparam.developmentGoldCorpus = '';
obj_globalparam.beamSize = 10;

% These commands are only used if tuning the number of latent states as mentioned in Narayan and Cohen (2016)
% Parsing command
obj_globalparam.parsingCommand = '/gpfs/scratch/snarayan/tools/parsing-evaluation/parse_shashi_grammar_enftagfalse.pl';
% Evaluation command 
obj_globalparam.evaluationCommand = '/gpfs/scratch/snarayan/tools/parsing-evaluation/eval';



% This is the number of latent states used by the spectral/clustering algorithm (an upper bound,
% the actual number can be smaller depending on the rank of the cross-covariance matrix for each
% nonterminal).
% If using the latent state optimization algorithm, this is the starting number of latent states
% for the search. The final number of latent states can be different.
obj_globalparam.predictedStartClusters = 4; % Put greater than 2 

% If you want, you can use a larger number for mSVD, and then calculate the SVD matrices
% just once. Then you can use them multiple times with different number of latent states.
% If you are using the optimization search algorithm, it is suggested that you use a fixed
% larger number than predictedStartClusters.
obj_globalparam.mSVD = obj_globalparam.predictedStartClusters;

% obj_globalparam.mSVD = 100;


% Output files
% Specify where to output the prune grammar, the grammar and the vocabulary file.
% Also creates intermediate structures, such as the result of the SVD running on the training data.
% Usually there is no need to change more than Output_Super_Directory.
% The directive below it creates a special signature directory according to the hyperparameters set
% in this file.
Output_Super_Directory = '/disk/ocean/snarayan/dist/lpcfgparser/example-output/';
obj_globalparam.outputDirectory = cell2mat({Output_Super_Directory,'spectral_',char(obj_globalparam.featuresUsed),'_feat-',num2str(obj_globalparam.frequencyCutoffNAACL),'_rare-',num2str(obj_globalparam.rarewordCutoff),'_t-',char(obj_globalparam.rarewordHandling),'_nt-',char(obj_globalparam.preterminalHandling)});

% printObject(obj_matlabinterface,obj_globalparam.outputDirectory);

% Change with caution below here

% ========== Initialize MatlabInterface object ===================
obj_matlabinterface = lpcfgsmoothclusterestimationuniversal.MatlabInterface;

% ========== Initialize Feature Dictionaries ======================
obj_naaclfeaturedict = lpcfgsmoothclusterestimationuniversal.NAACLFeaturesDictionary;
% Set Global Feature Generator
obj_globalparam.featuregenerator = getClusterSpectralFeatureGenerator(obj_matlabinterface, obj_naaclfeaturedict);

% ========== Initialize CorpusData Object =====================
obj_corpusdata = lpcfgsmoothclusterestimationuniversal.CorpusData;

% ========== Initialize LPCFGGrammar Object =====================
obj_lpcfggrammar = lpcfgsmoothclusterestimationuniversal.LPCFGGrammar;
