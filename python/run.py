import subprocess
import os
import numpy as np
from image import ster2csv, csv_zip, csv2ster, csv_unzip, csv_read, \
    bit_allocate, iter_array
bep = 0.001
dim = 1
rate = 4
seed = 1234
block_size = 8
train_dir = "training"
# for eps in range(0, 0.2, 0.001):
#     subprocess.call(["./covq/covq", "--test train.csv", "--bep" + str(eps)])

# Convert images to 1D csvs of DCT coefficients
(dim_x, dim_y) = ster2csv("left.png", "right.png", "l.csv", "r.csv")

# Convert to (block_size * block_size) sources (one for each type of
# coefficient)
left = csv_read("l.csv", dim_x, dim_y)
right = csv_read("r.csv", dim_x, dim_y)

# Find optimal bit allocation
# bit_alloc = bit_allocate("l.csv", "r.csv", rate, dim_x, dim_y, block_size=8)
bit_alloc = np.array([[11, 8, 7, 6, 5, 4, 3, 3],
                      [8, 7, 6, 5, 4, 3, 3, 3],
                      [8, 6, 5, 4, 4, 3, 3, 2],
                      [7, 5, 5, 4, 3, 3, 3, 2],
                      [6, 5, 4, 4, 3, 3, 2, 2],
                      [6, 4, 4, 3, 3, 3, 2, 2],
                      [5, 4, 3, 3, 3, 2, 2, 2],
                      [5, 4, 3, 3, 2, 2, 2, 2]], dtype=int)


# mkdir for training sets
if not os.path.isdir(train_dir):
    os.mkdir(train_dir)

# make covq program
os.chdir("../")
subprocess.check_call("make clean", shell=True)
subprocess.check_call("make covq", shell=True)
os.chdir("python/")

for i in range(block_size):
    for j in range(block_size):
        if not os.path.isdir(os.path.join(train_dir, str(i)+str(j))):
            os.mkdir(os.path.join(train_dir, str(i)+str(j)))

        # Write DCT coefficients i,j to csv files for training
        n = 0
        with open("_tmp_l.csv", 'w') as f:
            for block in iter_array(left, (block_size, block_size)):
                f.write(str(block[i, j]) + "\n")
                n += 1
        with open("_tmp_r.csv", 'w') as f:
            for block in iter_array(right, (block_size, block_size)):
                f.write(str(block[i, j]) + "\n")

        print("training on DCT coefficient ({},{})".format(i, j))
        print("with {} bits per sample".format(bit_alloc[i,j]))

        # Train independent covq on each csv
        subprocess.check_call(["time", "../covq/covq", "--train", "_tmp_l.csv",
                               os.path.join(train_dir, str(i)+str(j),
                                             "cb_l.csv"),
                               os.path.join(train_dir, str(i)+str(j),
                                             "cwmap_l.csv"),
                               "--bep", str(bep), "--dim", str(dim),
                               "--nsplits", str(bit_alloc[i,j]),
                               "--seed", str(seed)])
        subprocess.check_call(["time", "../covq/covq", "--train", "_tmp_r.csv",
                               os.path.join(train_dir, str(i)+str(j),
                                             "cb_r.csv"),
                               os.path.join(train_dir, str(i)+str(j),
                                             "cwmap_r.csv"),
                               "--bep", str(bep), "--dim", str(dim),
                               "--nsplits", str(bit_alloc[i,j]),
                               "--seed", str(seed)])

        # Zip the two csvs
        csv_zip("_tmp_l.csv", "_tmp_r.csv", "_tmp_both.csv")

        # Train on vector covq
        subprocess.check_call(["time", "../covq/covq", "--train", "_tmp_both.csv",
                               os.path.join(train_dir, str(i)+str(j),
                                             "cb_both.csv"),
                               os.path.join(train_dir, str(i)+str(j),
                                             "cwmap_both.csv"),
                               "--bep", str(bep), "--dim", str(2*dim),
                               "--nsplits", str(bit_alloc[i,j]*2),
                               "--seed", str(seed)])

        # TODO: Here, train on 2-source covq

# Cleanup tmp files
[os.remove(p) for p in ("_tmp_l.csv", "_tmp_r.csv", "_tmp_both.csv")]

# # Test on same data sets, output test data
# # independent covq
# subprocess.check_call(["../covq/covq", "--test l.csv", "cb_l.csv",
#                        "cwmap_l.csv", "t_out_l.csv", "--bep", str(bep),
#                        "--dim", str(dim), "--seed", str(seed)], shell=True)
# subprocess.check_call(["../covq/covq", "--test", "r.csv", "cb_r.csv",
#                        "cwmap_r.csv t_out_r.csv", "--bep", str(bep),
#                        "--dim", str(dim), "--seed", str(seed)], shell=True)

# # Test on vector covq
# subprocess.check_call(["../covq/covq", "--test", "both.csv", "cb_both.csv",
#                        "cwmap_both.csv", "t_out_both.csv", "--bep", str(bep),
#                        "--dim", str(dim), "--seed", str(seed)], shell=True)

# # TODO: Here, test on 2-source covq

# # Unzip vector covq results
# csv_unzip("t_out_both.csv", "t_out_both_l.csv", "t_out_both_r.csv")

# # TODO: Here, unzip 2-source covq results

# # Convert csvs to pngs

# csv2ster("t_out_l.csv", "t_out_r.csv", "ind_l.png", "ind_r.png", dim_x, dim_y)
# csv2ster("t_out_both_l.csv", "t_out_both_r.csv", "vec_l.png", "vec_r.png", dim_x, dim_y)

# TODO: Here, convert 2-source covq results to png