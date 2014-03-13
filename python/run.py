import subprocess
from image import ster2csv, csv_zip, csv2ster, csv_unzip
bep = 0.001
dim = 1
rate = 4
seed = 1234
# for eps in range(0, 0.2, 0.001):
#     subprocess.call(["./covq/covq", "--test train.csv", "--bep" + str(eps)])

# Convert images to 1D csvs
(dim_x, dim_y) = ster2csv("left.png", "right.png", "l.csv", "r.csv")


# Train independent covq on each csv
subprocess.check_call(["../covq/covq --train l.csv cb_l.csv cwmap_l.csv " +
                       " --bep " + str(bep) + " --dim " + str(dim) +
                       " --nsplits " + str(rate) + " --seed " + str(seed)],
                      stderr=subprocess.STDOUT, shell=True)
subprocess.check_call(["../covq/covq --train r.csv cb_r.csv cwmap_r.csv " +
                       " --bep " + str(bep) + " --dim " + str(dim) +
                       " --nsplits " + str(rate) + " --seed " + str(seed)],
                      stderr=subprocess.STDOUT, shell=True)

# Zip the two csvs
csv_zip("l.csv", "r.csv", "both.csv")

# Train on vector covq
subprocess.check_call(["../covq/covq --train both.csv cb_both.csv cwmap_both.csv" +
                       " --bep " + str(bep) + " --dim " + str(2*dim) + " --nsplits " +
                       str(rate+1) + " --seed " + str(seed)], shell=True)

# TODO: Here, train on 2-source covq

# Test on same data sets, output test data
# independent covq
subprocess.check_call(" ".join(["../covq/covq", "--test l.csv cb_l.csv cwmap_l.csv t_out_l.csv",
                 "--bep", str(bep), "--dim", str(dim), "--seed", str(seed)]), shell=True)
subprocess.check_call(" ".join(["../covq/covq", "--test r.csv cb_r.csv cwmap_r.csv t_out_r.csv",
                 "--bep", str(bep), "--dim", str(dim), "--seed", str(seed)]), shell=True)

# Test on vector covq
subprocess.check_call(" ".join(["../covq/covq", "--test both.csv cb_both.csv cwmap_both.csv t_out_both.csv",
                 "--bep", str(bep), "--dim", str(dim), "--seed", str(seed)]), shell=True)

# TODO: Here, test on 2-source covq

# Unzip vector covq results
csv_unzip("t_out_both.csv", "t_out_both_l.csv", "t_out_both_r.csv")

# TODO: Here, unzip 2-source covq results

# Convert csvs to pngs

csv2ster("t_out_l.csv", "t_out_r.csv", "ind_l.png", "ind_r.png", dim_x, dim_y)
csv2ster("t_out_both_l.csv", "t_out_both_r.csv", "vec_l.png", "vec_r.png", dim_x, dim_y)

# TODO: Here, convert 2-source covq results to png