
% Introduce Local Directory
inside_outside_dir = cell2mat({char(obj_globalparam.outputDirectory),'/Inside-Outside'});

% SVD Directory
svd_dir = SVD_Directory;

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
        
        % Adding noise to inside and outside matrices
        fprintf('Dropout Noise Introduction: %s : %s : Var=%2.2f\n',nonterminal,noisetype,probability); 

        [inI, inJ, inV] = find(insidematrix.(cell2mat({'ntI',int2str(count)})));
        binornd_inside = binornd(1,(1-probability), size(inI, 1), 1);
        binornd_inside = binornd_inside * (1.0/(1-probability));
        dropoutnoise_inside = sparse(inI, inJ, binornd_inside, size(insidematrix.(cell2mat({'ntI',int2str(count)})),1), size(insidematrix.(cell2mat({'ntI',int2str(count)})),2));
        insidematrix.(cell2mat({'ntI',int2str(count)})) = sparse(insidematrix.(cell2mat({'ntI',int2str(count)}))) .* sparse(dropoutnoise_inside);

        [outI, outJ, outV] = find(outsidematrix.(cell2mat({'ntO',int2str(count)})));
        binornd_outside = binornd(1,(1-probability), size(outI,1), 1);
        binornd_outside = binornd_outside * (1.0/(1-probability));
        dropoutnoise_outside = sparse(outI, outJ, binornd_outside, size(outsidematrix.(cell2mat({'ntO',int2str(count)})),1), size(outsidematrix.(cell2mat({'ntO',int2str(count)})),2));
        outsidematrix.(cell2mat({'ntO',int2str(count)})) = sparse(outsidematrix.(cell2mat({'ntO',int2str(count)}))) .* sparse(dropoutnoise_outside);
        
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