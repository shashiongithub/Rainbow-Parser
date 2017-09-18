
% Uses clusterstocalculate

% Introduce Local Directory
inside_outside_dir = cell2mat({char(obj_globalparam.outputDirectory),'/Inside-Outside'});
svd_dir = cell2mat({char(obj_globalparam.outputDirectory),'/SVD'});

% Creating Cluster Directory
cluster_dir = cell2mat({char(obj_globalparam.outputDirectory),'/Clusters-',char(obj_globalparam.clusteringMethod)});
if strcmp(obj_globalparam.clusteringMethod, 'kmeans-fuzzy') || strcmp(obj_globalparam.clusteringMethod, 'cmeans')
    cluster_dir = cell2mat({cluster_dir,'-',num2str(obj_globalparam.fuzziness)});
end
obj_globalparam.clusterDirectory = cluster_dir;
fprintf('Creating Cluster directory: %s \n',cluster_dir);
command = cell2mat({'mkdir -p ', cluster_dir});
shelloutput = system(command);

% ========== Intialize matlab variables from Java variables ==========
eigenvaluecutoff = obj_globalparam.eigenValueCutoff;
clusteringMethod = obj_globalparam.clusteringMethod;
% Used for kmeans and kmeans-fuzzy only
replicateclustering = obj_globalparam.replicateClustering;
% used for kmeans-fuzzy and cmeans only
fuzziness = obj_globalparam.fuzziness;
% used for kmeans and cmeans
maximumiteration = obj_globalparam.maxClusterIteration;


parfor linecount = 1:size(clusterstocalculate, 1)
    ntid = cell2mat(clusterstocalculate(linecount, 1));
    clustercount = cell2mat(clusterstocalculate(linecount, 2));
    
    nonterminal = cell2mat(nonterminals_info(ntid, 1));
    frequency = cell2mat(nonterminals_info(ntid, 2));
    mSVD = cell2mat(nonterminals_info(ntid, 3));
   
    if ~exist(cell2mat({cluster_dir, '/Clusters-', nonterminal,'-', int2str(clustercount),'.mat'}), 'file')
        
        fprintf('%s : freq=%d : cl=%d : msvd=%d\n',nonterminal,frequency,clustercount,mSVD);
            
        % Creating Clock
        nt_clock = tic;
        
        if clustercount == 1
            fuzzy_center = [];
            fuzzy_U = ones(1, frequency);
            fuzzy_obj_fcn = [];
            
            fprintf('Save %s results: [%s]\n',char(clusteringMethod), nonterminal);
            save_fuzzy_cmeans_results(cell2mat({cluster_dir, '/Clusters-', nonterminal,'-', int2str(clustercount),'.mat'}), fuzzy_center, fuzzy_U, fuzzy_obj_fcn, clustercount);
        else
            fprintf('Loading inside and outside matrices: %s\n',nonterminal);
            insidematrix = load(cell2mat({inside_outside_dir, '/I-', nonterminal, '.mat'}), cell2mat({'ntI',int2str(ntid)}));
            outsidematrix = load(cell2mat({inside_outside_dir, '/O-', nonterminal, '.mat'}), cell2mat({'ntO',int2str(ntid)}));
        
            fprintf('Loading SVDs S, U and V  matrices: %s\n',nonterminal);
            svdmatrix = load(cell2mat({svd_dir, '/SVD-', nonterminal, '.mat'}),'U', 'S', 'V');

            fprintf('Select meaningful eigen values (> %f): %s\n',eigenvaluecutoff,nonterminal);
            diagonal = diag(svdmatrix.('S'));
            diagonalfiltered = diagonal(diagonal > eigenvaluecutoff);
            validsize = size(diagonalfiltered, 1);
            fprintf('Before : %d and After : %d : %s\n',size(diagonal, 1),validsize,nonterminal);
            
            % Modify U and V
            svdmatrix.('U') = svdmatrix.('U')(:,1:validsize);
            svdmatrix.('V') = svdmatrix.('V')(:,1:validsize);
            
            fprintf('Dimension reduction: Insidematrix and OutsideMatrix : %s\n',nonterminal);
            insidematrix_reduced = (svdmatrix.('U').')*insidematrix.(cell2mat({'ntI',int2str(ntid)}));
            %clearvars U insidematrix
            outsidematrix_reduced = (svdmatrix.('V').')*outsidematrix.(cell2mat({'ntO',int2str(ntid)}));
            %clearvars V outsidematrix
            final_features = [insidematrix_reduced;outsidematrix_reduced];
            %clearvars insidematrix_reduced outsidematrix_reduced
          
            % Cluster Results
            fuzzy_center = [];
            fuzzy_U = [];
            fuzzy_obj_fcn = [];
            
            if strcmp(clusteringMethod, 'kmeans')
                fprintf('Kmeans Clustering : %s : %d : MaxIter=%d : Replicates:%d : Clusters:%d\n',nonterminal,frequency,maximumiteration,replicateclustering,clustercount);
                final_clusters = kmeans((final_features.'), clustercount,'MaxIter',maximumiteration,'Replicates',replicateclustering,'Options',statset('UseParallel',1),'start','sample');
                
                fuzzy_U = zeros(clustercount, frequency);
                for instanceidx = 1:size(final_clusters,1)
                    fuzzy_U(final_clusters(instanceidx, 1), instanceidx) = 1.0;
                end
            end
            
            if strcmp(clusteringMethod, 'kmeans-fuzzy')
                fprintf('Kmeans-fuzzy Clustering : %s : %d : MaxIter=%d : Replicates:%d : Clusters:%d\n',nonterminal,frequency,maximumiteration,replicateclustering,clustercount);
                [final_clusters, fuzzy_center, sumd, fuzzy_distance] = kmeans((final_features.'), clustercount,'MaxIter',maximumiteration,'Replicates',replicateclustering,'Options',statset('UseParallel',1),'start','sample');
                
                fuzzy_U = zeros(clustercount, frequency);
                for instanceidx = 1:size(fuzzy_distance, 1)
                    harmonic_sum = 0;
                    for cidx = 1:clustercount
                        harmonic_sum = harmonic_sum + (1.0 / (fuzzy_distance(instanceidx, cidx)^fuzziness));
                    end
                    harmonic_factor = 1.0 / harmonic_sum;
                    
                    for cidx = 1:clustercount
                        fuzzy_U(cidx, instanceidx) = harmonic_factor / (fuzzy_distance(instanceidx, cidx)^fuzziness);
                    end
                end
            end
            
            if strcmp(clusteringMethod, 'cmeans')
                fprintf('Fuzzy C-means Clustering : %s : %d : MaxIter=%d : Clusters:%d\n',nonterminal,frequency,maximumiteration,clustercount);
                fuzzy_options = [fuzziness;maximumiteration;nan;0];
                [fuzzy_center,fuzzy_U,fuzzy_obj_fcn] = fcm((final_features.'),clustercount,fuzzy_options);
            end

            fprintf('Save %s results: [%s]\n',char(clusteringMethod), nonterminal);
            save_fuzzy_cmeans_results(cell2mat({cluster_dir, '/Clusters-', nonterminal,'-', int2str(clustercount),'.mat'}), fuzzy_center, fuzzy_U, fuzzy_obj_fcn, clustercount);
        end
        
        % Stop the clock
        fprintf('Stopping Clock [%s-%d]: %f Seconds\n',nonterminal,clustercount,toc(nt_clock));
    else
        fprintf('%s : freq=%d : cl=%d : msvd=%d : %s already computed.\n',nonterminal,frequency,clustercount,mSVD,char(clusteringMethod));
    end
end

