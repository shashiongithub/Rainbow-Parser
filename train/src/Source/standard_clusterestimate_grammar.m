% Uses training_file_withclusters, obj_hashmap_nt_clusterinfo and grammar_file

% Introduce Local Directory
cluster_dir = char(obj_globalparam.clusterDirectory);

% Creating Training and Grammar directory
finaloutput_dir = cell2mat({char(obj_globalparam.outputDirectory),'/StandardClusterEstimate-',char(obj_globalparam.clusteringMethod)});
if strcmp(obj_globalparam.clusteringMethod, 'kmeans-fuzzy') || strcmp(obj_globalparam.clusteringMethod, 'cmeans')
    finaloutput_dir = cell2mat({finaloutput_dir,'-',num2str(obj_globalparam.fuzziness)});
end
obj_globalparam.trainWithClusterDirectory = finaloutput_dir;
obj_globalparam.grammarDirectory = finaloutput_dir;

fprintf('Creating Training and Grammar directory: %s \n',finaloutput_dir);
command = cell2mat({'mkdir -p ', finaloutput_dir});
shelloutput = system(command);

% Update Cluster Info in the CorpusData (only for those nonterminals which are introduced first time, or needed to be updated for currentnode)
obj_hashmap_keyset_iterator = getIteratorHashMapKeyset(obj_matlabinterface, obj_hashmap_nt_clusterinfo);
while obj_hashmap_keyset_iterator.hasNext()
    ntid = obj_hashmap_keyset_iterator.next();
    clustercount = getValueFromHashMap(obj_matlabinterface, obj_hashmap_nt_clusterinfo, ntid);
    nonterminal = cell2mat(nonterminals_info(ntid, 1));

    % Load Cluster Data and Update Queue
    ntcluster_to_load  = nonterminal;
    ntcluster_to_load_withls = clustercount;
    load_clusterdata_update_corpusdata;
end

% Start updating trees with cluster information
obj_globalparam.corpusParseTreesWithClusters = cell2mat({finaloutput_dir, '/',char(training_file_withclusters)});
updateAndWriteNTWithClusters(obj_corpusdata);

% Reset LPCFG Grammar Object
resetObject(obj_lpcfggrammar);

% Setting Local Variables and estimating the grammar file
obj_globalparam.corpusParseTreesWithClusters = cell2mat({finaloutput_dir, '/',char(training_file_withclusters)});
obj_globalparam.LPCFGGrammarFile = cell2mat({finaloutput_dir, '/',char(grammar_file)});
estimateAndWriteLPCFG(obj_lpcfggrammar);

