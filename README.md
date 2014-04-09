thesis
======

Implementation of Thesis

By Jamie Macdonald, Peter Thompson

## Version
You might be running an old version! (especially if it came from a CD).
Note that the latest version of this software will be hosted on Github at https://github.com/MacdonaldThompson/thesis

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
sh corr_j-j.sh
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

if you prematurely exited `corr_j-j.sh`, this will clean up the temp files:

```
rm -f {cw_map,cb}_j-j.csv
```
