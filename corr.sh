#!/usr/bin/bash
make covq

for i in {0..9}
do
for j in {0..9}
do
    python python/util.py csv_unzip dropbox/datasets/mydatasets/train_0.$i$j.csv tr_left.csv tr_right.csv
    python python/util.py csv_unzip dropbox/datasets/mydatasets/sim_0.$i$j.csv test_left.csv test_right.csv
    ./covq/covq
done
done
python python/util.py csv_unzip dropbox/datasets/mydatasets/train_1.00.csv tr_left.csv tr_right.csv
python python/util.py csv_unzip dropbox/datasets/mydatasets/sim_1.00.csv test_left.csv test_right.csv
./covq/covq