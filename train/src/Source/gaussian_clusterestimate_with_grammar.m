fprintf('Starting estimating the grammar with an available grammar for latent states.\n');

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

% ========== Initialize A Graph Node with default latent states =====================
obj_tuningbeamsearch = javaObjectEDT('lpcfgsmoothclusterestimationuniversal.TuningBeamSearch', obj_hashmap_ntid_freq);
% Create a node in the middle with a grammar file
fprintf('Starting with the grammar: %s\n', grammarfile);
obj_startgraphnode = createStartNodeInMiddle(obj_tuningbeamsearch, grammarfile, 1, obj_hashmap_ntstr_ntid);
setInComplete(obj_startgraphnode);

% Reset Cluster Info in the CorpusData
resetNonterminalsClusterInfo(obj_corpusdata);
    
% Extract all clusters to be created
stageone_clusters = cell(1,1);
clcount = 1;
for count = 1:size(nonterminals_info,1)
    stageone_clusters{clcount,1} = count;
    stageone_clusters{clcount,2} = getLSForNTId(obj_startgraphnode, count);
    clcount = clcount + 1;
end
        
% Calculate Clustering 
fprintf('Clustering\n');
clusterstocalculate = stageone_clusters;
gaussian_clustering_tuning_parallel
      
% Create the training file with clusters and estimate grammars
training_file_withclusters = 'trainingfile-withclusters.txt';
obj_hashmap_nt_clusterinfo = getHashMapNtIdtoLS(obj_startgraphnode);
grammar_file = 'grammar.txt';
noisy_clusterestimate_grammar
            
