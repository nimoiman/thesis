#!/bin/bash

N_X=8
N_Y=8
X_MIN=-4
X_MAX=4
Y_MIN=-4
Y_MAX=4

for i in {0..9}
do
    ./covq_2/covq_2 $N_X $N_Y $X_MIN $X_MAX $Y_MIN $Y_MAX corr_0.0$i
done
for i in {10..99}
do
    ./covq_2/covq_2 $N_X $N_Y $X_MIN $X_MAX $Y_MIN $Y_MAX corr_0.$i
done
./covq_2/covq_2 $N_X $N_Y $X_MIN $X_MAX $Y_MIN $Y_MAX corr_1.00
