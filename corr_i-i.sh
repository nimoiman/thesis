#!/usr/bin/bash
make covq

# Parameters
nsplits=2
bep=0.0
lbg_eps=0.001
cv_disp=0.0001

# tmp files
tr_1=tr_1.csv
tr_2=tr_2.csv
sim_1=sim_1.csv
sim_2=sim_2.csv
out_1=out_1.csv
out_2=out_2.csv

suff=i-i
snr_out=snr_$suff.out
cb=cb_$suff.csv
cw=cw_map_$suff.csv


# Clear SNR output file
rm -f $snr_out

k=0
for i in {0..9}
do
    for j in 0 5
    do
        train=corr_$k.$i$j.train
        sim=corr_$k.$i$j.sim
        if [ ! -f $train ] || [ ! -f $sim ]
            then echo "Cannot find train/sim files $train, $sim; rho=$k.$i$j"
            exit 1
        fi

        # Split the sets into single columns
        python python/util.py csv_unzip $train $tr_1 $tr_2
        python python/util.py csv_unzip $sim $sim_1 $sim_2
        
        # Run on left column
        ./covq/covq \
        --train $tr_1 $cb $cw \
        --test $sim_1 $cb $cw $out_1 \
        --dim 1 --nsplits $nsplits --bep $bep --lbg-eps $lbg_eps --cv-disp $cv_disp

        if [ $? -ne 0 ]
            then echo "Returned non-zero status"
            exit 1
        fi

        # Run on right column
        ./covq/covq \
        --train $tr_2 $cb $cw \
        --test $sim_2 $cb $cw $out_2 \
        --dim 1 --nsplits $nsplits --bep $bep --lbg-eps $lbg_eps --cv-disp $cv_disp

        if [ $? -ne 0 ]
            then echo "Returned non-zero status"
            exit 1
        fi

        # Zip the results back into 2 column csv
        python python/util.py csv_zip $out_1 $out_2 corr_$k.$i$j.out
        
        # Write rho to out file
        echo -n $k.$i$j >> $snr_out
        echo -n ', ' >> $snr_out
        # Get python to measure SNR
        python python/run.py get_snr $sim corr_$k.$i$j.out >> $snr_out
    done
done

k=1
i=0
j=0

train=$train
sim=$sim
if [ ! -f $train ] || [ ! -f $sim ]
    then echo "Cannot find train/sim files $train, $sim; rho=$k.$i$j"
    exit 1
fi

# Split the sets into single columns
python python/util.py csv_unzip corr_$k.$i$j.train $tr_1 $tr_2
python python/util.py csv_unzip $sim $sim_1 $sim_2

# Run on left column
./covq/covq \
--train $tr_1 $cb $cw \
--test $sim_1 $cb $cw $out_1 \
--dim 1 --nsplits $nsplits --bep $bep --lbg-eps $lbg_eps --cv-disp $cv_disp

if [ $? -ne 0 ]
    then echo "Returned non-zero status"
    exit 1
fi

# Run on right column
./covq/covq \
--train $tr_2 $cb $cw \
--test $sim_2 $cb $cw $out_2 \
--dim 1 --nsplits $nsplits --bep $bep --lbg-eps $lbg_eps --cv-disp $cv_disp

if [ $? -ne 0 ]
    then echo "Returned non-zero status"
    exit 1
fi

# Zip the results back into 2 column csv
python python/util.py csv_zip $out_1 $out_2 corr_$k.$i$j.out

# Write rho to out file
echo -n $k.$i$j >> $snr_out
echo -n ', ' >> $snr_out
# Get python to measure SNR
python python/run.py get_snr corr_$k.$i$j.sim corr_$k.$i$j.out >> $snr_out

# Clean up
rm $cw $cb $out_1 $out_2 $sim_1 $sim_2 $tr_1 $tr_2