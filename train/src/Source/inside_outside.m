
% Creating Inside-Outside Directory
inside_outside_dir = cell2mat({char(obj_globalparam.outputDirectory),'/Inside-Outside'});
fprintf('Creating Inside-Outside directory: %s \n',inside_outside_dir);
command = cell2mat({'mkdir -p ', inside_outside_dir});
shelloutput = system(command);

% Create Java HashMap for Nonterminals, Counter, inside and outside matrix
obj_hashmap_nt_int = getHashMapStrInt(obj_matlabinterface);
count_insidefeatures = dInsideTree(obj_matlabinterface);
count_outsidefeatutres = dprimeOutsideTree(obj_matlabinterface);
count_insideadditionalfeatures = getCountInsideAdditionalFeatures(obj_matlabinterface);
count_outsideadditionalfeatures = getCountOutsideAdditionalFeatures(obj_matlabinterface);


for count = 1:size(nonterminals_info,1)
    nonterminal = cell2mat(nonterminals_info(count, 1));
    frequency = cell2mat(nonterminals_info(count, 2));
    % Add to the HashMap
    obj_hashmap_nt_int.put(nonterminal, count);
    % Counter
    counter.(cell2mat({'ntC',int2str(count)})) = 0;
    % Inside matrix
    insidematrix.(cell2mat({'ntI',int2str(count)})) = sparse(count_insidefeatures, frequency);
    % Outside matrix
    outsidematrix.(cell2mat({'ntO',int2str(count)})) = sparse(count_outsidefeatutres, frequency);
end

% Creating Clock
obj_clock = getClock(obj_matlabinterface, 'Inside-Outside Computation');

% NAACL Features
if isNAACLFeaturesRequired(obj_matlabinterface)   
    
    obj_clock_naacl = getClock(obj_matlabinterface, 'Inside-Outside Computation: NAACL Features - One Go');
    
    % Training data iterator : Iterate on the file and populate all inside outside matrix
    obj_fileLineIterator = getFileLineIterator(obj_matlabinterface);
    % Iteration monitor
    obj_iterationmonitor = getIterationMonitor(obj_matlabinterface);

    count_line = 0;
    while obj_fileLineIterator.hasNext()
        obj_iterationmonitor.update();

        obj_trainingTree = obj_fileLineIterator.next();
        obj_constituentsmap = obj_trainingTree.getConstituents();

        % Get Post Order NodeList Iterator (Warning: Same order should be used while assigning clusters back)
        obj_nodetrees_postorder = getPostOrderNodeListIterator(obj_matlabinterface, obj_trainingTree);


        while obj_nodetrees_postorder.hasNext()
            obj_nodetree = obj_nodetrees_postorder.next();

            if isNodeTreeNotLeaf(obj_matlabinterface, obj_nodetree)
                nonterminal = obj_nodetree.getLabel();
                ntid = obj_hashmap_nt_int.get(char(nonterminal));

                % Increase Counter
                counter.(cell2mat({'ntC',int2str(ntid)})) = counter.(cell2mat({'ntC',int2str(ntid)})) + 1;
                count_nonterminal = counter.(cell2mat({'ntC',int2str(ntid)}));

                % Inside Matrix
                obj_insidetree = getInsideTreeT1(obj_matlabinterface, obj_nodetree, obj_constituentsmap, count_line);
                phi_insidetree = getPhiInsideTree(obj_matlabinterface, obj_insidetree);
                insidematrix.(cell2mat({'ntI',int2str(ntid)}))(:,count_nonterminal) = sparse(phi_insidetree);  

                % Outside Matrix
                obj_outsidetree = getOutsideTree(obj_matlabinterface, obj_trainingTree, obj_constituentsmap, obj_insidetree);
                psi_outsidetree = getPsiOutsideTree(obj_matlabinterface, obj_outsidetree);
                outsidematrix.(cell2mat({'ntO',int2str(ntid)}))(:,count_nonterminal) = sparse(psi_outsidetree);
            end
        end

        % Current Line Count
        count_line = count_line + 1;
        % printObject(obj_matlabinterface, count_line);
    end
    % Stop the clock
    obj_clock_naacl.tock();
else
    fprintf('Inside-Outside Computation: NAACL Features Not Required\n');
end

% Reset Counters
for count = 1:size(nonterminals_info,1)
    nonterminal = cell2mat(nonterminals_info(count, 1));
    % Counter
    counter.(cell2mat({'ntC',int2str(count)})) = 0;
end

% Additional Features
if isAdditionalFeaturesRequired(obj_matlabinterface)
    
    obj_clock_naacl = getClock(obj_matlabinterface, 'Inside-Outside Computation: Additional Features - One Go');
    
    % Additional Feature Data Iterator
    obj_additionalFeatFileIterator = getAdditionalFeatureFileIterator(obj_matlabinterface);
    % Iteration monitor
    obj_iterationmonitor = getIterationMonitor(obj_matlabinterface);

    count_line = 0;
    while obj_additionalFeatFileIterator.hasNext()
        obj_iterationmonitor.update();

        obj_trainingTree = obj_additionalFeatFileIterator.next();

        % Get Post Order NodeList Iterator (Warning: Same order should be used while assigning clusters back)
        obj_nodetrees_postorder = getPostOrderNodeListIterator(obj_matlabinterface, obj_trainingTree);


        while obj_nodetrees_postorder.hasNext()
            obj_nodetree = obj_nodetrees_postorder.next();

            if isNodeTreeNotLeaf(obj_matlabinterface, obj_nodetree)
                nonterminal = getLabel_AdditionalFeatureLabel(obj_matlabinterface, obj_nodetree);
                ntid = obj_hashmap_nt_int.get(char(nonterminal));

                % Increase Counter
                counter.(cell2mat({'ntC',int2str(ntid)})) = counter.(cell2mat({'ntC',int2str(ntid)})) + 1;
                count_nonterminal = counter.(cell2mat({'ntC',int2str(ntid)}));

                % Inside Matrix
                phi_insidetree = getPhi_AdditionalFeatureLabel(obj_matlabinterface, obj_nodetree);
                insidematrix.(cell2mat({'ntI',int2str(ntid)}))(((count_insidefeatures-count_insideadditionalfeatures)+1):count_insidefeatures,count_nonterminal) = sparse(phi_insidetree);  

                % Outside Matrix
                psi_outsidetree = getPsi_AdditionalFeatureLabel(obj_matlabinterface, obj_nodetree);
                outsidematrix.(cell2mat({'ntO',int2str(ntid)}))(((count_outsidefeatutres-count_outsideadditionalfeatures)+1):count_outsidefeatutres,count_nonterminal) = sparse(psi_outsidetree);
            end
        end

        % Current Line Count
        count_line = count_line + 1;
        % printObject(obj_matlabinterface, count_line);
    end
    % Stop the clock
    obj_clock_naacl.tock();
else
    fprintf('Inside-Outside Computation: Additional Features Not Required\n');
end

% Stop the clock
obj_clock.tock()

% Save and clear variables
obj_ntiterator = obj_hashmap_nt_int.entrySet().iterator();
while obj_ntiterator.hasNext()
    entry = obj_ntiterator.next();
    
    nonterminal = entry.getKey();
    ntid = entry.getValue();
   
    save(cell2mat({inside_outside_dir, '/I-', nonterminal, '.mat'}), '-struct', 'insidematrix', cell2mat({'ntI',int2str(ntid)}));
    save(cell2mat({inside_outside_dir, '/O-', nonterminal, '.mat'}), '-struct', 'outsidematrix', cell2mat({'ntO',int2str(ntid)}));
    
end

clearvars counter insidematrix outsidematrix;
