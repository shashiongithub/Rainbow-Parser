
% Introduce Local Directory
inside_outside_dir = cell2mat({char(obj_globalparam.outputDirectory),'/Inside-Outside'});
svd_dir = cell2mat({char(obj_globalparam.outputDirectory),'/SVD'});

% ========== Intialize matlab variables from Java variables ==========
eigenvaluecutoff = obj_globalparam.eigenValueCutoff;

% Projection Structure Variable
projections = struct();

for count = 1:size(nonterminals_info,1)
    nonterminal = cell2mat(nonterminals_info(count, 1));
    frequency = cell2mat(nonterminals_info(count, 2));
    mSVD = cell2mat(nonterminals_info(count, 3));
    mFinal = getValueFromHashMap(obj_matlabinterface, obj_hashmap_nt_clusterinfo, count);
    mMIN = min(mSVD, mFinal);
    
    % Creating Clock
    nt_clock = tic;
    
    fprintf('Loading inside and outside matrices: %s\n',nonterminal);
    insidematrix = load(cell2mat({inside_outside_dir, '/I-', nonterminal, '.mat'}), cell2mat({'ntI',int2str(count)}));
    outsidematrix = load(cell2mat({inside_outside_dir, '/O-', nonterminal, '.mat'}), cell2mat({'ntO',int2str(count)}));
    
    fprintf('Loading SVDs S, U and V  matrices: %s\n',nonterminal);
    svdmatrix = load(cell2mat({svd_dir, '/SVD-', nonterminal, '.mat'}),'U', 'S', 'V');    
    
    % mSVD has been changed to mMIN to get values from the grammar
    fprintf('Select meaningful eigen values (> %f): %s\n',eigenvaluecutoff,nonterminal);
    diagonal = diag(svdmatrix.('S'));
    if (size(diagonal,1) > mMIN)
        diagonal = diagonal(1:mMIN);
    end
    diagonalfiltered = diagonal(diagonal > eigenvaluecutoff);
    validsize = size(diagonalfiltered, 1);
    fprintf('Before : %d and After : %d : %s\n',size(diagonal, 1),validsize,nonterminal);
            
    % Modify U, V and S
    svdmatrix.('U') = svdmatrix.('U')(:,1:validsize);
    svdmatrix.('V') = svdmatrix.('V')(:,1:validsize);
    svdmatrix.('S') = svdmatrix.('S')(1:validsize,1:validsize);      
    
    fprintf('Dimension reduction: Insidematrix and OutsideMatrix : %s\n',nonterminal);
    projections.(cell2mat({'ntI',int2str(count)})) = (svdmatrix.('U').')*insidematrix.(cell2mat({'ntI',int2str(count)}));
    projections.(cell2mat({'ntO',int2str(count)})) = inv(svdmatrix.('S'))*((svdmatrix.('V').')*outsidematrix.(cell2mat({'ntO',int2str(count)})));
    
    % Update nonterminals_info
    nonterminals_info{count,4} = validsize;
     
    % Stop the clock
    fprintf('Stopping Clock [%s]: %f Seconds\n',nonterminal,toc(nt_clock));
    
end

% Update nonterminal info
% save(nonterminals_file,'nonterminals_info');

% Save projections
% save(projections_file, 'projections');


