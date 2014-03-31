#!/usr/bin/bash
# Run through correlation sets for d-d system
make covq

# Parameters
nsplits=2
bep=0.0
lbg_eps=0.001
cv_disp=0.0001

# tmp files
suff=d-d
snr_out=snr_$suff.out
cb=cb_$suff.csv
cw=cw_map_$suff.csv

# Clear SNR output file
rm -f $snr_out

k=0
for i in {0..9}
do
    for j in 5
    do
        train=corr_$k.$i$j.train
        sim=corr_$k.$i$j.sim
        if [ ! -f $train ] || [ ! -f $sim ]
            then echo "Cannot find train/sim files $train, $sim; rho=$k.$i$j"
            exit 1
        fi

        # Run on sets
        ./covq/covq \
        --train $train $cb $cw \
        --test $sim $cb $cw corr_$k.$i$j_$suff.out \
        --dim 2 --nsplits $nsplits --bep $bep --lbg-eps $lbg_eps --cv-disp $cv_disp

        if [ $? -ne 0 ]
            then echo "Returned non-zero status"
            exit 1
        fi

        # Write rho to out file
        echo -n $k.$i$j >> $snr_out
        echo -n ', ' >> $snr_out
        # Get python to measure SNR
        python python/run.py get_snr $sim corr_$k.$i$j_$suff.out >> $snr_out

        if [ $i == 9 ] && [ $j == 9 ]
            then k=1
        fi
    done
done

k=1
i=0
j=0

if [ ! -f $train ] || [ ! -f $sim ]
    then echo "Cannot find train/sim files $train, $sim; rho=$k.$i$j"
    exit 1
fi

# Run on sets for 1.00
./covq/covq \
--train $train $cb $cw \
--test $sim $cb $cw corr_$k.$i$j_$suff.out \
--dim 2 --nsplits $nsplits --bep $bep --lbg-eps $lbg_eps --cv-disp $cv_disp

if [ $? -ne 0 ]
    then echo "Returned non-zero status"
    exit 1
fi

# Write rho to out file
echo -n $k.$i$j >> $snr_out
echo -n ', ' >> $snr_out
# Get python to measure SNR
python python/run.py get_snr $sim corr_$k.$i$j_$suff.out >> $snr_out

if [ $i == 9 ] && [ $j == 9 ]
    then k=1
fi

# clean up
rm $cw $cb
