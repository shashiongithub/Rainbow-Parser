% Uses training_file_withclusters, obj_hashmap_nt_clusterinfo and grammar_file

% Introduce Local Directory
cluster_dir = char(obj_globalparam.clusterDirectory);

% Creating Training and Grammar directory
finaloutput_dir = Parent_Cluster_Directory;
obj_globalparam.trainWithClusterDirectory = finaloutput_dir;
obj_globalparam.grammarDirectory = finaloutput_dir;

% Update Cluster Info in the CorpusData (only for those nonterminals which are introduced first time, or needed to be updated for currentnode)
obj_hashmap_keyset_iterator = getIteratorHashMapKeyset(obj_matlabinterface, obj_hashmap_nt_clusterinfo);
while obj_hashmap_keyset_iterator.hasNext()
    ntid = obj_hashmap_keyset_iterator.next();
    clustercount = getValueFromHashMap(obj_matlabinterface, obj_hashmap_nt_clusterinfo, ntid);
    nonterminal = cell2mat(nonterminals_info(ntid, 1));

    % Load Cluster Data and Update Queue
    ntcluster_to_load  = nonterminal;
    ntcluster_to_load_withls = clustercount;
        
    fprintf('Loading Cluster file and Update Queue: Clusters-%s-%s.mat\n',ntcluster_to_load,int2str(ntcluster_to_load_withls));
    load(cell2mat({cluster_dir, '/Clusters-', ntcluster_to_load,'-', int2str(ntcluster_to_load_withls),'.mat'}), 'fuzzy_U');
    
    % Create a queue
    obj_queue_arraydouble = getQueueOfDoubleArray(obj_matlabinterface);
    for ntcount = 1:size(fuzzy_U,2)
        % Create a double array
        obj_doublearray = getDoubleArray(obj_matlabinterface);
        
        for clusterid = 1:ntcluster_to_load_withls
            obj_doublearray.add(fuzzy_U(clusterid, ntcount));
        end
        
        obj_queue_arraydouble.add(obj_doublearray);
    end
        
    updateNonterminalsClusterInfo(obj_corpusdata, ntcluster_to_load, obj_queue_arraydouble);
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

