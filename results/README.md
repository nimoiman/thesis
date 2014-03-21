# Results

## Gaussian bivariate data
A covariance matrix [1, 0.5; 0.5, 1] was used to generate 10,000 training vectors and 10,000 testing vectors
### Varying channel epsilon

Channel epsilon was varied for (epsilon=0; epsilon<0.1 epsilon*=1.2) for independent (1D) COSQ

Other params:
 * lbg_epsilon = 0.001;
 * codevector_displace = 0.001;
 * seed = 1234;
 * nsplits = 4;

The resulting SNR and PSNR (in dB) are in `gauss_1d_eps_snr.csv` & `gauss_1d_eps_psnr.csv`. First column is epsilon, second is {SNR,PSNR}.

### Varying nsplits

The number of splits was tested for each combination of [0,6]x[0,6].

Other params:
 * lbg_epsilon = 0.001;
 * codevector_displace = 0.001;
 * seed = 1234;
 * channel epsilon = 0.001;

The resulting SNR and PSNR (in dB) tables are in `gauss_1d_nsplits_snr.out` & `gauss_1d_nsplits_psnr.out`

The same was run for channel epsilon = 0. The resulting SQNR, PSQNR (in dB) tables are in `gauss_1d_nsplits_sqnr.out` & `gauss_1d_nsplits_psqnr.out`

### Varying Correlation
A covariance matrix [1, ρ; ρ, 1] was used to generate 10,000 training vectors and 10,000 testing vectors for ρ=0.1...0.9 and tested on independent (1D) COSQ

Other params:
 * lbg_epsilon = 0.001
 * codevector_displace = 0.001
 * seed = 1234
 * channel epsilon = 0.001
 * nsplits = 4

The resulting SNR and PSNR (in dB) are in `gauss_1d_corr_snr.csv` & `gauss_1d_corr_psnr.csv` First column is ρ, second column is {SNR,PSNR}.

The same was run for channel epsilon = 0. The resulting SQNR, PSQNR (in dB) values are in `gauss_1d_corr_sqnr.csv` & `gauss_1d_corr_psqnr.csv` First column is ρ, second column is {SNR,PSNR}.out`

## Images


