thesis
======

Implementation of Thesis

## Shell scripts

### Correlation
Iterate through correlation values by generating data with octave/MATLAB with script `data_gen/corr_data.m`

```
octave -qf data_gen/corr_data.m
```

And it will barf files in your current working directory.

Then, decide which system to generate SNR data for:

For the I->I case:

```
sh corr_i-i.sh
```

For the J->J case:

```
sh corr_d-d.sh
```

For the I->J case:

```
sh cov_run.sh
```

These will produce an output file with rho, SNR data

Then clean up the barf.

```
rm corr_*.{sim,train,out}
```

if you prematurely exited `corr_i-i.sh`, this will clean up the temporary files:

```
rm -f {cw_map_i-i,cb_i-i,out_i-i_1,out_i-i_2,sim_i-i_1,sim_i-i_2,tr_i-i_1,tr_i-i_2}.csv
```

if you prematurely exited `corr_d-d.sh`, this will clean up the temp files:

```
rm -f {cw_map,cb}_d-d.csv
```