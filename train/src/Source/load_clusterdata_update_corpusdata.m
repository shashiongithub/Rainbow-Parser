
% Creating Cluster Directory
cluster_dir = cell2mat({char(obj_globalparam.outputDirectory),'/Clusters-',char(obj_globalparam.clusteringMethod)});
if strcmp(obj_globalparam.clusteringMethod, 'kmeans-fuzzy') || strcmp(obj_globalparam.clusteringMethod, 'cmeans')
    cluster_dir = cell2mat({cluster_dir,'-',num2str(obj_globalparam.fuzziness)});
end
obj_globalparam.clusterDirectory = cluster_dir;

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