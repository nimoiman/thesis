#!/usr/bin/bash
make covq_2

for i in {0..9}
do
for j in {0..9}
do
    # python python/util.py csv_unzip dropbox/datasets/mydatasets/train_0.$i$j.csv tr_left.csv tr_right.csv
    # python python/util.py csv_unzip dropbox/datasets/mydatasets/sim_0.$i$j.csv test_left.csv test_right.csv
    cp dropbox/datasets/mydatasets/train_0.$i$j.csv train.csv
    # python python/plot.py scatter_2d train.csv &
    if [ @! ]
        then ./covq_2/covq_2 train.csv train.csv output.csv 0.$i$j
    else echo whoops
    fi
done
done
# python python/util.py csv_unzip dropbox/datasets/mydatasets/train_1.00.csv tr_left.csv tr_right.csv
# python python/util.py csv_unzip dropbox/datasets/mydatasets/sim_1.00.csv test_left.csv test_right.csv
# ./covq/covq
cp dropbox/datasets/mydatasets/train_1.00.csv train.csv
./covq_2/covq_2 train.csv train.csv output.csv 1.00