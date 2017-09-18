
fprintf('Starting the tuning process for %d nonterminals.\n', size(nonterminals_info,1));

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


%%%%%%%%%%%% BEAM SEARCH %%%%%%%%%%%%%%%%%%%%%%%%%%%%

% ========== Initialize Parallel Pool 
parobject = parpool(Parallel_Pool_Size);

% ========== Initialize TuningBeamSearch Object =====================
obj_tuningbeamsearch = javaObjectEDT('lpcfgsmoothclusterestimationuniversal.TuningBeamSearch', obj_hashmap_ntid_freq);

% Create a node in the middle with a grammar file
fprintf('Starting at nodeid: %f with the grammar: %s\n', nodeidinmiddle, grammarfileinmiddle);
obj_startgraphnode = createStartNodeInMiddle(obj_tuningbeamsearch, grammarfileinmiddle, nodeidinmiddle, obj_hashmap_ntstr_ntid);

% % Create the start Node in the problem graph
% fprintf('Starting fresh at nodeid: 1\n');
% obj_startgraphnode = createStartNode(obj_tuningbeamsearch);

% Offer the node to update the priority queue
offerToPriorityQueue(obj_tuningbeamsearch, obj_startgraphnode);

% Reset Cluster Info in the CorpusData
resetNonterminalsClusterInfo(obj_corpusdata);

% Start a loop until process the whole problem search space 
while isContinueBeamSearch(obj_tuningbeamsearch)
    
    % Print Full Beam 
    fprintf('Full beam status: %s\n',char(printFullBeam(obj_tuningbeamsearch)));
    
    % Poll 
    fprintf('POLL and Reinitialize\n')
    obj_bestgraphnode = pollFromPriorityQueue(obj_tuningbeamsearch);
    
    % Get current node being processed
    currentNtId = getCurrentNtId(obj_bestgraphnode);
    
    % Update obj_corpusdata with the best-ls for the completed NT, iff its complete
    if isNodeComplete(obj_bestgraphnode)
        fprintf('Update obj_corpusdata with the best-ls for the completed NT\n');
        ntcluster_to_load = cell2mat(nonterminals_info(currentNtId-1, 1));
        ntcluster_to_load_withls = getLSForNTId(obj_bestgraphnode, currentNtId-1);
        load_clusterdata_update_corpusdata;
    end
    
    % Current node being processed
    fprintf('Processing the node: %s : %d : frequency %d : preassigned LS %d\n', ...
        cell2mat(nonterminals_info(currentNtId, 1)), currentNtId, cell2mat(nonterminals_info(currentNtId, 2)), getLSForNTId(obj_bestgraphnode, currentNtId));
    % printObject(obj_matlabinterface, currentNtId);
    
    % Reinitialize 
    obj_tuningbeamsearch = javaObjectEDT('lpcfgsmoothclusterestimationuniversal.TuningBeamSearch', obj_hashmap_ntid_freq);
   
    %%%%%%%%%%%%%%%%%%%%%%% STAGE ONE %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    % Stage One : Sparse Search for latent states, if too many to explore
    obj_hashset_stageone_ls =  exploreStageOneLatentStates(obj_tuningbeamsearch, obj_bestgraphnode);
    fprintf('Stage one: potential latent states: ');
    printObject(obj_matlabinterface, obj_hashset_stageone_ls);
    
    % Extract all clusters to be created
    stageone_clusters = cell(1,1);
    clcount = 1;
    for count = 1:size(nonterminals_info,1)
        if count == currentNtId
            obj_iterator = getIteratorHashSet(obj_matlabinterface, obj_hashset_stageone_ls);
            while obj_iterator.hasNext()
                clustertogenerate = obj_iterator.next();
                stageone_clusters{clcount,1} = count;
                stageone_clusters{clcount,2} = clustertogenerate;
                clcount = clcount + 1;
            end
        else
            stageone_clusters{clcount,1} = count;
            stageone_clusters{clcount,2} = getLSForNTId(obj_bestgraphnode, count);
            clcount = clcount + 1;
        end
    end
        
    % Calculate Clustering 
    fprintf('Stage one clustering\n');
    clusterstocalculate = stageone_clusters;
    clustering_tuning_parallel
    
    fprintf('Parsing and evaluation if node is not complete\n');
    % Object for Children nodes
    obj_hashmap_double_graphchildren_stageone = getHashMapDoubleGraphNode(obj_matlabinterface);
    count = 1;
    % Parsing and evaluation commands
    parsing_eval_commands = cell(1,2);
    commandcount = 0;
    
    obj_iterator = getIteratorHashSet(obj_matlabinterface, obj_hashset_stageone_ls);
    while obj_iterator.hasNext()
        clustertogenerate = obj_iterator.next();
        obj_stageone_graphchildnode = createNewChild(obj_tuningbeamsearch, obj_bestgraphnode, clustertogenerate);
        
        fprintf('Processing the node: %s : %d : frequency %d : preassigned LS %d : Potential LS %d : ', ...
             cell2mat(nonterminals_info(currentNtId, 1)), currentNtId, cell2mat(nonterminals_info(currentNtId, 2)), getLSForNTId(obj_bestgraphnode, currentNtId), clustertogenerate);
        
        % Check if new node, create uniqueid, train with clusters, and
        % grammar file
        if ~isNodeComplete(obj_stageone_graphchildnode)
            fprintf('Incomplete.\n');
            
            % Create Unique Id and File names
            createUniqueId(obj_stageone_graphchildnode);
            % Create the training file with clusters
            training_file_withclusters = getTrainfilewithclusters(obj_stageone_graphchildnode);
            obj_hashmap_nt_clusterinfo = getHashMapNtIdtoLS(obj_stageone_graphchildnode);
            generate_trainingfile_with_clusterinfo
            % Estimate the grammar file
            grammar_file = getGrammarfile(obj_stageone_graphchildnode);
            lpcfg_estimation
            
            % Create commands 
            parse_command = cell2mat({char(obj_globalparam.parsingCommand), ' ', ... 
                int2str(obj_globalparam.rarewordCutoff), ' ', char(obj_globalparam.corpusVocab), ' ', ...
                char(obj_globalparam.PruneGrammarFile), ' ', char(obj_globalparam.LPCFGGrammarFile), ' ', ...
                char(obj_globalparam.developmentTaggedCorpus), ' > ', char(obj_globalparam.LPCFGGrammarFile), '.output', ...
                ' 2> ', char(obj_globalparam.LPCFGGrammarFile), '.parser.log'});
            parsing_eval_commands{commandcount+1,1} = parse_command;
            
            eval_command = cell2mat({char(obj_globalparam.evaluationCommand), ' ', ...
                char(obj_globalparam.LPCFGGrammarFile), '.output', ' ', char(obj_globalparam.developmentGoldCorpus), ' > ', ...
                char(obj_globalparam.LPCFGGrammarFile), '.eval'});
            parsing_eval_commands{commandcount+1,2} = eval_command;
            
            commandcount = commandcount + 1;
        else
            fprintf('Complete.\n');
        end
        
        obj_hashmap_double_graphchildren_stageone.put(count, obj_stageone_graphchildnode);
        count = count + 1;
    end
    % printObject(obj_matlabinterface, obj_hashmap_double_graphchildren_stageone);
    graphchildren_stageone_size = getSizeHashMapDoubleGraphNode(obj_matlabinterface, obj_hashmap_double_graphchildren_stageone);
    fprintf('Number of stage-one nodes: %d : Parsers to run: %d\n',graphchildren_stageone_size, commandcount);
    
    % Parse and evaluation in parallel
    if commandcount ~= 0
        fprintf('Stage one parsing in parallel\n');
        full_parse_command = '';
        for commandcount = 1:size(parsing_eval_commands, 1)
            full_parse_command = cell2mat({full_parse_command, cell2mat(parsing_eval_commands(commandcount, 1)), ' & '});
        end
        full_parse_command = cell2mat({full_parse_command, ' wait'});
        system(full_parse_command);
    
        % Evaluation in parallel
        fprintf('Stage one evaluation in sequence\n') % parallel\n');
        full_evaluation_command = '';
        for commandcount = 1:size(parsing_eval_commands, 1)
            full_evaluation_command = cell2mat({full_evaluation_command, cell2mat(parsing_eval_commands(commandcount, 2)), ' ; '}); % ' & '});
        end
        full_evaluation_command = cell2mat({full_evaluation_command, ' wait'});
        system(full_evaluation_command);
    end
    
    % Update node, push in the queue
    for graphchildcount = 1:graphchildren_stageone_size
        obj_stageone_graphchildnode = obj_hashmap_double_graphchildren_stageone.get(graphchildcount);
        
        fprintf('Completing the node: %s : %d : frequency %d : preassigned LS %d : This LS %d : ', ...
             cell2mat(nonterminals_info(currentNtId, 1)), currentNtId, cell2mat(nonterminals_info(currentNtId, 2)), ...
             getLSForNTId(obj_bestgraphnode, currentNtId), getLSForNTId(obj_stageone_graphchildnode, currentNtId));
        
        % Check if new node complete
        if ~isNodeComplete(obj_stageone_graphchildnode)
            fprintf('Incomplete.\nProcess the evaluation file and complete this node.\n');
            
            % Process the evaluation file
            processEvaluationFileAndComplete(obj_stageone_graphchildnode);
            
        else 
            fprintf('Complete : %f\n', getFMeasureOnDevData(obj_stageone_graphchildnode));
        end
        
        % Offer to the queue
        offerToPriorityQueue(obj_tuningbeamsearch, obj_stageone_graphchildnode);
    end
    
    %%%%%%%%%%%%%%%%%%%%%%% STAGE TWO %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    
    % Print Full Beam after Stage One
    fprintf('Stage One Beam Status: %s\n',char(printFullBeam(obj_tuningbeamsearch)));
    
    % Peek the best entry
    fprintf('PEEK\n')
    obj_stageone_bestgraphchildnode = peekFromPriorityQueue(obj_tuningbeamsearch);
    
    % Extract refined set to latent states for stage two
    bestls = getLSForNTId(obj_stageone_bestgraphchildnode, currentNtId);
    obj_hashset_stagetwo_ls =  exploreStageTwoLatentStates(obj_tuningbeamsearch, obj_bestgraphnode, bestls, obj_hashset_stageone_ls);
    fprintf('Stage two: potential latent states: ');
    printObject(obj_matlabinterface, obj_hashset_stagetwo_ls);
        
    % Check is any, if not skip rest
    stagetwo_ls_size = getSizeHashSetInteger(obj_matlabinterface, obj_hashset_stagetwo_ls);
    if stagetwo_ls_size == 0
        continue
    end
    
    % Extract all new clusters to be created
    stagetwo_clusters = cell(1,1);
    clcount = 1;
    for count = 1:size(nonterminals_info,1)
        if count == currentNtId
            obj_iterator = getIteratorHashSet(obj_matlabinterface, obj_hashset_stagetwo_ls);
            while obj_iterator.hasNext()
                clustertogenerate = obj_iterator.next();
                stagetwo_clusters{clcount,1} = count;
                stagetwo_clusters{clcount,2} = clustertogenerate;
                clcount = clcount + 1;
            end
        end
    end
        
    % Calculate Clustering 
    fprintf('Stage two clustering\n');
    clusterstocalculate = stagetwo_clusters;
    clustering_tuning_parallel
    
    fprintf('Parsing and evaluation if node is not complete\n');
    % Object for Children nodes
    obj_hashmap_double_graphchildren_stagetwo = getHashMapDoubleGraphNode(obj_matlabinterface);
    count = 1;
    % Parsing and evaluation commands
    parsing_eval_commands = cell(1,2);
    commandcount = 0;
    
    obj_iterator = getIteratorHashSet(obj_matlabinterface, obj_hashset_stagetwo_ls);
    while obj_iterator.hasNext()
        clustertogenerate = obj_iterator.next();
        obj_stagetwo_graphchildnode = createNewChild(obj_tuningbeamsearch, obj_bestgraphnode, clustertogenerate);
        
        fprintf('Processing the node: %s : %d : frequency %d : preassigned LS %d : Potential LS %d : ', ...
             cell2mat(nonterminals_info(currentNtId, 1)), currentNtId, cell2mat(nonterminals_info(currentNtId, 2)), getLSForNTId(obj_bestgraphnode, currentNtId), clustertogenerate);
        
        % Check if new node, create uniqueid, train with clusters, and
        % grammar file
        if ~isNodeComplete(obj_stagetwo_graphchildnode)
            fprintf('Incomplete.\n');
            
            % Create Unique Id and File names
            createUniqueId(obj_stagetwo_graphchildnode);
            % Create the training file with clusters
            training_file_withclusters = getTrainfilewithclusters(obj_stagetwo_graphchildnode);
            obj_hashmap_nt_clusterinfo = getHashMapNtIdtoLS(obj_stagetwo_graphchildnode);
            % printObject(obj_matlabinterface, obj_hashmap_nt_clusterinfo);
            generate_trainingfile_with_clusterinfo
            % Estimate the grammar file
            grammar_file = getGrammarfile(obj_stagetwo_graphchildnode);
            lpcfg_estimation            
            
            % Create commands 
            parse_command = cell2mat({char(obj_globalparam.parsingCommand), ' ', ... 
                int2str(obj_globalparam.rarewordCutoff), ' ', char(obj_globalparam.corpusVocab), ' ', ...
                char(obj_globalparam.PruneGrammarFile), ' ', char(obj_globalparam.LPCFGGrammarFile), ' ', ...
                char(obj_globalparam.developmentTaggedCorpus), ' > ', char(obj_globalparam.LPCFGGrammarFile), '.output', ...
                ' 2> ', char(obj_globalparam.LPCFGGrammarFile), '.parser.log'});
            parsing_eval_commands{commandcount+1,1} = parse_command;
            
            eval_command = cell2mat({char(obj_globalparam.evaluationCommand), ' ', ...
                char(obj_globalparam.LPCFGGrammarFile), '.output', ' ', char(obj_globalparam.developmentGoldCorpus), ' > ', ...
                char(obj_globalparam.LPCFGGrammarFile), '.eval'});
            parsing_eval_commands{commandcount+1,2} = eval_command;
            
            commandcount = commandcount + 1;
        else
            fprintf('Complete.\n');
        end
        
        obj_hashmap_double_graphchildren_stagetwo.put(count, obj_stagetwo_graphchildnode);
        count = count + 1;
    end
    % printObject(obj_matlabinterface, obj_hashmap_double_graphchildren_stagetwo);
    graphchildren_stagetwo_size = getSizeHashMapDoubleGraphNode(obj_matlabinterface, obj_hashmap_double_graphchildren_stagetwo);
    fprintf('Number of stage-two nodes: %d : Parsers to run: %d\n',graphchildren_stagetwo_size, commandcount);
    
    % Parse and evaluation in parallel
    if commandcount ~= 0
        fprintf('Stage two parsing in parallel\n');
        full_parse_command = '';
        for commandcount = 1:size(parsing_eval_commands, 1)
            full_parse_command = cell2mat({full_parse_command, cell2mat(parsing_eval_commands(commandcount, 1)), ' & '});
        end
        full_parse_command = cell2mat({full_parse_command, ' wait'});
        system(full_parse_command);
    
        % Evaluation in parallel
        fprintf('Stage two evaluation in sequence\n') % parallel\n');
        full_evaluation_command = '';
        for commandcount = 1:size(parsing_eval_commands, 1)
            full_evaluation_command = cell2mat({full_evaluation_command, cell2mat(parsing_eval_commands(commandcount, 2)), ' ; '}); % ' & '});
        end
        full_evaluation_command = cell2mat({full_evaluation_command, ' wait'});
        system(full_evaluation_command);
    end
    
    % Update node, push in the queue
    for graphchildcount = 1:graphchildren_stagetwo_size
        obj_stagetwo_graphchildnode = obj_hashmap_double_graphchildren_stagetwo.get(graphchildcount);
        
        fprintf('Completing the node: %s : %d : frequency %d : preassigned LS %d : This LS %d : ', ...
             cell2mat(nonterminals_info(currentNtId, 1)), currentNtId, cell2mat(nonterminals_info(currentNtId, 2)), ...
             getLSForNTId(obj_bestgraphnode, currentNtId), getLSForNTId(obj_stagetwo_graphchildnode, currentNtId));
        
        % Check if new node complete
        if ~isNodeComplete(obj_stagetwo_graphchildnode)
            fprintf('Incomplete.\nProcess the evaluation file and complete this node.\n');
            
            % Process the evaluation file
            processEvaluationFileAndComplete(obj_stagetwo_graphchildnode);
            
        else 
            fprintf('Complete : %f\n', getFMeasureOnDevData(obj_stagetwo_graphchildnode));
        end
        
        % Offer to the queue
        offerToPriorityQueue(obj_tuningbeamsearch, obj_stagetwo_graphchildnode);
    end    
end

% Get the final output 
fprintf('Best Node: %s\n',char(printBestNode(obj_tuningbeamsearch)));


% ========== Delete Parallel Pool
delete(parobject);

