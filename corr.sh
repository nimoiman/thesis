#!/usr/bin/bash
make covq

for i in {1..9}
do
    python python/image.py csv_unzip training_set_$i.csv tr_left.csv tr_right.csv
    python python/image.py csv_unzip testing_set_$i.csv test_left.csv test_right.csv
    ./covq/covq
done