
fprintf('Starting spectral estimation of a grammar with default cutoff for latent states.\n');

% Get frequency of all nonterminals 
obj_hashmap_ntid_freq = getHashMapDoubleDouble(obj_matlabinterface);
for count = 1:size(nonterminals_info,1)
    nonterminal = cell2mat(nonterminals_info(count, 1));
    frequency = cell2mat(nonterminals_info(count, 2));
    % Add to the HashMap
    obj_hashmap_ntid_freq.put(count, frequency);
end
% printObject(obj_matlabinterface, obj_hashmap_ntid_freq);

% ========== Initialize Parallel Pool 
parobject = parpool(Parallel_Pool_Size);

% ========== Initialize A Graph Node with default latent states =====================
obj_tuningbeamsearch = javaObjectEDT('lpcfgsmoothclusterestimationuniversal.TuningBeamSearch', obj_hashmap_ntid_freq);
% Create the start Node in the problem graph
obj_startgraphnode = createStartNode(obj_tuningbeamsearch);

% Create Unique Id and File names
createUniqueId(obj_startgraphnode);
% Create the training file with clusters and estimate grammars
training_file_withclusters = getTrainfilewithclusters(obj_startgraphnode);
obj_hashmap_nt_clusterinfo = getHashMapNtIdtoLS(obj_startgraphnode);
grammar_file = getGrammarfile(obj_startgraphnode);
standard_spectralestimate_grammar
            
% ========== Delete Parallel Pool
delete(parobject);
