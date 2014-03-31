#!/usr/bin/octave -qf
n_train = 250000;
n_sim = 25000;
mu = [0 0];
var1 = 1;
var2 = 1;
lim_x = 4;
lim_y = 4;

pkg load statistics

 
for corr_coeff = 0:0.01:1
    filename_train = sprintf('corr_%.2f.train', corr_coeff);
    filename_sim = sprintf('corr_%.2f.sim', corr_coeff);

 
    cov = corr_coeff*sqrt(var1*var2);
    SIGMA = [var1 cov; cov var2];

    
    train = mvnrnd(mu,SIGMA,n_train);
    sim = mvnrnd(mu,SIGMA,n_sim);

    

    
    % Remove outliers in training set
    first_pass = 1;
    while first_pass == 1 || outlier_count > 0
        Outliers = abs(train(:,1)) > lim_x | abs(train(:,2)) > lim_y;
        outlier_count = nnz(Outliers);
        if first_pass == 1
            display(corr_coeff);
            display(outlier_count);
            first_pass = 0;
        end
        if outlier_count > 0
            train(Outliers,:) = mvnrnd(mu,SIGMA,outlier_count);
        end
    end

    

    
    % Remove outliers in simulation set
    first_pass = 1;
    while first_pass == 1 || outlier_count > 0
        Outliers = abs(sim(:,1)) > lim_x | abs(sim(:,2)) > lim_y;
        outlier_count = nnz(Outliers);
        if first_pass == 1
            first_pass = 0;
        end
        if outlier_count > 0
            sim(Outliers,:) = mvnrnd(mu,SIGMA,outlier_count);
        end
    end

    
    csvwrite(filename_train, train);
    csvwrite(filename_sim, sim);
end
