
% Creating Result Directory
fprintf('Creating result directory: %s \n',char(obj_globalparam.outputDirectory));
command = cell2mat({'mkdir -p ', char(obj_globalparam.outputDirectory)});
shelloutput = system(command);

% Open Diary
timestamp = datetime('now','TimeZone','local','Format','d-MMM-y HH:mm:ss');
diary(cell2mat({char(obj_globalparam.outputDirectory),'/diary-',char(timestamp),'.out'}));

% Preprocessing Training Corpus
% Setting number of lines in the training corpus: hugeFileCount
settingNumberOfTrainingInstances(obj_matlabinterface);
% Preprocessing training corpus and generating intermediate training files
preprocessTrainingCorpus(obj_matlabinterface);

% Preprocessing Development and Test data
preprocessDevAndTestCorpus(obj_matlabinterface);

% Update NAACL Feature Dictionary
updateNAACLDictionary(obj_matlabinterface, obj_naaclfeaturedict);
% Inside and Outside Feature Dimension
inside_features = dInsideTree(obj_matlabinterface)
outside_features = dprimeOutsideTree(obj_matlabinterface)

% Update CorpusData Object
updatecorpusDataWithNonterminals(obj_matlabinterface, obj_corpusdata);

% Extract a cell array of nonterminal ids 
obj_nonterminalids = obj_corpusdata.nonterminalsAlphabet.reverseMap.keys();
nonterminals_info = cell(size(obj_nonterminalids,1),3);
for count = 1:size(obj_nonterminalids,1)
    % Name
    nonterminals_info{count,1} = char(getNonterminalAlphabet(obj_matlabinterface, obj_corpusdata, obj_nonterminalids(count)));
    
    % Frequency
    nonterminals_info{count,2} = getFrequencyNonterminalAlphabet(obj_matlabinterface, obj_corpusdata, obj_nonterminalids(count));
    
    % mSVD Count
    nonterminals_info{count,3} = getmSVD(obj_matlabinterface, nonterminals_info{count,2});
end

% clearvars obj_nonterminals
nonterminalfile = cell2mat({char(obj_globalparam.outputDirectory),'/nonterminals.mat'});
fprintf('Creating nonterminal file: %s \n',nonterminalfile);
save(nonterminalfile,'nonterminals_info');
