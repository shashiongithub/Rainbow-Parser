
% Uses training_file_withclusters, obj_hashmap_nt_clusterinfo and obj_stageone_graphchildnode 

% Introduce Local Directory
cluster_dir = char(obj_globalparam.clusterDirectory);

% Creating Training with Clusters Information
trainwithcl_dir = cell2mat({char(obj_globalparam.outputDirectory),'/TrainWithClusters-',char(obj_globalparam.clusteringMethod)});
if strcmp(obj_globalparam.clusteringMethod, 'kmeans-fuzzy') || strcmp(obj_globalparam.clusteringMethod, 'cmeans')
    trainwithcl_dir = cell2mat({trainwithcl_dir,'-',num2str(obj_globalparam.fuzziness)});
end
obj_globalparam.trainWithClusterDirectory = trainwithcl_dir;
fprintf('Creating Training with Clusters directory: %s \n',trainwithcl_dir);
command = cell2mat({'mkdir -p ', trainwithcl_dir});
shelloutput = system(command);

% Update Cluster Info in the CorpusData (only for those nonterminals which are introduced first time, or needed to be updated for currentnode)
obj_hashmap_keyset_iterator = getIteratorHashMapKeyset(obj_matlabinterface, obj_hashmap_nt_clusterinfo);
while obj_hashmap_keyset_iterator.hasNext()
    ntid = obj_hashmap_keyset_iterator.next();
    clustercount = getValueFromHashMap(obj_matlabinterface, obj_hashmap_nt_clusterinfo, ntid);
    % clustercount = getLSForNTId(obj_stageone_graphchildnode, ntid);
    nonterminal = cell2mat(nonterminals_info(ntid, 1));
    % frequency = cell2mat(nonterminals_info(ntid, 2));
    
    % Check if cluster update needed
    isupdateclusterinfo = 0;
    if ntid == currentNtId
        isupdateclusterinfo = 1;
    else
        if ~ isNonterminalInClusterInfo(obj_corpusdata, nonterminal)
            isupdateclusterinfo = 1;
        end
    end
    
    if isupdateclusterinfo
        % Load Cluster Data and Update Queue
        ntcluster_to_load  = nonterminal;
        ntcluster_to_load_withls = clustercount;
        load_clusterdata_update_corpusdata;
    end
end

% Start updating trees with cluster information
obj_globalparam.corpusParseTreesWithClusters = cell2mat({trainwithcl_dir, '/',char(training_file_withclusters)});
updateAndWriteNTWithClusters(obj_corpusdata);
