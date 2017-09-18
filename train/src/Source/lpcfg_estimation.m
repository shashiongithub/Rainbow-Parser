
% Uses training_file_withclusters and grammar_file

% Introduce Local Directory
trainwithcl_dir = char(obj_globalparam.trainWithClusterDirectory);

% Creating Grammar Directory
grammar_dir = cell2mat({char(obj_globalparam.outputDirectory),'/Grammars-',char(obj_globalparam.clusteringMethod)});
if strcmp(obj_globalparam.clusteringMethod, 'kmeans-fuzzy') || strcmp(obj_globalparam.clusteringMethod, 'cmeans')
    grammar_dir = cell2mat({grammar_dir,'-',num2str(obj_globalparam.fuzziness)});
end
obj_globalparam.grammarDirectory = grammar_dir;
fprintf('Creating Grammar directory: %s \n',grammar_dir);
command = cell2mat({'mkdir -p ', grammar_dir});
shelloutput = system(command);

% Reset LPCFG Grammar Object
resetObject(obj_lpcfggrammar);

% Setting Local Variables and estimating the grammar file
obj_globalparam.corpusParseTreesWithClusters = cell2mat({trainwithcl_dir, '/',char(training_file_withclusters)});
obj_globalparam.LPCFGGrammarFile = cell2mat({grammar_dir, '/',char(grammar_file)});
estimateAndWriteLPCFG(obj_lpcfggrammar);


