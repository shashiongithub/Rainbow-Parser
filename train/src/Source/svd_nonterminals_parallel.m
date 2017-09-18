
% Introduce Local Directory
inside_outside_dir = cell2mat({char(obj_globalparam.outputDirectory),'/Inside-Outside'});

% Creating SVD Directory
svd_dir = cell2mat({char(obj_globalparam.outputDirectory),'/SVD'});
fprintf('Creating SVD directory: %s \n',svd_dir);
command = cell2mat({'mkdir -p ', svd_dir});
shelloutput = system(command);

% ========== Initialize Parallel Pool 
parobject = parpool(Parallel_Pool_Size);

parfor count = 1:size(nonterminals_info,1)
    nonterminal = cell2mat(nonterminals_info(count, 1));
    frequency = cell2mat(nonterminals_info(count, 2));
    mSVD = cell2mat(nonterminals_info(count, 3));
    
    if ~exist(cell2mat({svd_dir, '/SVD-', nonterminal, '.mat'}), 'file')
        fprintf('%s : freq=%d : msvd=%d\n',nonterminal,frequency,mSVD);
            
        % Creating Clock
        nt_clock = tic;
        
        fprintf('Loading inside and outside matrices: %s\n',nonterminal);
        insidematrix = load(cell2mat({inside_outside_dir, '/I-', nonterminal, '.mat'}), cell2mat({'ntI',int2str(count)}));
        outsidematrix = load(cell2mat({inside_outside_dir, '/O-', nonterminal, '.mat'}), cell2mat({'ntO',int2str(count)}));
        
        fprintf('Covariance matrix: %s\n',nonterminal);
        covarmatrix = sparse(insidematrix.(cell2mat({'ntI',int2str(count)}))) * (sparse(outsidematrix.(cell2mat({'ntO',int2str(count)}))).');
        covarmatrix = covarmatrix/frequency;
        % clearvars insidematrix outsidematrix;
                
        if strcmp(nonterminal,'SHASHI') 
            fprintf('SVDSRAND Computation: [%s]\n',nonterminal);   
            [U,S,V] = svdsrand(covarmatrix, mSVD);
        else
            fprintf('SVDS Computation: [%s]\n',nonterminal);
            [U,S,V] = svds(covarmatrix, mSVD);
        end
        % clearvars covarmatrix;

        fprintf('Save SVD variables: [%s]\n',nonterminal);
        save_svd_variables(cell2mat({svd_dir, '/SVD-', nonterminal, '.mat'}), U, S, V);
        % clearvars U S V;

        % Stop the clock
        fprintf('Stopping Clock [%s]: %f Seconds\n',nonterminal,toc(nt_clock));
    else
        fprintf('%s : freq=%d : msvd=%d : SVD already computed.\n',nonterminal,frequency,mSVD);
    end
end

% ========== Delete Parallel Pool
delete(parobject);
