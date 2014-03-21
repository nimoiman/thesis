#!/usr/bin/env python3
"""A collection of utilities for csv, png I/O for use with COVQ training
and testing
"""

# Python 2 compat
from __future__ import print_function

from api_docs import command, parse_args
from util import *


def _2d_dct(a):
    """Perform 2d DCT-II on a in row, column order"""

    from scipy.fftpack import dct
    return dct(dct(a, norm='ortho').transpose(), norm='ortho').transpose()


def _2d_idct(a):
    """Perform 2d DCT-III on a in column, row order"""

    from scipy.fftpack import idct
    return idct(idct(a.transpose(), norm='ortho').transpose(), norm='ortho')


def _count_lines(f):
    """Return number of newlines in f"""
    n_lines = 0
    for line in open(f):
        n_lines += 1
    return n_lines


def ster2arr(filename_1, filename_2, block_size=8):
    """Read pngs filename_1, filename_2, return tuple of np.arrays
    cropped to correct size"""

    from PIL import Image

    ster_images = [Image.open(filename_1), Image.open(filename_2)]

    # Crop to size of smallest image
    new_dim_x = min([im.size[0] for im in ster_images])
    new_dim_y = min([im.size[1] for im in ster_images])

    # Trim images to length & width multiples of block_size
    new_dim_x -= new_dim_x % block_size
    new_dim_y -= new_dim_y % block_size

    return [ster_images[0].crop((0, 0, new_dim_x, new_dim_y)),
            ster_images[1].crop((0, 0, new_dim_x, new_dim_y))], \
           (new_dim_x, new_dim_y)


def _img_dct(img_1, img_2, block_size=8):
    """Accept 2 grayscale 8bit PIL.Images, convert to np.array of
    floats, subtract 128 from each pixel value, perform 2D DCT-II,
    return resulting two np.arrays
    """
    import numpy as np

    # transform to np.array of floats
    ster_images = [np.array(img_1, dtype=float), np.array(img_2, dtype=float)]

    # put pixel blocks in range [-128, 127]
    ster_images = [im - 128 for im in ster_images]

    # Take DCT block by block
    for img in ster_images:
        for block in iter_array(img, (block_size, block_size)):
            block_dct = _2d_dct(block)

            # copy block_dct into block
            for i in range(block_size):
                for j in range(block_size):
                    block[i,j] = block_dct[i,j]

    return ster_images


def _var_block(imgs, block_size):
    """Return matrix of Variances of blocks of block_size in training
    set of images :param:`imgs`
    """

    import numpy as np
    mean_block = np.zeros((block_size, block_size), dtype=float)
    var_block = np.zeros((block_size, block_size), dtype=float)
    
    n_entries = 0
    for im in imgs:
        for block in iter_array(im, (block_size, block_size)):
            mean_block += block
            n_entries += 1

    # Element-wise divide by n
    mean_block = np.divide(mean_block, n_entries)
    # Square each entry
    mean_sq_block = np.multiply(mean_block, mean_block)

    n_entries = 0
    for block in iter_array(im, (block_size, block_size)):
        # Calculate Variances
        var_block += np.multiply(block, block) - mean_sq_block
        n_entries += 1

    # Element-wise divide by n
    var_block = np.divide(var_block, n_entries)

    return var_block


def dist_rate_gauss(var, n_bits):
    """Return distortion-rate function for Gaussian source with
    rate :param:`n_bits`, variance :param:`var`"""

    return var / (2 ** (2*n_bits))


def dist_rate_laplace(var, delta):
    """Return distortion-rate approximation for Laplacian source with
    rate :param:`n_bits`, variance :param:`var`,
    quantization bin size :param:`delta
    """

    from numpy import tanh, exp
    from math import sqrt
    lambda_ = sqrt(2/var)
    return (2/lambda_**2) - (delta/lambda_) * \
        (1 + (1/tanh((lambda_*delta)/2)))*exp((-lambda_*delta)/2)


def bit_distortion(var_block, bit_alloc, ranges):
    """Return minimum distortion of DCT coefficients that are
    distributed according to variances in :param:`var_block`, and the
    DC coefficient ~ Gaussian and the the AC coefficients ~ Laplacian
    where :param:`_range` is the quantization bin range of AC coefficients
    """

    sum_ = 0

    for i in range(var_block.shape[0]):
        for j in range(var_block.shape[1]):
            if i == 0 and j == 0:
                # DC coefficient
                sum_ += dist_rate_gauss(var_block[0, 0], bit_alloc[0, 0])
            else:
                # AC coefficients
                delta = ranges[i, j] / (2**bit_alloc[i, j])
                sum_ += dist_rate_laplace(var_block[i, j], delta)

    return sum_ / (var_block.shape[0] * var_block.shape[1])


def _ranges(imgs, block_size):
    """Find max/mins for each DCT coefficient in imgs"""

    import numpy as np
    ranges = np.zeros((block_size, block_size), dtype=float)
    
    for i in range(block_size):
        for j in range(block_size):
            dct_coeff = [block[i, j] for im in imgs
                         for block in iter_array(im, (block_size, block_size))]
            ranges[i, j] = max(dct_coeff) - min(dct_coeff)

    return ranges


def bit_allocate(csv_filename_1, csv_filename_2, block_bit_rate, dims,
                 block_size=8):
    """Use greedy steepest descent algorithm to find optimum bit
    allocation for DCT coefficients in :param:`img` assuming the DC
    coefficient ~ Gaussian and the the AC coefficients ~ Laplacian
    """

    import numpy as np

    n_lines_1 = _count_lines(csv_filename_1)
    n_lines_2 = _count_lines(csv_filename_2)

    if n_lines_1 != n_lines_2:
        stderr_("size of input csvs don't match")
        exit_(1)

    if n_lines_1 % (block_size * block_size) != 0:
        stderr_("block_size does not divide the number of lines in " +
              "the csv file.")
        exit_(1)

    ster_images = []

    # Read in image data from csv
    ster_images.append(csv_read(csv_filename_1, dims,
                                block_order=False, block_size=block_size))
    ster_images.append(csv_read(csv_filename_2, dims,
                                block_order=False, block_size=block_size))

    var_blocks = [_var_block(ster_images[0], block_size),
                  _var_block(ster_images[1], block_size)]

    bit_allocs = [np.zeros((block_size, block_size), dtype=int),
                  np.zeros((block_size, block_size), dtype=int)]

    ranges = [_ranges(ster_images[0], block_size),
              _ranges(ster_images[1], block_size)]

    # Find optimal bit allocation for left, then right images
    for idx, imgs in enumerate(ster_images):
        for bit in range(block_bit_rate):
            best_dist = np.inf
            best_step = None
            for i in range(block_size):
                for j in range(block_size):
                    bit_allocs[idx][i, j] += 1
                    new_dist = bit_distortion(var_blocks[idx], bit_allocs[idx],
                                              ranges[idx])
                    if new_dist < best_dist:
                        best_step = (i, j)
                        best_dist = new_dist
                    bit_allocs[idx][i, j] -= 1
            bit_allocs[idx][best_step] += 1

    return (bit_allocs, var_blocks)


@command
def ster2csv(filename_1, filename_2, out_filename_1, out_filename_2,
             mode='w', block_order=False, block_size=8):
    """Extract pixel data from stereo grayscale images filename_1,
    filename_2, perform normalized DCT-II on block_size x block_size
    blocks and output to 2 csv files of DCT coefficients

    .. note::
        If size of images is not multiple of block_size, will crop from
        the right and bottom such that the two images have dimensions
        which are multiples of block_size, and are the same size.

    If block_order=True, serialize data per block first.
    """

    import numpy as np

    ster_images, dim = ster2arr(filename_1, filename_2, block_size=block_size)

    ster_images = _img_dct(ster_images[0], ster_images[1],
                           block_size=block_size)

    csv_write(out_filename_1, ster_images[0], mode=mode,
              block_order=block_order, block_size=block_size)
    csv_write(out_filename_2, ster_images[1], mode=mode,
              block_order=block_order, block_size=block_size)

    return dim


def csv2ster(csv_filename_1, csv_filename_2, out_filenames_1, out_filenames_2,
             dims, block_order=False, block_size=8):
    """Extract DCT coefficiencts from 2-column csv_filename, perform 
    inverse 2D DCT on block_size x block_size blocks and output to two
    .png's"""

    from PIL import Image
    import numpy as np

    # Find how many lines in csv_filename_{1,2}
    lines_1 = _count_lines(csv_filename_1)
    lines_2 = _count_lines(csv_filename_2)

    if lines_1 != lines_2:
        stderr_("The provided csv files do not have the same number of " +
                "lines.")
        exit_(1)

    if lines_1 != sum([dim[0] * dim[1] for dim in dims]):
        stderr_("The provided dimensions do not match the number of lines " +
                "in the input files")
        stderr_("file1: {}".format(csv_filename_1))
        stderr_("file2: {}".format(csv_filename_2))
        stderr_("# of lines: {}".format(lines_1))
        stderr_("dimensions: {}".format(dims))
        exit_(1)
    else:
        for dim in dims:
            if dim[0] % block_size != 0 or dim[1] % block_size != 0:
                stderr_("The provided dimensions must be multiples of block " +
                        "size {}".format(block_size))
                exit_(1)

    ster_images = []
    ster_images.append(csv_read(csv_filename_1, dims, block_order, block_size))
    ster_images.append(csv_read(csv_filename_2, dims, block_order, block_size))
    ster_images = zip(ster_images[0], ster_images[1])

    for idx, dim in enumerate(dims):
        # Perform inverse DCT on blocks
        for img_pair in ster_images:
            for img in img_pair:
                for blk_idx, block in enumerate(iter_array(img, (block_size, block_size))):
                    if blk_idx in [193, 194, 195, 196, 197, 198]:
                        print(blk_idx)
                        print(block)
                        print(_2d_idct(block) + 128)
                    block_dct = _2d_idct(block)
                    for i in range(block_size):
                        for j in range(block_size):
                            block[i,j] = block_dct[i,j]

            # Convert to rounded signed integers
            out_im = [np.around(im).astype(int) for im in img_pair]

        # Ensure no overflow errors
        for im in out_im:
            for i in range(dim[0]):
                for j in range(dim[1]):
                    if int(im[i,j]) < -128:
                        im[i,j] = -128
                    elif int(im[i,j]) > 127:
                        im[i,j] = 127


        # Add 128, convert to 8bit ints
        out_im = [(a + 128).astype(np.uint8) for a in out_im]
        print(out_im)
        print("len={}".format(len(out_im)))
        # convert to uint8 & save (as png)
        Image.fromarray(out_im[0]).save(out_filenames_1[idx])
        Image.fromarray(out_im[1]).save(out_filenames_2[idx])


if __name__ == '__main__':
    parse_args()
