
% Uses obj_hashmap_nt_clusterinfo and grammar_file

% data_lpcfg_file = cell2mat({Tuning_Spectral_Dir,'/data-lpcfg'});
% projections_file = cell2mat({Tuning_Spectral_Dir,'/projections'});
Estimates_Directory = cell2mat({Tuning_Spectral_Dir,'/Estimates-',char(grammar_file),'/'});
command = cell2mat({'mkdir -p ', Estimates_Directory});
shelloutput = system(command);

% Extract Rule details 
fprintf('Extract Rule details\n'); 
spectral_extract_rule_details
% load(data_lpcfg_file, 'nonterminals', 'unaryrules', 'binaryrules', 'total_root');

% Final Projections File
fprintf('Final Projections calculations\n'); 
spectral_projections_nonterminals

% Estimating Smoothing Parameters
fprintf('Estimating Smoothing Parameters\n'); 
spectral_estimating_smoothing_parameters

% Estimate and Write LPCFG - Smoothed
fprintf('Estimating and Writing Smoothed Grammar\n'); 
LPCFG_GRAMMAR_file = cell2mat({Tuning_Spectral_Dir, '/',char(grammar_file)});
obj_globalparam.LPCFGGrammarFile = LPCFG_GRAMMAR_file; 
spectral_estimate_lpcfg
