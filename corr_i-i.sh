#!/usr/bin/bash
make covq

# Parameters
snr_out=snr_i-i.out
nsplits=2
bep=0.0
lbg_eps=0.001
cv_disp=0.0001



# Clear SNR output file
rm -f $snr_out

k=0
for i in {0..9}
do
    for j in 0 5
    do
        # Split the sets into single columns
        python python/util.py csv_unzip corr_$k.$i$j.train tr_1.csv tr_2.csv
        python python/util.py csv_unzip corr_$k.$i$j.sim sim_1.csv sim_2.csv
        
        # Run on left column
        ./covq/covq \
        --train tr_1.csv cb_out.csv cw_map_out.csv \
        --test sim_1.csv cb_out.csv cw_map_out.csv sim_1.out \
        --dim 1 --nsplits $nsplits --bep $bep --lbg-eps $lbg_eps --cv-disp $cv_disp

        if [ $? -ne 0 ]
            then echo "Returned non-zero status"
            exit
        fi

        # Run on right column
        ./covq/covq \
        --train tr_2.csv cb_out.csv cw_map_out.csv \
        --test sim_2.csv cb_out.csv cw_map_out.csv sim_2.out \
        --dim 1 --nsplits $nsplits --bep $bep --lbg-eps $lbg_eps --cv-disp $cv_disp

        if [ $? -ne 0 ]
            then echo "Returned non-zero status"
            exit
        fi

        # Zip the results back into 2 column csv
        python python/util.py csv_zip sim_1.out sim_2.out corr_$k.$i$j.out
        
        # Write rho to out file
        echo -n $k.$i$j >> $snr_out
        echo -n ', ' >> $snr_out
        # Get python to measure SNR
        python python/run.py get_snr corr_$k.$i$j.sim corr_$k.$i$j.out >> $snr_out
    done
done

k=1
i=0
j=0

# Split the sets into single columns
python python/util.py csv_unzip corr_$k.$i$j.train tr_1.csv tr_2.csv
python python/util.py csv_unzip corr_$k.$i$j.sim sim_1.csv sim_2.csv

# Run on left column
./covq/covq \
--train tr_1.csv cb_out.csv cw_map_out.csv \
--test sim_1.csv cb_out.csv cw_map_out.csv sim_1.out \
--dim 1 --nsplits $nsplits --bep $bep --lbg-eps $lbg_eps --cv-disp $cv_disp

if [ $? -ne 0 ]
    then echo "Returned non-zero status"
    exit
fi

# Run on right column
./covq/covq \
--train tr_2.csv cb_out.csv cw_map_out.csv \
--test sim_2.csv cb_out.csv cw_map_out.csv sim_2.out \
--dim 1 --nsplits $nsplits --bep $bep --lbg-eps $lbg_eps --cv-disp $cv_disp

if [ $? -ne 0 ]
    then echo "Returned non-zero status"
    exit
fi

# Zip the results back into 2 column csv
python python/util.py csv_zip sim_1.out sim_2.out corr_$k.$i$j.out

# Write rho to out file
echo -n $k.$i$j >> $snr_out
echo -n ', ' >> $snr_out
# Get python to measure SNR
python python/run.py get_snr corr_$k.$i$j.sim corr_$k.$i$j.out >> $snr_out

if [ $i == 9 ] && [ $j == 9 ]
    then k=1
fi