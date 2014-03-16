from api_docs import command, parse_args, help
import subprocess
import os
import numpy as np
from image import ster2csv, csv_zip, csv2ster, csv_unzip, csv_read, \
    bit_allocate, iter_array, lala
import pickle
bep = 0.001
vector_dim = 1
rate = 1
seed = 1234
block_size = 8
train_dir = "training"
test_dir = "testing"
# lbg_eps = variance / lbg_granularity
lbg_granularity = 10
# for eps in range(0, 0.2, 0.001):
#     subprocess.call(["./covq/covq", "--test train.csv", "--bep" + str(eps)])

@command
def train(*args):
    """Trains systems on pairs of pngs in *args"""
    if len(args) % 2 != 0:
        print("odd number of filenames not acceptable")
        raise SystemExit(1)
    else:
        # pair up the images
        imgs = []
        for i, f in enumerate(args):
            if i % 2 == 0:
                f_1 = f
            else:
                imgs.append((f_1, f))

    # make covq program
    os.chdir("../")
    subprocess.check_call("make clean", shell=True)
    subprocess.check_call("make covq", shell=True)
    os.chdir("python/")

    dims = []
    for idx, img_pair in enumerate(imgs):
        # Convert images to 1D csvs of DCT coefficients
        if idx == 0:
            dim = ster2csv(img_pair[0], img_pair[1], "l.csv", "r.csv")
        else:
            dim = ster2csv(img_pair[0], img_pair[1], "l.csv", "r.csv",
                           True)
        dims.append(dim)
    # Convert to (block_size * block_size) sources (one for each type of
    # coefficient)
    print("in run.py dims={}".format(dims))
    left = csv_read("l.csv", dims)
    print("in run.py dims={}".format(dims))
    right = csv_read("r.csv", dims)
    print("in run.py dims={}".format(dims))

    # Find optimal bit allocation
    bit_alloc, var_block = bit_allocate("l.csv", "r.csv", rate, dims, block_size=8)

    print("DCT Coefficient Variance Matrix:")
    print(var_block)
    print("DCT Coefficient Bit Allocation Matrix:")
    print(bit_alloc)

    # write bit_alloc to file
    with open(os.path.join(train_dir, "bit_alloc.pickle"), "wb") as f:
        import code
        code.interact(local=locals())
        pickle.dump(bit_alloc, f)

    # mkdir for training sets
    if not os.path.isdir(train_dir):
        os.mkdir(train_dir)

    for i in range(block_size):
        for j in range(block_size):
            if not os.path.isdir(os.path.join(train_dir, str(i)+str(j))):
                os.mkdir(os.path.join(train_dir, str(i)+str(j)))

            # Write DCT coefficients i,j to csv files for training
            n = 0
            for el in left:
                with open("_tmp_l.csv", 'w') as f:
                    for block in iter_array(el, (block_size, block_size)):
                        f.write(str(block[i, j]) + "\n")
                        n += 1
            for r in right:
                with open("_tmp_r.csv", 'w') as f:
                    for block in iter_array(r, (block_size, block_size)):
                        f.write(str(block[i, j]) + "\n")

            print("training on DCT coefficient ({},{})".format(i, j))
            print("with {} bits per sample".format(bit_alloc[i,j]))

            # Train independent covq on each csv
            subprocess.check_call(["time", "../covq/covq", "--train", "_tmp_l.csv",
                                   os.path.join(train_dir, str(i)+str(j),
                                                "cb_l.csv"),
                                   os.path.join(train_dir, str(i)+str(j),
                                                "cwmap_l.csv"),
                                   "--bep", str(bep), "--dim", str(vector_dim),
                                   "--nsplits", str(bit_alloc[i,j]),
                                   "--seed", str(seed), "--lbg-eps",
                                   str(var_block[i,j]/lbg_granularity)])
            subprocess.check_call(["time", "../covq/covq", "--train", "_tmp_r.csv",
                                   os.path.join(train_dir, str(i)+str(j),
                                                "cb_r.csv"),
                                   os.path.join(train_dir, str(i)+str(j),
                                                "cwmap_r.csv"),
                                   "--bep", str(bep), "--dim", str(vector_dim),
                                   "--nsplits", str(bit_alloc[i,j]),
                                   "--seed", str(seed), "--lbg-eps",
                                   str(var_block[i,j]/lbg_granularity)])

            # Zip the two csvs
            # csv_zip("_tmp_l.csv", "_tmp_r.csv", "_tmp_both.csv")

            # Train on vector covq
            # subprocess.check_call(["time", "../covq/covq", "--train", "_tmp_both.csv",
            #                        os.path.join(train_dir, str(i)+str(j),
            #                                      "cb_both.csv"),
            #                        os.path.join(train_dir, str(i)+str(j),
            #                                      "cwmap_both.csv"),
            #                        "--bep", str(bep), "--dim", str(2*dim),
            #                        "--nsplits", str(bit_alloc[i,j]*2),
            #                        "--seed", str(seed), "--lbg-eps",
            #                        str(var_block[i,j]/lbg_granularity)])

            # TODO: Here, train on 2-source covq

    # Cleanup tmp files
    [os.remove(p) for p in ("_tmp_l.csv", "_tmp_r.csv")] #, "_tmp_both.csv")]

@command
def test(*args):
    """Accept png filenames in pairs"""
    if len(args) % 2 != 0:
        print("odd number of filenames not acceptable")
        raise SystemExit(1)
    else:
        # pair up the images
        imgs = []
        for i, f in enumerate(args):
            if i % 2 == 0:
                f_1 = f
            else:
                imgs.append((f_1, f))

    # make covq program
    os.chdir("../")
    subprocess.check_call("make clean", shell=True)
    subprocess.check_call("make covq", shell=True)
    os.chdir("python/")

    dims = []
    for idx, img_pair in enumerate(imgs):
        # Convert images to 1D csvs of DCT coefficients
        if idx == 0:
            (dim_x, dim_y) = ster2csv(img_pair[0], img_pair[1], "l.csv", "r.csv")
        else:
            (dim_x, dim_y) = ster2csv(img_pair[0], img_pair[1], "l.csv", "r.csv",
                                      True)
        dims.append((dim_x, dim_y))
    # Convert to (block_size * block_size) sources (one for each type of
    # coefficient)
    left = csv_read("l.csv", dims)
    right = csv_read("r.csv", dims)

    # mkdir for testing sets
    if not os.path.isdir(test_dir):
        os.mkdir(test_dir)

    with open(os.path.join(train_dir, "bit_alloc.pickle"), "rb") as f:
        bit_alloc = pickle.load(f)

    for i in range(block_size):
        for j in range(block_size):

            if not os.path.isdir(os.path.join(test_dir, str(i)+str(j))):
                os.mkdir(os.path.join(test_dir, str(i)+str(j)))

            # Write DCT coefficients i,j to csv files for testing
            n = 0
            for idx, dim in enumerate(dims):
                if idx == 0:
                    mode = 'w'
                else:
                    mode = 'a'

                with open("_tmp_l.csv", mode) as f:
                    for block in iter_array(left[idx], (block_size, block_size)):
                        f.write(str(block[i, j]) + "\n")
                        n += 1
                with open("_tmp_r.csv", mode) as f:
                    for block in iter_array(right[idx], (block_size, block_size)):
                        f.write(str(block[i, j]) + "\n")

            print("testing on DCT coefficient ({},{})".format(i, j))
            print("with {} bits per sample".format(bit_alloc[i,j]))

            # Test on same data sets, output test data
            # independent covq
            subprocess.check_call(["time", "../covq/covq", "--test", "_tmp_l.csv",
                                   os.path.join(train_dir, str(i)+str(j),
                                                "cb_l.csv"),
                                   os.path.join(train_dir, str(i)+str(j),
                                                "cwmap_l.csv"),
                                   os.path.join(test_dir, str(i)+str(j),
                                                "t_out_l.csv"),
                                   "--bep", str(bep), "--dim", str(vector_dim), "--seed",
                                   str(seed)])
            subprocess.check_call(["time", "../covq/covq", "--test", "_tmp_r.csv",
                                   os.path.join(train_dir, str(i)+str(j),
                                                "cb_r.csv"),
                                   os.path.join(train_dir, str(i)+str(j),
                                                "cwmap_r.csv"),
                                   os.path.join(test_dir, str(i)+str(j),
                                                "t_out_r.csv"),
                                   "--bep", str(bep), "--dim", str(vector_dim), "--seed",
                                   str(seed)])

            # Test on vector covq
            # subprocess.check_call(["../covq/covq", "--test", "both.csv", "cb_both.csv",
            #                        "cwmap_both.csv", "t_out_both.csv", "--bep", str(bep),
            #                        "--dim", str(dim), "--seed", str(seed)], shell=True)

            # TODO: Here, test on 2-source covq

            # Unzip vector covq results
            # csv_unzip("t_out_both.csv", "t_out_both_l.csv", "t_out_both_r.csv")

            # TODO: Here, unzip 2-source covq results

    # Join the separate files into single column csv per image
    
    for img_pair in imgs:
        left_dct = []
        right_dct = []
        for i in range(block_size):
            left_dct.append([])
            for j in range(block_size):
                left_dct[i].append([])
                with open(os.path.join(test_dir, str(i)+str(j), "t_out_l.csv")) as f:
                    for n_x in range(int(dim_x / block_size)):
                        left_dct[i][j].append([])
                        for n_y in range(int(dim_y / block_size)):
                            left_dct[i][j][n_x].append(float(f.readline()))

        for i in range(block_size):
            right_dct.append([])
            for j in range(block_size):
                right_dct[i].append([])
                with open(os.path.join(test_dir, str(i)+str(j), "t_out_r.csv")) as f:
                    for n_x in range(int(dim_x / block_size)):
                        right_dct[i][j].append([])
                        for n_y in range(int(dim_y / block_size)):
                            right_dct[i][j][n_x].append(float(f.readline()))

        left_im = []
        right_im = []
        for n_x in range(int(dim_x / block_size)):
            for i in range(block_size):
                for n_y in range(int(dim_y / block_size)):
                    for j in range(block_size):
                        left_im.append(left_dct[i][j][n_x][n_y])
                        right_im.append(right_dct[i][j][n_x][n_y])

        if idx == 0:
            mode = 'w'
        else:
            mode = 'a'
        with open(os.path.join(test_dir, "t_out_r.csv"), mode) as f:
            f.writelines([str(n) + "\n" for n in right_im])
        with open(os.path.join(test_dir, "t_out_l.csv"), mode) as f:
            f.writelines([str(n) + "\n" for n in left_im])

    # Convert csvs to pngs

    csv2ster(os.path.join(test_dir, "t_out_l.csv"),
             os.path.join(test_dir, "t_out_r.csv"),
             ["{}_t_out.png".format(os.path.splitext(img_pair[0])[0])
              for img_pair in imgs],
             ["{}_t_out.png".format(os.path.splitext(img_pair[1])[0])
              for img_pair in imgs],
             dims)
    # csv2ster("t_out_both_l.csv", "t_out_both_r.csv", "vec_l.png", "vec_r.png", dim_x, dim_y)

    # TODO: Here, convert 2-source covq results to png

if __name__ == '__main__':
    parse_args()