% Uses obj_hashmap_nt_clusterinfo and grammar_file

% Introduce Local Directory
cluster_dir = char(obj_globalparam.clusterDirectory);

% Creating Training and Grammar directory
finaloutput_dir = cell2mat({char(obj_globalparam.outputDirectory),'/StandardSpectralEstimate'});
fprintf('Creating Spectral directory: %s \n',finaloutput_dir);
command = cell2mat({'mkdir -p ', finaloutput_dir});
shelloutput = system(command);

data_lpcfg_file = cell2mat({finaloutput_dir,'/data-lpcfg'});
projections_file = cell2mat({finaloutput_dir,'/projections'});
Estimates_Directory = cell2mat({finaloutput_dir,'/Estimates/'});
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
LPCFG_GRAMMAR_file = cell2mat({finaloutput_dir, '/',char(grammar_file)});
spectral_estimate_lpcfg

