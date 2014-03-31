#!/usr/bin/bash
# Run through correlation sets for d-d system
make covq

# Parameters
snr_out=snr_d-d.out
nsplits=4
bep=0.0
lbg_eps=0.001
cv_disp=0.0001

# Clear SNR output file
rm -f $snr_out

k=0
for i in {0..9}
do
    for j in 5
    do
        # Run on sets
        ./covq/covq \
        --train corr_$k.$i$j.train cb_out.csv cw_map_out.csv \
        --test corr_$k.$i$j.sim cb_out.csv cw_map_out.csv corr_$k.$i$j.out \
        --dim 2 --nsplits $nsplits --bep $bep --lbg-eps $lbg_eps --cv-disp $cv_disp

        if [ $? -ne 0 ]
            then echo "Returned non-zero status"
            exit
        fi

        # Write rho to out file
        echo -n $k.$i$j >> $snr_out
        echo -n ', ' >> $snr_out
        # Get python to measure SNR
        python python/run.py get_snr corr_$k.$i$j.sim corr_$k.$i$j.out >> $snr_out

        if [ $i == 9 ] && [ $j == 9 ]
            then k=1
        fi
    done
done

k=1
i=0
j=0
# Run on sets for 1.00
./covq/covq \
--train corr_$k.$i$j.train cb_out.csv cw_map_out.csv \
--test corr_$k.$i$j.sim cb_out.csv cw_map_out.csv corr_$k.$i$j.out \
--dim 2 --nsplits $nsplits --bep $bep --lbg-eps $lbg_eps --cv-disp $cv_disp

if [ $? -ne 0 ]
    then echo "Returned non-zero status"
    exit
fi

# Write rho to out file
echo -n $k.$i$j >> $snr_out
echo -n ', ' >> $snr_out
# Get python to measure SNR
python python/run.py get_snr corr_$k.$i$j.sim corr_$k.$i$j.out >> $snr_out

if [ $i == 9 ] && [ $j == 9 ]
    then k=1
fi

