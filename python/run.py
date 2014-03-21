#!/usr/bin/env python3
"""Python command line wrapper for running COVQ tests on images"""

# Python 2 compat
from __future__ import print_function

import numpy as np
import subprocess
import pickle
import traceback
import os, sys

from api_docs import command, parse_args
from image import *
from util import *

# Filenames
csv_src_l = "l.csv"
csv_src_r = "r.csv"
bit_allocs_pickle = "bit_allocs.pickle"
tmp_l = "_tmp_l.csv"
tmp_r = "_tmp_r.csv"
tmp_both = "_tmp_both.csv"
cb_1D_l = "cb_l.csv"
cb_1D_r = "cb_r.csv"
cwmap_1D_l = "cwmap_l.csv"
cwmap_1D_r = "cwmap_r.csv"
cb_2D_both = "cb_2D_both.csv"
cb_2D_l = "cb_l.csv"
cb_2D_r = "cb_r.csv"
cwmap_2D_both = "cwmap_2D_both.csv"
cwmap_2D_l = "cwmap_l.csv"
cwmap_2D_r = "cwmap_r.csv"
t_out_1D_l = "t_out_1D_l.csv"
t_out_1D_r = "t_out_1D_r.csv"
t_out_2D_both = "t_out_2D_both.csv"
t_out_2D_l = "t_out_2D_l.csv"
t_out_2D_r = "t_out_2D_r.csv"


@command
def png_train_1d(*pngs, bep=0.001, block_rate=256, seed=1234,
        lbg_eps=0.001, train_dir="training_1d", block_size=8, make=True):
    """Train 2*1D COSQ on DCT coefficients of pairs of pngs in *pngs
    Store training output in :param:`train_dir`

    e.g.

    :code::
        python run.py png_train_1d left_1.png right_1.png left_2.png right_2.png
    """
    # COSQ Parameters
    bep = float(bep)
    vector_dim = 1
    block_rate = int(block_rate)
    seed = int(seed)
    lbg_eps = float(lbg_eps)
    block_size = int(block_size)
    if make in ['True', True]:
        make = True
    elif make in ['False', False]:
        make = False
    else:
        stderr_("Invalid value for kwarg 'make'")
        raise SystemExit(1)

    if len(pngs) % 2 != 0:
        stderr_("Please provide training images in pairs")
        raise SystemExit(1)

    # Pair up the images
    imgs = zip(pngs[0::2], pngs[1::2])
    if make:
        # Make COVQ program
        os.chdir("../")
        subprocess.check_call("make clean", shell=True)
        subprocess.check_call("make covq", shell=True)
        os.chdir("python/")

    dims = []
    for idx, img_pair in enumerate(imgs):
        # Convert images to 1 column csvs of DCT coefficients
        if idx == 0:
            dim = ster2csv(img_pair[0], img_pair[1], csv_src_l, csv_src_r,
                           mode='w')
        else:
            dim = ster2csv(img_pair[0], img_pair[1], csv_src_l, csv_src_r,
                           mode='a')
        # Store the image pair dimensions in dims
        dims.append(dim)
    # Convert to (block_size * block_size) sources (one for each type of
    # coefficient)
    left = csv_read(csv_src_l, dims)
    right = csv_read(csv_src_r, dims)

    # Find optimal bit allocation for left, right images independently
    # See Julien Cheng's MSc Ch. 3
    bit_allocs, var_blocks = bit_allocate(csv_src_l, csv_src_r, block_rate,
                                          dims, block_size=block_size)

    stderr_("DCT Coefficient Bit Allocation Matrix Left:")
    stderr_(bit_allocs[0])
    stderr_("DCT Coefficient Bit Allocation Matrix Right:")
    stderr_(bit_allocs[1])

    # mkdir for training sets
    mkdir_(train_dir)

    # Write bit_allocs to binary pickled file for use in testing code
    with open(os.path.join(train_dir, bit_allocs_pickle), "wb") as f:
        pickle.dump(bit_allocs, f)

    # Send each DCT frequency into the COSQ training code as a set
    for i in range(block_size):
        for j in range(block_size):
            # mkdir train_dir/ij
            # This dir is for storing the results of the training phase
            # namely, the codebook and codeword map for DCT frequency ij
            mkdir_(os.path.join(train_dir, str(i)+str(j)))

            # Write DCT coefficients i,j to tmp csv files for training
            n = 0
            for el in left:
                with open(tmp_l, 'w') as f:
                    for block in iter_array(el, (block_size, block_size)):
                        f.write(str(block[i, j]) + "\n")
                        n += 1
            for r in right:
                with open(tmp_r, 'w') as f:
                    for block in iter_array(r, (block_size, block_size)):
                        f.write(str(block[i, j]) + "\n")

            stderr_("training on DCT coefficient ({},{})".format(i, j))
            stderr_("with {} bits per sample".format(bit_allocs[0][i,j]))

            # Train independent covq on each csv
            subprocess.check_call(["../covq/covq", "--train", tmp_l,
                                   os.path.join(train_dir, str(i)+str(j),
                                                cb_1D_l),
                                   os.path.join(train_dir, str(i)+str(j),
                                                cwmap_1D_l),
                                   "--bep", str(bep), "--dim", str(vector_dim),
                                   "--nsplits", str(bit_allocs[0][i,j]),
                                   "--seed", str(seed), "--lbg-eps",
                                   str(lbg_eps)])

            stderr_("training on DCT coefficient ({},{})".format(i, j))
            stderr_("with {} bits per sample".format(bit_allocs[1][i,j]))
            subprocess.check_call(["../covq/covq", "--train", "_tmp_r.csv",
                                   os.path.join(train_dir, str(i)+str(j),
                                                cb_1D_r),
                                   os.path.join(train_dir, str(i)+str(j),
                                                cwmap_1D_r),
                                   "--bep", str(bep), "--dim", str(vector_dim),
                                   "--nsplits", str(bit_allocs[1][i,j]),
                                   "--seed", str(seed), "--lbg-eps",
                                   str(lbg_eps)])


    # Cleanup tmp files
    [os.remove(p) for p in (tmp_l, tmp_r)]

@command
def png_test_1d(*pngs, test_dir="testing_1d", train_dir="training_1d", block_size=8,
                bep=0.001, seed=1234):
    """Accept png filenames in pairs"""
    if len(pngs) % 2 != 0:
        print("odd number of pngs not acceptable")
        raise SystemExit(1)
    if len(pngs) == 0:
        print("Too few pngs provided!")
        raise SystemExit(1)

    vector_dim = 1

    # Pair up the images
    imgs = list(zip(pngs[0::2], pngs[1::2]))

    # make covq program
    os.chdir("../")
    subprocess.check_call("make clean", shell=True)
    subprocess.check_call("make covq", shell=True)
    os.chdir("python/")

    dims = []
    for idx, img_pair in enumerate(imgs):
        # Convert images to 1D csvs of DCT coefficients
        if idx == 0:
            (dim_x, dim_y) = ster2csv(img_pair[0], img_pair[1], csv_src_l, csv_src_r)
        else:
            (dim_x, dim_y) = ster2csv(img_pair[0], img_pair[1], csv_src_l, csv_src_r,
                                      'a')
        dims.append((dim_x, dim_y))
    # Convert to (block_size * block_size) sources (one for each type of
    # coefficient)
    left = csv_read(csv_src_l, dims)
    right = csv_read(csv_src_r, dims)

    # mkdir for testing sets
    if not os.path.isdir(test_dir):
        os.mkdir(test_dir)

    with open(os.path.join(train_dir, bit_allocs_pickle), "rb") as f:
        bit_allocs = pickle.load(f)

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

                with open(tmp_l, mode) as f:
                    for block in iter_array(left[idx], (block_size, block_size)):
                        f.write(str(block[i, j]) + "\n")
                        n += 1
                with open(tmp_r, mode) as f:
                    for block in iter_array(right[idx], (block_size, block_size)):
                        f.write(str(block[i, j]) + "\n")

            print("testing on DCT coefficient ({},{})".format(i, j))
            print("with {} bits per sample".format(bit_allocs[0][i,j]))

            # Test on same data sets, output test data
            # independent covq
            subprocess.check_call(["../covq/covq", "--test", tmp_l,
                                   os.path.join(train_dir, str(i)+str(j),
                                                cb_1D_l),
                                   os.path.join(train_dir, str(i)+str(j),
                                                cwmap_1D_l),
                                   os.path.join(test_dir, str(i)+str(j),
                                                t_out_1D_l),
                                   "--bep", str(bep), "--dim", str(vector_dim), "--seed",
                                   str(seed)])

            print("testing on DCT coefficient ({},{})".format(i, j))
            print("with {} bits per sample".format(bit_allocs[1][i,j]))
            subprocess.check_call(["../covq/covq", "--test", tmp_r,
                                   os.path.join(train_dir, str(i)+str(j),
                                                cb_1D_r),
                                   os.path.join(train_dir, str(i)+str(j),
                                                cwmap_1D_r),
                                   os.path.join(test_dir, str(i)+str(j),
                                                t_out_1D_r),
                                   "--bep", str(bep), "--dim", str(vector_dim), "--seed",
                                   str(seed)])


    # Join the separate files into single column csv per image
    
    for img_pair in imgs:
        left_1D_dct = []
        right_1D_dct = []
        for i in range(block_size):
            left_1D_dct.append([])
            for j in range(block_size):
                left_1D_dct[i].append([])
                with open(os.path.join(test_dir, str(i)+str(j), t_out_1D_l)) as f:
                    for n_x in range(int(dim_x / block_size)):
                        left_1D_dct[i][j].append([])
                        for n_y in range(int(dim_y / block_size)):
                            left_1D_dct[i][j][n_x].append(float(f.readline()))


        for i in range(block_size):
            right_1D_dct.append([])
            for j in range(block_size):
                right_1D_dct[i].append([])
                with open(os.path.join(test_dir, str(i)+str(j), t_out_1D_r)) as f:
                    for n_x in range(int(dim_x / block_size)):
                        right_1D_dct[i][j].append([])
                        for n_y in range(int(dim_y / block_size)):
                            right_1D_dct[i][j][n_x].append(float(f.readline()))


        # Write 1D COSQ to file
        left_im = []
        right_im = []
        for n_x in range(int(dim_x / block_size)):
            for i in range(block_size):
                for n_y in range(int(dim_y / block_size)):
                    for j in range(block_size):
                        left_im.append(left_1D_dct[i][j][n_x][n_y])
                        right_im.append(right_1D_dct[i][j][n_x][n_y])

        if idx == 0:
            mode = 'w'
        else:
            mode = 'a'
        with open(os.path.join(test_dir, t_out_1D_l), mode) as f:
            f.writelines([str(n) + "\n" for n in left_im])
        with open(os.path.join(test_dir, t_out_1D_r), mode) as f:
            f.writelines([str(n) + "\n" for n in right_im])

    # Convert 1D COSQ csvs to pngs
    print(["{}_t_out.png".format(os.path.splitext(img_pair[0])[0])
              for img_pair in imgs])
    print(list(imgs))

    csv2ster(os.path.join(test_dir, t_out_1D_l),
             os.path.join(test_dir, t_out_1D_r),
             ["{}_t_out_1D.png".format(os.path.splitext(img_pair[0])[0])
              for img_pair in imgs],
             ["{}_t_out_1D.png".format(os.path.splitext(img_pair[1])[0])
              for img_pair in imgs], dims)


@command
def png_train_1_5d(*pngs, bep=0.001, vector_dim=1, block_rate=256, seed=1234,
        lbg_eps=0.01, train_dir="training_1_5d", block_size=8, make=True):
    """Train 2*1.5D COVQ on pairs of pngs in *pngs
    Store training output in :param:`train_dir`

    e.g.

    :code::
        python run.py train left_1.png right_1.png left_2.png right_2.png
    """
    bep = float(bep)
    vector_dim = int(vector_dim)
    block_rate = int(block_rate)
    seed = int(seed)
    lbg_eps = float(lbg_eps)
    block_size = int(block_size)
    if make in ['True', True]:
        make = True
    elif make in ['False', False]:
        make = False
    else:
        stderr_("Invalid value for kwarg 'make'")
        raise SystemExit(1)

    if len(pngs) % 2 != 0:
        stderr_("Please provide a training images in pairs")
        raise SystemExit(1)

    # Pair up the images
    imgs = zip(pngs[0::2], pngs[1::2])
    if make:
        # Make COVQ_2 program
        os.chdir("../")
        subprocess.check_call("make clean", shell=True)
        subprocess.check_call("make covq_2", shell=True)
        os.chdir("python/")

    dims = []
    for idx, img_pair in enumerate(imgs):
        # Convert images to 1 column csvs of DCT coefficients
        if idx == 0:
            dim = ster2csv(img_pair[0], img_pair[1], csv_src_l, csv_src_r,
                           mode='w')
        else:
            dim = ster2csv(img_pair[0], img_pair[1], csv_src_l, csv_src_r,
                           mode='a')
        # Store the image pair dimensions in dims
        dims.append(dim)
    # Convert to (block_size * block_size) sources (one for each type of
    # coefficient)
    left = csv_read(csv_src_l, dims)
    right = csv_read(csv_src_r, dims)

    # Find optimal bit allocation for left, right images independently
    # See Julien Cheng's MSc Ch. 3
    # TODO: derive optimal scheme for bit allocation in the 2 source case
    # - for now, bits are allocated in same way as independent scheme
    bit_allocs, var_blocks = bit_allocate(csv_src_l, csv_src_r, block_rate, dims,
                                          block_size=block_size)

    stderr_("DCT Coefficient Bit Allocation Matrix Left:")
    stderr_(bit_allocs[0])
    stderr_("DCT Coefficient Bit Allocation Matrix Right:")
    stderr_(bit_allocs[1])

    # mkdir for training sets
    mkdir_(train_dir)

    # Write bit_allocs to binary pickled file for use in testing code
    with open(os.path.join(train_dir, bit_allocs_pickle), "wb") as f:
        pickle.dump(bit_allocs, f)

    # Send each DCT frequency into the COSQ training code as a set
    for i in range(block_size):
        for j in range(block_size):
            # mkdir train_dir/ij
            # This dir is for storing the results of the training phase
            # namely, the codebook and codeword map for DCT frequency ij
            mkdir_(os.path.join(train_dir, str(i)+str(j)))

            # Write DCT coefficients i,j to tmp csv files for training
            n = 0
            for el in left:
                with open(tmp_l, 'w') as f:
                    for block in iter_array(el, (block_size, block_size)):
                        f.write(str(block[i, j]) + "\n")
                        n += 1
            for r in right:
                with open(tmp_r, 'w') as f:
                    for block in iter_array(r, (block_size, block_size)):
                        f.write(str(block[i, j]) + "\n")

            stderr_("training on DCT coefficient ({},{})".format(i, j))
            stderr_("with {} bits per sample".format(bit_allocs[0][i,j]))

            # Zip the two csvs
            csv_zip(tmp_l, tmp_r, tmp_both)

            # # Train on vector covq
            # subprocess.check_call(["../covq/covq", "--train", tmp_both,
            #                        os.path.join(train_dir, str(i)+str(j),
            #                                      cb_2D_both),
            #                        os.path.join(train_dir, str(i)+str(j),
            #                                      cwmap_2D_both),
            #                        "--bep", str(bep), "--dim", str(2*vector_dim),
            #                        "--nsplits", str(bit_allocs[0][i,j]*2),
            #                        "--seed", str(seed), "--lbg-eps",
            #                        str(lbg_eps)])

            # TODO: Here, train on 2-source covq
            # subprocess.check_call(["../covq_2/covq_2", "_tmp_both.csv"])

    # Cleanup tmp files
    [os.remove(p) for p in (tmp_l, tmp_r)]#, tmp_both)]

@command
def png_test(*pngs, test_dir="testing", train_dir="training", block_size=8,
             bep=0.001, vector_dim=1, seed=1234):
    """Accept png filenames in pairs"""
    if len(pngs) % 2 != 0:
        print("odd number of pngs not acceptable")
        raise SystemExit(1)

    # Pair up the images
    imgs = list(zip(pngs[0::2], pngs[1::2]))

    # make covq program
    os.chdir("../")
    subprocess.check_call("make clean", shell=True)
    subprocess.check_call("make covq", shell=True)
    os.chdir("python/")

    dims = []
    for idx, img_pair in enumerate(imgs):
        # Convert images to 1D csvs of DCT coefficients
        if idx == 0:
            (dim_x, dim_y) = ster2csv(img_pair[0], img_pair[1], csv_src_l, csv_src_r)
        else:
            (dim_x, dim_y) = ster2csv(img_pair[0], img_pair[1], csv_src_l, csv_src_r,
                                      'a')
        dims.append((dim_x, dim_y))
    # Convert to (block_size * block_size) sources (one for each type of
    # coefficient)
    left = csv_read(csv_src_l, dims)
    right = csv_read(csv_src_r, dims)

    # mkdir for testing sets
    if not os.path.isdir(test_dir):
        os.mkdir(test_dir)

    with open(os.path.join(train_dir, bit_allocs_pickle), "rb") as f:
        bit_allocs = pickle.load(f)

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

                with open(tmp_l, mode) as f:
                    for block in iter_array(left[idx], (block_size, block_size)):
                        f.write(str(block[i, j]) + "\n")
                        n += 1
                with open(tmp_r, mode) as f:
                    for block in iter_array(right[idx], (block_size, block_size)):
                        f.write(str(block[i, j]) + "\n")

            print("testing on DCT coefficient ({},{})".format(i, j))
            print("with {} bits per sample".format(bit_allocs[0][i,j]))

            # Test on same data sets, output test data
            # independent covq
            subprocess.check_call(["../covq/covq", "--test", tmp_l,
                                   os.path.join(train_dir, str(i)+str(j),
                                                cb_1D_l),
                                   os.path.join(train_dir, str(i)+str(j),
                                                cwmap_1D_l),
                                   os.path.join(test_dir, str(i)+str(j),
                                                t_out_1D_l),
                                   "--bep", str(bep), "--dim", str(vector_dim), "--seed",
                                   str(seed)])

            print("testing on DCT coefficient ({},{})".format(i, j))
            print("with {} bits per sample".format(bit_allocs[1][i,j]))
            subprocess.check_call(["../covq/covq", "--test", tmp_r,
                                   os.path.join(train_dir, str(i)+str(j),
                                                cb_1D_r),
                                   os.path.join(train_dir, str(i)+str(j),
                                                cwmap_1D_r),
                                   os.path.join(test_dir, str(i)+str(j),
                                                t_out_1D_r),
                                   "--bep", str(bep), "--dim", str(vector_dim), "--seed",
                                   str(seed)])

            # # Zip two csvs
            # csv_zip(tmp_l, tmp_r, tmp_both)

            # # Test on vector covq
            # subprocess.check_call(["../covq/covq", "--test", tmp_both,
            #                        os.path.join(train_dir, str(i)+str(j),
            #                                     cb_2D_both),
            #                        os.path.join(train_dir, str(i)+str(j),
            #                                     cwmap_2D_both),
            #                        os.path.join(test_dir, str(i)+str(j),
            #                                     t_out_2D_both), "--bep", str(bep),
            #                        "--dim", str(vector_dim), "--seed", str(seed)])

            # TODO: Here, test on 2-source covq
            # subprocess.check_call(["../covq_2/covq_2", "_tmp_both.csv"])

            # Unzip vector covq results
            # csv_unzip("t_out_both.csv", "t_out_both_l.csv", "t_out_both_r.csv")

            # # TODO: Here, unzip 2-source covq results
            # csv_unzip(os.path.join(test_dir, str(i)+str(j), t_out_2D_both),
            #           os.path.join(test_dir, str(i)+str(j), t_out_2D_l),
            #           os.path.join(test_dir, str(i)+str(j), t_out_2D_r))

    # Join the separate files into single column csv per image
    
    for img_pair in imgs:
        left_1D_dct = []
        right_1D_dct = []
        # left_2D_dct = []
        # right_2D_dct = []
        for i in range(block_size):
            left_1D_dct.append([])
            # left_2D_dct.append([])
            for j in range(block_size):
                left_1D_dct[i].append([])
                # left_2D_dct[i].append([])
                with open(os.path.join(test_dir, str(i)+str(j), t_out_1D_l)) as f:
                    for n_x in range(int(dim_x / block_size)):
                        left_1D_dct[i][j].append([])
                        for n_y in range(int(dim_y / block_size)):
                            left_1D_dct[i][j][n_x].append(float(f.readline()))
                # with open(os.path.join(test_dir, str(i)+str(j), t_out_2D_l)) as f:
                #     for n_x in range(int(dim_x / block_size)):
                #         left_2D_dct[i][j].append([])
                #         for n_y in range(int(dim_y / block_size)):
                #             left_2D_dct[i][j][n_x].append(float(f.readline()))

        for i in range(block_size):
            right_1D_dct.append([])
            # right_2D_dct.append([])
            for j in range(block_size):
                right_1D_dct[i].append([])
                # right_2D_dct[i].append([])
                with open(os.path.join(test_dir, str(i)+str(j), t_out_1D_r)) as f:
                    for n_x in range(int(dim_x / block_size)):
                        right_1D_dct[i][j].append([])
                        for n_y in range(int(dim_y / block_size)):
                            right_1D_dct[i][j][n_x].append(float(f.readline()))
                # with open(os.path.join(test_dir, str(i)+str(j), t_out_2D_r)) as f:
                #     for n_x in range(int(dim_x / block_size)):
                #         right_2D_dct[i][j].append([])
                #         for n_y in range(int(dim_y / block_size)):
                #             right_2D_dct[i][j][n_x].append(float(f.readline()))

        # Write 1D COSQ to file
        left_im = []
        right_im = []
        for n_x in range(int(dim_x / block_size)):
            for i in range(block_size):
                for n_y in range(int(dim_y / block_size)):
                    for j in range(block_size):
                        left_im.append(left_1D_dct[i][j][n_x][n_y])
                        right_im.append(right_1D_dct[i][j][n_x][n_y])

        if idx == 0:
            mode = 'w'
        else:
            mode = 'a'
        with open(os.path.join(test_dir, t_out_1D_l), mode) as f:
            f.writelines([str(n) + "\n" for n in left_im])
        with open(os.path.join(test_dir, t_out_1D_r), mode) as f:
            f.writelines([str(n) + "\n" for n in right_im])

        # # Write 2D COVQ to file
        # left_im = []
        # right_im = []
        # for n_x in range(int(dim_x / block_size)):
        #     for i in range(block_size):
        #         for n_y in range(int(dim_y / block_size)):
        #             for j in range(block_size):
        #                 left_im.append(left_2D_dct[i][j][n_x][n_y])
        #                 right_im.append(right_2D_dct[i][j][n_x][n_y])

        # if idx == 0:
        #     mode = 'w'
        # else:
        #     mode = 'a'
        # with open(os.path.join(test_dir, t_out_2D_l), mode) as f:
        #     f.writelines([str(n) + "\n" for n in left_im])
        # with open(os.path.join(test_dir, t_out_2D_r), mode) as f:
        #     f.writelines([str(n) + "\n" for n in right_im])


    # Convert 1D COSQ csvs to pngs
    print(["{}_t_out.png".format(os.path.splitext(img_pair[0])[0])
              for img_pair in imgs])
    print(list(imgs))

    csv2ster(os.path.join(test_dir, t_out_1D_l),
             os.path.join(test_dir, t_out_1D_r),
             ["{}_t_out_1D.png".format(os.path.splitext(img_pair[0])[0])
              for img_pair in imgs],
             ["{}_t_out_1D.png".format(os.path.splitext(img_pair[1])[0])
              for img_pair in imgs], dims)

    # # Convert 2D COVQ csvs to pngs
    # csv2ster(os.path.join(test_dir, t_out_2D_l),
    #          os.path.join(test_dir, t_out_2D_r),
    #          ["{}_t_out_2D.png".format(os.path.splitext(img_pair[0])[0])
    #           for img_pair in imgs],
    #          ["{}_t_out_2D.png".format(os.path.splitext(img_pair[1])[0])
    #           for img_pair in imgs], dims)
    # csv2ster("t_out_both_l.csv", "t_out_both_r.csv", "vec_l.png", "vec_r.png", dim_x, dim_y)

    # TODO: Here, convert 2-source covq results to png
    # for img_pair in imgs:
    #     left_dct = []
    #     right_dct = []
    #     for i in range(block_size):
    #         left_dct.append([])
    #         for j in range(block_size):
    #             left_dct[i].append([])
    #             with open(os.path.join(test_dir, str(i)+str(j), "t_out_both_l.csv")) as f:
    #                 for n_x in range(int(dim_x / block_size)):
    #                     left_dct[i][j].append([])
    #                     for n_y in range(int(dim_y / block_size)):
    #                         left_dct[i][j][n_x].append(float(f.readline()))

    #     for i in range(block_size):
    #         right_dct.append([])
    #         for j in range(block_size):
    #             right_dct[i].append([])
    #             with open(os.path.join(test_dir, str(i)+str(j), "t_out_both_r.csv")) as f:
    #                 for n_x in range(int(dim_x / block_size)):
    #                     right_dct[i][j].append([])
    #                     for n_y in range(int(dim_y / block_size)):
    #                         right_dct[i][j][n_x].append(float(f.readline()))

    #     left_im = []
    #     right_im = []
    #     for n_x in range(int(dim_x / block_size)):
    #         for i in range(block_size):
    #             for n_y in range(int(dim_y / block_size)):
    #                 for j in range(block_size):
    #                     left_im.append(left_dct[i][j][n_x][n_y])
    #                     right_im.append(right_dct[i][j][n_x][n_y])

    #     if idx == 0:
    #         mode = 'w'
    #     else:
    #         mode = 'a'
    #     with open(os.path.join(test_dir, "t_out_both_r.csv"), mode) as f:
    #         f.writelines([str(n) + "\n" for n in right_im])
    #     with open(os.path.join(test_dir, "t_out_both_l.csv"), mode) as f:
    #         f.writelines([str(n) + "\n" for n in left_im])

    # # Convert csvs to pngs

    # csv2ster(os.path.join(test_dir, "t_out_both_l.csv"),
    #          os.path.join(test_dir, "t_out_both_r.csv"),
    #          ["{}_t_out_both.png".format(os.path.splitext(img_pair[0])[0])
    #           for img_pair in imgs],
    #          ["{}_t_out_both.png".format(os.path.splitext(img_pair[1])[0])
    #           for img_pair in imgs],
    #          dims)

@command
def iter_gaussian_epsilon(psnr_out_1D, psnr_out_2D, snr_out_1D,
                          snr_out_2D):
    """Iterate through channel epsilons, on Gaussian data"""
    import numpy as np
    rho = 0.5
    bit_rate = 3
    psnrs = []
    snrs = []


@command
def iter_gaussian_bitrate(rho, psnr_out_1D, psnr_out_2D, snr_out_1D,
                          snr_out_2D):
    """Iterate through bit rates, on Gaussian data"""
    import numpy as np
    rho = float(rho)
    psnrs = []
    snrs = []
    for bit_rate in range(1, 5):
        result_dict = gaussian_train_test(bit_rate, rho)

        psnr_1d = result_dict['psnr']['1d']
        mean_psnr_1d = sum(psnr_1d)/len(psnr_1d)
        psnr_2d = result_dict['psnr']['2d']
        mean_psnr_2d = sum(psnr_2d)/len(psnr_2d)
        psnrs.append((bit_rate, mean_psnr_1d, mean_psnr_2d))

        snr_1d = result_dict['snr']['1d']
        mean_snr_1d = sum(snr_1d)/len(snr_1d)
        snr_2d = result_dict['snr']['2d']
        mean_snr_2d = sum(snr_2d)/len(snr_2d)
        snrs.append((bit_rate, mean_snr_1d, mean_snr_2d))

    with open(psnr_out_1D, "w") as f:
        for entry in psnrs:
            f.write("{},{}\n".format(entry[0], entry[1]))
    with open(psnr_out_2D, "w") as f:
        for entry in psnrs:
            f.write("{},{}\n".format(entry[0], entry[2]))

    with open(snr_out_1D, "w") as f:
        for entry in snrs:
            f.write("{},{}\n".format(entry[0], entry[1]))
    with open(snr_out_2D, "w") as f:
        for entry in snrs:
            f.write("{},{}\n".format(entry[0], entry[2]))


@command
def iter_gaussian_correlation(bit_rate, psnr_out_1D, psnr_out_2D,
                              snr_out_1D, snr_out_2D):
    """Iterate through correlation constants, on Gaussian data"""

    import numpy as np
    bit_rate = int(bit_rate)
    psnrs = []
    snrs = []
    for rho in np.linspace(0, 1, 40):
        result_dict = gaussian_train_test(bit_rate, rho)

        psnr_1d = result_dict['psnr']['1d']
        mean_psnr_1d = sum(psnr_1d)/len(psnr_1d)
        psnr_2d = result_dict['psnr']['2d']
        mean_psnr_2d = sum(psnr_2d)/len(psnr_2d)
        psnrs.append((rho, mean_psnr_1d, mean_psnr_2d))

        snr_1d = result_dict['snr']['1d']
        mean_snr_1d = sum(snr_1d)/len(snr_1d)
        snr_2d = result_dict['snr']['2d']
        mean_snr_2d = sum(snr_2d)/len(snr_2d)
        snrs.append((rho, mean_snr_1d, mean_snr_2d))

    with open(psnr_out_1D, "w") as f:
        for entry in psnrs:
            f.write("{},{}\n".format(entry[0], entry[1]))
    with open(psnr_out_2D, "w") as f:
        for entry in psnrs:
            f.write("{},{}\n".format(entry[0], entry[2]))

    with open(snr_out_1D, "w") as f:
        for entry in snrs:
            f.write("{},{}\n".format(entry[0], entry[1]))
    with open(snr_out_2D, "w") as f:
        for entry in snrs:
            f.write("{},{}\n".format(entry[0], entry[2]))


def gaussian_data(rho=0.5, sigma_1=1, sigma_2=1, mean_1=0, mean_2=0,
                  seed=1234):
    """Return bivariate normal data with correlation coefficient
    :param:`rho`, std deviations :param:`sigma_1`, :param:`sigma_2`,
    means :param:`mean_1`, :param:`mean_2`
    """

    from numpy.random import multivariate_normal
    from numpy.random import seed as np_seed

    np_seed(seed)

    cov = np.array([[float(sigma_1)**2,
                     float(rho)*float(sigma_1)*float(sigma_2)],
                    [float(rho)*float(sigma_1)*float(sigma_2),
                     float(sigma_2)**2]], dtype=float)

    means = np.array([float(mean_1), float(mean_2)], dtype=float)

    return multivariate_normal(means, cov, int(tr_size))


# def train_2d

# def train_1d

# def test_2d

# def test_1d

def gaussian_train_test(bit_rate, rho, tr_size=10000, test_size=10000,
                        error_rate=0.001, seed=1234, sigma_1=1, sigma_2=1,
                        mean_1=0, mean_2=0):
    """Generate Gaussian training set of size :param:`tr_size`, and
    Gaussian testing set of size :param:`test_size`, correlation
    :param:`rho`. Output two columns of PSNR, SNR, SQNR in a dictionary

    {'psnr': {'2d': [...], '1d': [...]},
     'snr': {'2d': [...], '1d': [...]},
     'sqnr': {'2d': [...], '1d': [...]}}
    """

    import subprocess

    # train = 
    # test = 

    with open("_tmp.csv", "w") as f:
        for row in iter_array(train, (1, 2)):
            f.write("{},{}\n".format(row[0,0], row[0,1]))
    with open("_tmp_test.csv", "w") as f:
        for row in iter_array(test, (1, 2)):
            f.write("{},{}\n".format(row[0,0], row[0,1]))

    csv_unzip("_tmp.csv", "_tmp_l.csv", "_tmp_r.csv")
    csv_unzip("_tmp_test.csv", "_tmp_test_l.csv", "_tmp_test_r.csv")
    subprocess.check_call(["../covq/covq", "--train", "_tmp_l.csv",
                           "_cb_out.csv", "_cwmap_out.csv", "--test",
                           "_tmp_test_l.csv", "_cb_out.csv", "_cwmap_out.csv",
                           "_test_out_l.csv", "--bep", str(error_rate),
                           "--nsplits", str(bit_rate), "--seed", str(seed),
                           "--dim", "1"])
    subprocess.check_call(["../covq/covq", "--train", "_tmp_r.csv",
                           "_cb_out.csv", "_cwmap_out.csv", "--test",
                           "_tmp_test_r.csv", "_cb_out.csv", "_cwmap_out.csv",
                           "_test_out_r.csv", "--bep", str(error_rate),
                           "--nsplits", str(bit_rate), "--seed", str(seed),
                           "--dim", "1"])

    csv_zip("_test_out_l.csv", "_test_out_r.csv", "_test_out.csv")
    new_signal = []
    with open("_test_out.csv") as f:
        new_signal.append([float(n) for n in f.readline().strip().split(",")])

    psnr_1D = [psnr(train[:,0], [s[0] for s in new_signal]),
               psnr(train[:,1], [s[1] for s in new_signal])]
    snr_1D = [snr(train[:,0], [s[0] for s in new_signal]),
              snr(train[:,1], [s[1] for s in new_signal])]

    subprocess.check_call(["../covq/covq", "--train", "_tmp.csv",
                           "_cb_out.csv", "_cwmap_out.csv", "--test",
                           "_tmp_test.csv", "_cb_out.csv", "_cwmap_out.csv",
                           "_test_out.csv", "--bep", str(error_rate),
                           "--nsplits", str(int(bit_rate)*2), "--seed",
                           str(seed), "--dim", "2"])
    
    new_signal = []
    with open("_test_out.csv") as f:
        new_signal.append([float(n) for n in f.readline().strip().split(",")])
    
    psnr_2D = [psnr(train[:,0], [s[0] for s in new_signal]),
               psnr(train[:,1], [s[1] for s in new_signal])]
    snr_2D = [snr(train[:,0], [s[0] for s in new_signal]),
              snr(train[:,1], [s[1] for s in new_signal])]

    return {'psnr': {'2d': psnr_2D, '1d': psnr_1D},
            'snr': {'2d': snr_2D, '1d': snr_1D}}


def snr(orig_signal, new_signal):
    """Return SNR in dB"""

    from math import log
    mse = 0
    energy = 0
    mean = 0

    for orig_val, new_val in zip(orig_signal, new_signal):
        mse += (orig_val - new_val) ** 2
        energy += orig_val ** 2
        mean += orig_val

    mean /= len(orig_signal)
    mean_sq = mean**2
    mse /= len(orig_signal)
    energy /= len(orig_signal)

    return 10*log((energy - mean_sq) / mse, 10)


def psnr(orig_signal, new_signal):
    """Return PSNR in dB"""

    from math import log
    mse = 0
    for orig_val, new_val in zip(orig_signal, new_signal):
        mse += (orig_val - new_val)**2
    mse /= len(orig_signal)
    return 10*log((max(orig_signal) - min(orig_signal))**2 / mse, 10)


@command
def get_psnr(orig_left, orig_right, new_left, new_right):
    """Return PSNR of pixel values in original pngs to reconstructed pngs"""

    from PIL import Image
    from math import log
    im_orig = [Image.open(orig_left), Image.open(orig_right)]
    im_new = [Image.open(new_left), Image.open(new_right)]

    mse_l = 0
    mse_r = 0

    for orig_pix, new_pix in zip(im_orig[0].getdata(), im_new[0].getdata()):
        mse_l += (orig_pix - new_pix)**2
    for orig_pix, new_pix in zip(im_orig[1].getdata(), im_new[1].getdata()):
        mse_r += (orig_pix - new_pix)**2

    mse_l /= (im_orig[0].size[0] * im_orig[0].size[1])
    mse_r /= (im_orig[1].size[0] * im_orig[1].size[1])

    return (10*log(256**2 / mse_l, 10), 10*log(256**2 / mse_r, 10))

def psnr_vs_compression(*args):
    """Accept png filenames in pairs"""

    test(args)
    for i, filename in enumerate(args):
        if i % 2 == 0:
            filename_prev = filename
        else:
            print(get_psnr(filename_prev, filename,
                           os.path.splitext(filename_prev)[0]+"_t_out.png",
                           os.path.splitext(filename)[0]+"_t_out.png"))


if __name__ == '__main__':
    parse_args()