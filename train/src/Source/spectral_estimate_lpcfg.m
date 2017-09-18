
fprintf('Initialize Grammar with latent states ...\n');
for count = 1:size(nonterminals_info,1)
    nonterminal = cell2mat(nonterminals_info(count, 1));
    mFinal = cell2mat(nonterminals_info(count, 4));
    % Add latents states to the grammar
    updateLPCFGGrammarNTState(obj_matlabinterface, obj_lpcfg_grammar, nonterminal, mFinal);
end

fprintf('Initialize Grammar with pi, q and t ...\n');
initializeGrammarParameters(obj_matlabinterface, obj_lpcfg_grammar);

% Updating pi parameters
fprintf('Update pi parameters ... \n');
obj_iterator_interminal = getInterminalsIterator(obj_matlabinterface, obj_lpcfg_grammar);
while obj_iterator_interminal.hasNext()
    interminal_id = obj_iterator_interminal.next();
    fprintf('Processing interminal id : %d ...\n', interminal_id);
    
    % Load node file
    nodestruct = load(cell2mat({Estimates_Directory, 'Node-', int2str(interminal_id), '.mat'}), 'e_root');
    
    for m=1:size(nodestruct.e_root,1)       
        updatePiParameter(obj_matlabinterface, obj_lpcfg_grammar, interminal_id, m-1, nodestruct.e_root(m));
    end
end

% Updating q parameters
fprintf('Update q parameters ... \n');
obj_iterator_unaryruleids = getUnaryRulesIterator(obj_matlabinterface, obj_lpcfg_grammar);
while obj_iterator_unaryruleids.hasNext()
    unaryruleid = obj_iterator_unaryruleids.next();
    fprintf('Processing unary rule id : %d ...\n',unaryruleid);
    
    % Load rule file
    rulestruct = load(cell2mat({Estimates_Directory, 'Unary-', int2str(unaryruleid), '.mat'}), 'e_ax');
    
    % Get the preterminal id [cnf_unary_javaid, nt_a_javaid, nt_a_matlabid, nt_a_instance]
    preterminal_id = unaryrules.(cell2mat({'rule',int2str(unaryruleid)}))(1, 2);
    
    % Multiplication factor
    count_rule = getRuleCount(obj_matlabinterface, obj_lpcfg_grammar, unaryruleid);
    count_preterminal = getNonterminalCount(obj_matlabinterface, obj_lpcfg_grammar, preterminal_id);
    mul_factor = count_rule / count_preterminal;
    
    
    total_instance = size(unaryrules.(cell2mat({'rule',int2str(unaryruleid)})),1);
    if (isGrammarSmoothed == 1) && (total_instance < interpolation_cutoff)
        % Requires Smoothing
        
        % Load node file
        preterminalstruct = load(cell2mat({Estimates_Directory, 'Node-', int2str(preterminal_id), '.mat'}), 'e_preter');
    
        % Smoothed Value
        smoothed_val = parameter_v * rulestruct.e_ax + (1-parameter_v) * preterminalstruct.e_preter;
        
        for m=1:size(rulestruct.e_ax,1)
            updateQParameter(obj_matlabinterface, obj_lpcfg_grammar, unaryruleid, m-1, mul_factor*smoothed_val(m));
        end
        
    else
        % Does not require smoothing 
        for m=1:size(rulestruct.e_ax,1)
            updateQParameter(obj_matlabinterface, obj_lpcfg_grammar, unaryruleid, m-1, mul_factor*rulestruct.e_ax(m));
        end
       
    end
end

% Updating t parameters
fprintf('Update t parameters ... \n');
obj_iterator_binaryruleids = getBinaryRulesIterator(obj_matlabinterface, obj_lpcfg_grammar);
while obj_iterator_binaryruleids.hasNext()
    binaryruleid = obj_iterator_binaryruleids.next();
    fprintf('Processing binary rule id : %d ...\n',binaryruleid);

    % Load rule file
    rulestruct = load(cell2mat({Estimates_Directory, 'Binary-', int2str(binaryruleid), '.mat'}), 'e_abc', 'e_2_abc', 'e_3_abc');
    
    % Get the nonterminal ids 
    % [cnf_binary_javaid, nt_a_javaid, nt_a_matlabid, nt_a_instance, nt_b_javaid, nt_b_matlabid, nt_b_instance, nt_c_javaid, nt_c_matlabid, nt_c_instance]
    nonterminal_a_id = binaryrules.(cell2mat({'rule',int2str(binaryruleid)}))(1, 2);
    nonterminal_b_id = binaryrules.(cell2mat({'rule',int2str(binaryruleid)}))(1, 5);
    nonterminal_c_id = binaryrules.(cell2mat({'rule',int2str(binaryruleid)}))(1, 8);
    
    % Multiplication factor
    count_rule = getRuleCount(obj_matlabinterface, obj_lpcfg_grammar, binaryruleid);
    count_interminal = getNonterminalCount(obj_matlabinterface, obj_lpcfg_grammar, nonterminal_a_id);
    mul_factor = count_rule / count_interminal;
    
    total_instance = size(binaryrules.(cell2mat({'rule',int2str(binaryruleid)})),1);
    if (isGrammarSmoothed == 1) && (total_instance < interpolation_cutoff)
        % Requires Smoothing
        
        % Load node file
        nonterminal_a_struct = load(cell2mat({Estimates_Directory, 'Node-', int2str(nonterminal_a_id), '.mat'}), 'h_a');
        nonterminal_b_struct = load(cell2mat({Estimates_Directory, 'Node-', int2str(nonterminal_b_id), '.mat'}), 'f_a');
        nonterminal_c_struct = load(cell2mat({Estimates_Directory, 'Node-', int2str(nonterminal_c_id), '.mat'}), 'f_a');
        
        % Estimate parameter labmda
        parameter_lambda = sqrt(total_instance) / (parameter_c + sqrt(total_instance));
        
        % Estimate e_4_abc
        e_4_abc = zeros(size(nonterminal_a_struct.h_a, 1), size(nonterminal_b_struct.f_a, 1), size(nonterminal_c_struct.f_a, 1));
        prod_fb_fc = nonterminal_b_struct.f_a * (nonterminal_c_struct.f_a).';
        for m_a=1:size(nonterminal_a_struct.h_a, 1)
            e_4_abc(m_a,:,:) = nonterminal_a_struct.h_a(m_a) * prod_fb_fc;
        end
        
        % Smoothed Value
        k1_abc = parameter_lambda * rulestruct.e_3_abc + (1 - parameter_lambda) * e_4_abc;
        k2_abc = parameter_lambda * rulestruct.e_2_abc + (1 - parameter_lambda) * k1_abc;      
        smoothed_val = parameter_lambda * rulestruct.e_abc + (1 - parameter_lambda) * k2_abc;
        
        total_a = size(rulestruct.e_abc,1);
        total_b = size(rulestruct.e_abc,2);
        total_c = size(rulestruct.e_abc,3);
        
        for m_a=1:total_a
            for m_b=1:total_b
                for m_c=1:total_c
                    index = (total_b * total_c) * (m_a - 1) + total_c * (m_b - 1) + (m_c - 1);
                    updateTParameter(obj_matlabinterface, obj_lpcfg_grammar, binaryruleid, index, mul_factor*smoothed_val(m_a,m_b,m_c));
                end
            end
        end        
    else
        % Does not require smoothing 
        total_a = size(rulestruct.e_abc,1);
        total_b = size(rulestruct.e_abc,2);
        total_c = size(rulestruct.e_abc,3);
        
        for m_a=1:total_a
            for m_b=1:total_b
                for m_c=1:total_c
                    index = (total_b * total_c) * (m_a - 1) + total_c * (m_b - 1) + (m_c - 1);
                    updateTParameter(obj_matlabinterface, obj_lpcfg_grammar, binaryruleid, index, mul_factor*rulestruct.e_abc(m_a,m_b,m_c));
                end
            end
        end      
    end
end

% Write Grammar
fprintf('Writing Grammar ... \n');
writeGrammarToFile(obj_matlabinterface, obj_lpcfg_grammar, LPCFG_GRAMMAR_file);
