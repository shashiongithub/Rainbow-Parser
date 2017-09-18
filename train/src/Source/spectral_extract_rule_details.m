
% Initiate LPCFG Grammar with basics
fprintf('Update Basic Info of the Grammar ...\n');
obj_lpcfg_grammar = lpcfgsmoothclusterestimationuniversal.SpectralLPCFGGrammar;
updateLPCFGGrammarBasics(obj_matlabinterface, obj_lpcfg_grammar);

% Create Java HashMap for Nonterminals, Counter and mFinal
obj_hashmap_nt_id = getHashMapStrInt(obj_matlabinterface);

for count = 1:size(nonterminals_info,1)
    nonterminal = cell2mat(nonterminals_info(count, 1));
    frequency = cell2mat(nonterminals_info(count, 2));
    mSVD = cell2mat(nonterminals_info(count, 3));
    mFinal = getValueFromHashMap(obj_matlabinterface, obj_hashmap_nt_clusterinfo, count);
    
    % Add to the HashMap
    obj_hashmap_nt_id.put(nonterminal, count);
    
    % Add latents states to the grammar
    updateLPCFGGrammarNTState(obj_matlabinterface, obj_lpcfg_grammar, nonterminal, mFinal);
    
    % Counter
    counter.(cell2mat({'ntC',int2str(count)})) = 0;
end

% Initialize structures unaryrules and binaryrules
fprintf('Initialize rule-details structures ...\n');

obj_iterator_unaryruleids = getUnaryRulesIterator(obj_matlabinterface, obj_lpcfg_grammar);
while obj_iterator_unaryruleids.hasNext()
    ruleid = obj_iterator_unaryruleids.next();
    unaryrules.(cell2mat({'rule',int2str(ruleid)})) = [];
end

obj_iterator_binaryruleids = getBinaryRulesIterator(obj_matlabinterface, obj_lpcfg_grammar);
while obj_iterator_binaryruleids.hasNext()
    ruleid = obj_iterator_binaryruleids.next();
    binaryrules.(cell2mat({'rule',int2str(ruleid)})) = [];
end

% Initialize structures nonterminals
fprintf('Initialize nonterminals structures ...\n');

obj_iterator_nts = getNonterminalsIterator(obj_matlabinterface, obj_lpcfg_grammar);
while obj_iterator_nts.hasNext()
    ntid = obj_iterator_nts.next();
    nonterminals.(cell2mat({'nt',int2str(ntid)})) = [];
end


% Total root count
total_root = 0;

% Start Collecting Rule Details: 
fprintf('Start populating rule-details and nonterminals structures ...\n');

% Creating Clock
obj_clock = getClock(obj_matlabinterface, 'Collecting Rule Details');

% Training data iterator : Iterate on the file and populate
obj_fileLineIterator = getFileLineIterator(obj_matlabinterface);
% Iteration monitor
obj_iterationmonitor = getIterationMonitor(obj_matlabinterface);

count_line = 0;
while obj_fileLineIterator.hasNext()
    obj_iterationmonitor.update();
    
    % Current Line Count
    count_line = count_line + 1;
    % printObject(obj_matlabinterface, count_line);
            
    obj_trainingTree = obj_fileLineIterator.next();
    obj_constituentsmap = obj_trainingTree.getConstituents();
        
    % Get Post Order NodeList Iterator (Warning: Same order should be used while assigning clusters back)
    obj_nodetrees_postorder = getPostOrderNodeListIterator(obj_matlabinterface, obj_trainingTree);
    
    while obj_nodetrees_postorder.hasNext()
        obj_nodetree = obj_nodetrees_postorder.next();
        
        if isNodeTreeNotLeaf(obj_matlabinterface, obj_nodetree)
               
            if isNodeTreePreterminal(obj_matlabinterface, obj_nodetree)
                % Preterminal Tree: Unary rules 
               
                % Preterminal information
                nt_a = obj_nodetree.getLabel();
                nt_a_javaid = lookupNonterminal(obj_matlabinterface, obj_lpcfg_grammar, nt_a);
                nt_a_matlabid = obj_hashmap_nt_id.get(nt_a);
                counter.(cell2mat({'ntC',int2str(nt_a_matlabid)})) = counter.(cell2mat({'ntC',int2str(nt_a_matlabid)})) + 1;
                nt_a_instance = counter.(cell2mat({'ntC',int2str(nt_a_matlabid)}));
                % Update preterminal label
                changeTreeLabel(obj_matlabinterface,obj_nodetree,nt_a_instance);
                
                % Update nts (not root) (last two, isPreterminal, IsRoot)
                node_vector = [nt_a_javaid, nt_a_matlabid, nt_a_instance, 1, 0];
                temp_value = nonterminals.(cell2mat({'nt',int2str(nt_a_javaid)}));
                nonterminals.(cell2mat({'nt',int2str(nt_a_javaid)})) = vertcat(temp_value,node_vector);
                
                % Build Unary Rule
                obj_terminaltree = obj_nodetree.getChildren().get(0);
                terminal = obj_terminaltree.getLabel();
                t_javaid = lookupTerminal(obj_matlabinterface, obj_lpcfg_grammar, terminal);
                obj_cnf_unary = getUnaryCNFRule(obj_matlabinterface, nt_a_javaid, t_javaid);
                cnf_unary_javaid = lookupRule(obj_matlabinterface, obj_lpcfg_grammar, obj_cnf_unary);
                
                % Add this instance
                rule_vector = [cnf_unary_javaid, nt_a_javaid, nt_a_matlabid, nt_a_instance];
                temp_value = unaryrules.(cell2mat({'rule',int2str(cnf_unary_javaid)}));
                unaryrules.(cell2mat({'rule',int2str(cnf_unary_javaid)})) = vertcat(temp_value,rule_vector);
                
            else
                % Interminal: Binary rules
                
                % interminal information
                nt_a = obj_nodetree.getLabel();
                nt_a_javaid = lookupNonterminal(obj_matlabinterface, obj_lpcfg_grammar, nt_a);
                nt_a_matlabid = obj_hashmap_nt_id.get(nt_a);
                counter.(cell2mat({'ntC',int2str(nt_a_matlabid)})) = counter.(cell2mat({'ntC',int2str(nt_a_matlabid)})) + 1;
                nt_a_instance = counter.(cell2mat({'ntC',int2str(nt_a_matlabid)}));
                % Update interminal label
                changeTreeLabel(obj_matlabinterface,obj_nodetree,nt_a_instance);
                
                % Update nts (last two, isPreterminal, IsRoot)
                if isRoot(obj_matlabinterface,obj_nodetree,obj_trainingTree)
                    % Add this root instance
                    node_vector = [nt_a_javaid, nt_a_matlabid, nt_a_instance, 0, 1];
                    temp_value = nonterminals.(cell2mat({'nt',int2str(nt_a_javaid)}));
                    nonterminals.(cell2mat({'nt',int2str(nt_a_javaid)})) = vertcat(temp_value,node_vector);
                    
                    total_root = total_root + 1;
                else
                    node_vector = [nt_a_javaid, nt_a_matlabid, nt_a_instance, 0, 0];
                    temp_value = nonterminals.(cell2mat({'nt',int2str(nt_a_javaid)}));
                    nonterminals.(cell2mat({'nt',int2str(nt_a_javaid)})) = vertcat(temp_value,node_vector);
                end
                
                % Left child
                obj_leftchild = obj_nodetree.getChildren().get(0);
                nt_b = char(getLabel_LabelInsance(obj_matlabinterface, obj_leftchild.getLabel()));
                nt_b_javaid = lookupNonterminal(obj_matlabinterface, obj_lpcfg_grammar, nt_b);
                nt_b_matlabid = obj_hashmap_nt_id.get(nt_b);
                nt_b_instance = getInstance_LabelInsance(obj_matlabinterface, obj_leftchild.getLabel());
                
                % Right child
                obj_rightchild = obj_nodetree.getChildren().get(1);
                nt_c = char(getLabel_LabelInsance(obj_matlabinterface, obj_rightchild.getLabel()));
                nt_c_javaid = lookupNonterminal(obj_matlabinterface, obj_lpcfg_grammar, nt_c);
                nt_c_matlabid = obj_hashmap_nt_id.get(nt_c);
                nt_c_instance = getInstance_LabelInsance(obj_matlabinterface, obj_rightchild.getLabel());
                
                % Binary rule              
                obj_cnf_binary = getBinaryCNFRule(obj_matlabinterface, nt_a_javaid, nt_b_javaid, nt_c_javaid, obj_leftchild, obj_rightchild);
                cnf_binary_javaid = lookupRule(obj_matlabinterface, obj_lpcfg_grammar, obj_cnf_binary);
                
                % Add this rule instance
                rule_vector = [cnf_binary_javaid, nt_a_javaid, nt_a_matlabid, nt_a_instance, nt_b_javaid, nt_b_matlabid, nt_b_instance, nt_c_javaid, nt_c_matlabid, nt_c_instance];
                temp_value = binaryrules.(cell2mat({'rule',int2str(cnf_binary_javaid)}));
                binaryrules.(cell2mat({'rule',int2str(cnf_binary_javaid)})) = vertcat(temp_value,rule_vector);

            end
        end
    end
    
end

% Stop the clock
obj_clock.tock()
% 
% % Save Rule Details: 
% fprintf('Save rule-details structures ...\n');
% save(data_lpcfg_file, 'nonterminals', 'unaryrules', 'binaryrules', 'total_root');
