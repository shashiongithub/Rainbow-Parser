
fprintf('Starting spectral estimation of the grammar with an available grammar.\n');

% Get the latent states for different nonterminals 

% Get frequency of all nonterminals
obj_hashmap_ntid_freq = getHashMapDoubleDouble(obj_matlabinterface);
for count = 1:size(nonterminals_info,1)
    nonterminal = cell2mat(nonterminals_info(count, 1));
    frequency = cell2mat(nonterminals_info(count, 2));
    % Add to the HashMap
    obj_hashmap_ntid_freq.put(count, frequency);
end
% printObject(obj_matlabinterface, obj_hashmap_ntid_freq);

% Get id of all nonterminals
obj_hashmap_ntstr_ntid = getHashMapStringDouble(obj_matlabinterface);
for count = 1:size(nonterminals_info,1)
    nonterminal = cell2mat(nonterminals_info(count, 1));
    frequency = cell2mat(nonterminals_info(count, 2));
    % Add to the HashMap
    obj_hashmap_ntstr_ntid.put(java.lang.String(nonterminal), count);
end
% printObject(obj_matlabinterface, obj_hashmap_ntstr_ntid);


% ========== Initialize Parallel Pool 
parobject = parpool(Parallel_Pool_Size);

% ========== Initialize A Graph Node with default latent states =====================
obj_tuningbeamsearch = javaObjectEDT('lpcfgsmoothclusterestimationuniversal.TuningBeamSearch', obj_hashmap_ntid_freq);
% Create a node in the middle with a grammar file
fprintf('Starting with the grammar: %s\n', grammarfile);
obj_startgraphnode = createStartNodeInMiddle(obj_tuningbeamsearch, grammarfile, 1, obj_hashmap_ntstr_ntid);
setInComplete(obj_startgraphnode);
      
% Create Unique Id and File names
createUniqueId(obj_startgraphnode);
% Create the training file with clusters and estimate grammars
training_file_withclusters = getTrainfilewithclusters(obj_startgraphnode);
obj_hashmap_nt_clusterinfo = getHashMapNtIdtoLS(obj_startgraphnode);
grammar_file = getGrammarfile(obj_startgraphnode);
standard_spectralestimate_grammar
            
% ========== Delete Parallel Pool
delete(parobject);
