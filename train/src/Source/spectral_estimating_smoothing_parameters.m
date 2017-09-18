% 
% % Initiate LPCFG Grammar with basics
% fprintf('Update Basic Info of the Grammar ...\n');
% obj_lpcfg_grammar = lpcfgspectralestimation.LPCFGGrammar;
% updateLPCFGGrammarBasics(obj_matlabinterface, obj_lpcfg_grammar);

% Update and Save Node parameters
fprintf('Update and Save smoothing paramater [node] ... \n');
obj_iterator_nts = getNonterminalsIterator(obj_matlabinterface, obj_lpcfg_grammar);
ntids = [];
while obj_iterator_nts.hasNext()
    ntid = obj_iterator_nts.next();
    ntids = horzcat(ntids,[ntid]);
end

parfor ntcount=1:size(ntids,2)
    ntid = ntids(ntcount);
    fprintf('%d : Processing nodeid : %d : %d ...\n',ntcount,ntid,size(nonterminals.(cell2mat({'nt',int2str(ntid)})),1));

    % Get the first instance to find the latent states [nt_a_javaid, nt_a_matlabid, nt_a_instance, isPreterminal, IsRoot];
    first_matlabid = nonterminals.(cell2mat({'nt',int2str(ntid)}))(1, 2);
    nt_latentstates = size(projections.(cell2mat({'ntI',int2str(first_matlabid)})),1);
    % Initiate parameters
    e_root = zeros(nt_latentstates,1);
    f_a = zeros(nt_latentstates,1);
    h_a = zeros(nt_latentstates,1);
    e_preter = zeros(nt_latentstates,1);
    
    total_instance = 0;
    total_preter = 0;
    
    for inscount=1:size(nonterminals.(cell2mat({'nt',int2str(ntid)})),1)
        node_javaid = nonterminals.(cell2mat({'nt',int2str(ntid)}))(inscount, 1);
        node_matlabid = nonterminals.(cell2mat({'nt',int2str(ntid)}))(inscount, 2);
        node_instance = nonterminals.(cell2mat({'nt',int2str(ntid)}))(inscount, 3);
        node_isPreterminal = nonterminals.(cell2mat({'nt',int2str(ntid)}))(inscount, 4);
        node_isRoot = nonterminals.(cell2mat({'nt',int2str(ntid)}))(inscount, 5);

        % Get the inside and outside matrices
        outsidematrix = projections.(cell2mat({'ntO',int2str(node_matlabid)}))(:,node_instance);
        insidematrix = projections.(cell2mat({'ntI',int2str(node_matlabid)}))(:,node_instance);      
        
        total_instance = total_instance + 1;
        
        % update f_a
        f_a = f_a + insidematrix;
    
        % update h_a
        h_a = h_a + outsidematrix;
    
        % update e_roots
        if node_isRoot == 1
            e_root =  e_root + insidematrix;
        end
    
        % update e_preter
        if node_isPreterminal == 1
            total_preter = total_preter + 1;
            e_preter = e_preter + outsidematrix;
        end
    end
    
    % Normalising parameters
    f_a = f_a / total_instance;
    h_a = h_a / total_instance;
    
    if total_root ~= 0
        e_root = e_root / total_root;
    end
    
    if total_preter ~= 0
        e_preter = e_preter / total_preter;
    end
    
    
    % Save parameter variables
    save_node_estimates(cell2mat({Estimates_Directory, 'Node-', int2str(ntid), '.mat'}), e_root, f_a, h_a, e_preter);
end

% Update and Save Unary Rule parameters
fprintf('Update and Save smoothing paramater structures [Unary Rules] ... \n');
obj_iterator_unaryruleids = getUnaryRulesIterator(obj_matlabinterface, obj_lpcfg_grammar);
unaryruleids = [];
while obj_iterator_unaryruleids.hasNext()
    ruleid = obj_iterator_unaryruleids.next();
    unaryruleids = horzcat(unaryruleids, [ruleid]);
end

parfor rulecount=1:size(unaryruleids,2)
    unaryruleid = unaryruleids(rulecount);
    fprintf('%d : Processing unary rule id : %d : %d ...\n',rulecount,unaryruleid,size(unaryrules.(cell2mat({'rule',int2str(unaryruleid)})),1));

    % Get the first instance to find the latent states [cnf_unary_javaid, nt_a_javaid, nt_a_matlabid, nt_a_instance]
    first_matlabid = unaryrules.(cell2mat({'rule',int2str(unaryruleid)}))(1, 3);
    nt_latentstates = size(projections.(cell2mat({'ntI',int2str(first_matlabid)})),1);
    % Initiate parameters
    e_ax = zeros(nt_latentstates,1);
    total_instance = 0;
    
    for inscount=1:size(unaryrules.(cell2mat({'rule',int2str(unaryruleid)})),1)
        unarycnf_javaid = unaryrules.(cell2mat({'rule',int2str(unaryruleid)}))(inscount, 1);
        node_a_javaid = unaryrules.(cell2mat({'rule',int2str(unaryruleid)}))(inscount, 2);
        node_a_matlabid = unaryrules.(cell2mat({'rule',int2str(unaryruleid)}))(inscount, 3);
        node_a_instance = unaryrules.(cell2mat({'rule',int2str(unaryruleid)}))(inscount, 4);
        
        % Get the outside matrix
        outsidematrix_a_pre = projections.(cell2mat({'ntO',int2str(node_a_matlabid)}))(:,node_a_instance);
        
        total_instance = total_instance + 1;
        
        % update e_ax
        e_ax = e_ax + outsidematrix_a_pre;
    end
    
    % Normalising parameters
    e_ax = e_ax / total_instance;
        
    % Save parameter variables
    save_unaryrule_estimates(cell2mat({Estimates_Directory, 'Unary-', int2str(unaryruleid), '.mat'}), e_ax);
end

% Update and Save Binary Rule parameters
fprintf('Update and Save smoothing paramater structures [Binary Rules] ... \n');
obj_iterator_binaryruleids = getBinaryRulesIterator(obj_matlabinterface, obj_lpcfg_grammar);
binaryruleids = [];
while obj_iterator_binaryruleids.hasNext()
    ruleid = obj_iterator_binaryruleids.next();
    binaryruleids = horzcat(binaryruleids, [ruleid]);
end

parfor rulecount=1:size(binaryruleids,2)
    binaryruleid = binaryruleids(rulecount);
    fprintf('%d : Processing binary rule id : %d : %d ...\n',rulecount,binaryruleid,size(binaryrules.(cell2mat({'rule',int2str(binaryruleid)})),1));

    % Get the first instance to find the latent states 
    % [cnf_binary_javaid, nt_a_javaid, nt_a_matlabid, nt_a_instance, nt_b_javaid, nt_b_matlabid, nt_b_instance, nt_c_javaid, nt_c_matlabid, nt_c_instance]
    first_a_matlabid = binaryrules.(cell2mat({'rule',int2str(binaryruleid)}))(1, 3);
    nt_a_latentstates = size(projections.(cell2mat({'ntI',int2str(first_a_matlabid)})),1);
    first_b_matlabid = binaryrules.(cell2mat({'rule',int2str(binaryruleid)}))(1, 6);
    nt_b_latentstates = size(projections.(cell2mat({'ntI',int2str(first_b_matlabid)})),1);
    first_c_matlabid = binaryrules.(cell2mat({'rule',int2str(binaryruleid)}))(1, 9);
    nt_c_latentstates = size(projections.(cell2mat({'ntI',int2str(first_c_matlabid)})),1);
    
    % Initiate parameters
    e_abc = zeros(nt_a_latentstates, nt_b_latentstates, nt_c_latentstates);
    
    e_abc_ij = zeros(nt_a_latentstates, nt_b_latentstates);
    e_abc_ik = zeros(nt_a_latentstates, nt_c_latentstates);
    e_abc_jk = zeros(nt_b_latentstates, nt_c_latentstates);
    e_abc_i = zeros(nt_a_latentstates, 1);
    e_abc_j = zeros(nt_b_latentstates, 1);
    e_abc_k = zeros(nt_c_latentstates, 1);
    
    % third order
    % e_4_abc = zeros(nt_a_latentstates, nt_b_latentstates, nt_c_latentstates);
    
    total_instance = 0;
    
    for inscount=1:size(binaryrules.(cell2mat({'rule',int2str(binaryruleid)})),1)
        
        binarycnf_javaid = binaryrules.(cell2mat({'rule',int2str(binaryruleid)}))(inscount, 1);
        node_a_javaid = binaryrules.(cell2mat({'rule',int2str(binaryruleid)}))(inscount, 2);
        node_a_matlabid = binaryrules.(cell2mat({'rule',int2str(binaryruleid)}))(inscount, 3);
        node_a_instance = binaryrules.(cell2mat({'rule',int2str(binaryruleid)}))(inscount, 4);
        node_b_javaid = binaryrules.(cell2mat({'rule',int2str(binaryruleid)}))(inscount, 5);
        node_b_matlabid = binaryrules.(cell2mat({'rule',int2str(binaryruleid)}))(inscount, 6);
        node_b_instance = binaryrules.(cell2mat({'rule',int2str(binaryruleid)}))(inscount, 7);
        node_c_javaid = binaryrules.(cell2mat({'rule',int2str(binaryruleid)}))(inscount, 8);
        node_c_matlabid = binaryrules.(cell2mat({'rule',int2str(binaryruleid)}))(inscount, 9);
        node_c_instance = binaryrules.(cell2mat({'rule',int2str(binaryruleid)}))(inscount, 10);
        
        % Get the required outside and inside matrices
        outsidematrix_a = projections.(cell2mat({'ntO',int2str(node_a_matlabid)}))(:,node_a_instance);
        insidematrix_b = projections.(cell2mat({'ntI',int2str(node_b_matlabid)}))(:,node_b_instance);
        insidematrix_c = projections.(cell2mat({'ntI',int2str(node_c_matlabid)}))(:,node_c_instance);
        
        total_instance = total_instance + 1;
        
        % update parameters
        prod_ab = outsidematrix_a * insidematrix_b.';
        prod_bc = insidematrix_b * insidematrix_c.';
        prod_ac = outsidematrix_a * insidematrix_c.';
        
        % estimate e_abc
        for m_c=1:size(insidematrix_c,1)
            e_abc(:,:,m_c) = e_abc(:,:,m_c) + (prod_ab * insidematrix_c(m_c));
        end
        
        % Other estimates
        e_abc_ij = e_abc_ij + prod_ab;
        e_abc_ik = e_abc_ik + prod_ac;
        e_abc_jk = e_abc_jk + prod_bc;
        
        e_abc_i = e_abc_i + outsidematrix_a;
        e_abc_j = e_abc_j + insidematrix_b;
        e_abc_k = e_abc_k + insidematrix_c;
        
    end
    
    % Normalising parameters
    e_abc = e_abc / total_instance;
    
    e_abc_ij = e_abc_ij / total_instance;
    e_abc_ik = e_abc_ik / total_instance;
    e_abc_jk = e_abc_jk / total_instance;
    e_abc_i = e_abc_i / total_instance;
    e_abc_j = e_abc_j / total_instance;
    e_abc_k = e_abc_k / total_instance;
    
    % Estimating first-order e_2_abc (nt_a_latentstates, nt_b_latentstates, nt_c_latentstates)
    e_abc_ij_k = zeros(nt_a_latentstates, nt_b_latentstates, nt_c_latentstates);
    e_abc_ik_j = zeros(nt_a_latentstates, nt_b_latentstates, nt_c_latentstates);
    e_abc_jk_i = zeros(nt_a_latentstates, nt_b_latentstates, nt_c_latentstates);
    
    for k=1:nt_c_latentstates
        e_abc_ij_k(:,:,k) =  e_abc_ij * e_abc_k(k)
    end
    for j=1:nt_b_latentstates
        e_abc_ik_j(:,j,:) =  e_abc_ik * e_abc_j(j)
    end
    for i=1:nt_a_latentstates
        e_abc_jk_i(i,:,:) =  e_abc_jk * e_abc_i(i)
    end 
    e_2_abc = (e_abc_ij_k + e_abc_ik_j + e_abc_jk_i) / 3;
    
    % Estimating second-order e_3_abc
    e_3_abc = zeros(nt_a_latentstates, nt_b_latentstates, nt_c_latentstates);
    prod_e_abc_i_e_abc_j = e_abc_i * e_abc_j.';
    for m_c=1:nt_c_latentstates
        e_3_abc(:,:,m_c) = (prod_e_abc_i_e_abc_j * e_abc_k(m_c));
    end
    
    % Save parameter variables
    save_binaryrule_estimates(cell2mat({Estimates_Directory, 'Binary-', int2str(binaryruleid), '.mat'}), e_abc, e_2_abc, e_3_abc);
end

