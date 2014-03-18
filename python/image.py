#!/usr/bin/env python
from api_docs import command, parse_args, help

def _2d_dct(a):
    """Perform 2d DCT-II on a in row, column order"""
    from scipy.fftpack import dct
    return dct(dct(a, norm='ortho').transpose(), norm='ortho').transpose()


def _2d_idct(a):
    """Perform 2d DCT-III on a in column, row order"""
    from scipy.fftpack import idct
    return idct(idct(a.transpose(), norm='ortho').transpose(), norm='ortho')

def csv_read(filename, dims, block_order=False, block_size=None):
    """Read a single column newline-separated file of floats, return
    an numpy array of dim_x by dim_y
    .. note::
        The single column newline-separated file is the standard that
        we use because we wrote C code that likes it that way.
    """
    # Check that num lines == dim_x * dim_y
    # Find how many lines in csv_filename
    import numpy as np
    lines = 0
    for line in open(filename, 'r'):
        lines += 1

    print("dims={}".format(dims))
    if lines != sum([dim[0] * dim[1] for dim in dims]):
        print("The provided dimensions do not match the number of lines " +
              "in the input file")
        raise SystemExit(1)

    arrs = []
    for dim in dims:
        if block_order:
            arr = np.empty(dim, dtype=float)
            if block_size is None:
                raise TypeError("No block size provided with option " +
                                "block_order=True")
            else:
                block_size = int(block_size)
                block_shape = (block_size, block_size)
            # Read as block by block serial data
            with open(filename, 'r') as f:
                for block in iter_array(arr, block_shape):
                    for i in range(block_size):
                        for j in range(block_size):
                            arr[i,j] = float(f.readline())
        else:
            arr = np.empty(dim[0] * dim[1], dtype=float)
            with open(filename, 'r') as f:
                for i in range(dim[0] * dim[1]):
                    arr[i] = float(f.readline())

            arr = arr.reshape((dim[0], dim[1]))
            arrs.append(arr)

    return arrs


def iter_array(arr, vec_shape=(8,1)):
    """Iterate through numpy array arr in left-to-right, then
    top-to-bottom order, outputting windowed views of arr of shape
    vec_shape

    .. note::
        Any change you make to the outputted views changes the original
        array arr
    """
    # Make sure vec_shape matches arr shape
    if arr.shape[0] % vec_shape[0] != 0 or arr.shape[1] % vec_shape[1] != 0:
        raise ValueError('dimension mismatch')

    for i in range(0, arr.shape[0], vec_shape[0]):
        for j in range(0, arr.shape[1], vec_shape[1]):
            yield arr[i:i+vec_shape[0], j:j+vec_shape[1]]


@command
def csv_zip(filename_1, filename_2, out_file):
    """Zips two files together line-by-line"""
    # Check that the number of lines is the same
    lines_1 = 0
    for line in open(filename_1, 'r'):
        lines_1 += 1
    lines_2 = 0
    for line in open(filename_2, 'r'):
        lines_2 += 1
    if lines_1 != lines_2:
        print("Files provided do not have the same number of lines")
        raise SystemExit(1)

    with open(filename_1, 'r') as f_1:
        with open(filename_2, 'r') as f_2:
            with open(out_file, 'w') as f:
                for line in f_1:
                    f.write(line[:-1] + ',' + f_2.readline())


@command
def csv_unzip(filename, out_file_1, out_file_2, slice_index=1):
    """Unzips file into two files containing slice_index, and
    (original dim - slice_index) columns in each respectively"""
    with open(filename, 'r') as f:
        with open(out_file_1, 'w') as f_1:
            with open(out_file_2, 'w') as f_2:
                for line in f:
                    entries = line.split(',')
                    for i, entry in enumerate(line[:-1].split(',')):
                        if i < slice_index:
                            f_1.write(entry)
                            if i % slice_index < slice_index - 1:
                                f_1.write(',')
                            else:
                                f_1.write('\n')
                        else:
                            f_2.write(entry)
                            if i % slice_index < slice_index - 1:
                                f_2.write(',')
                            else:
                                f_2.write('\n')


@command
def csv_set_dim(filename, out_file, dim):
    """Reshape csv file to have dim entries per line"""
    # Find number of lines
    # & find number of entries per line
    entries = []
    dim = int(dim)
    for n, line in enumerate(open(filename, 'r')):
        line_entries = line[:-1].split(',')
        if n == 0:
            in_dim = len(line_entries)
        elif len(line_entries) != in_dim:
            print("csv input file is jagged, exiting")
            raise SystemExit(1)
        entries += line_entries
    # Check that dim divides num lines * original dim
    if len(entries) % dim != 0:
        print("Provided dimension does not divide the data")
        raise SystemExit(1)
    with open(out_file, 'w') as f:
        import code
        code.interact(local=locals())
        for i, entry in enumerate(entries):
            f.write(entry)
            if i % dim < dim - 1:
                f.write(',')
            else:
                f.write('\n')


@command
def serialize(*filenames):
    if len(filenames) % 2 == 1:
        print("Odd number of filenames provided, indicate pairs of " +
              "input/output files.")
        raise SystemExit(1)


    for (in_file, out_file) in zip(*[iter(filenames)] * 2):
        csv_set_dim(in_file, out_file, 1)

def lala(arg_1, arg_2, append=False, lo=False):
    print(append)

@command
def ster2csv(filename_1, filename_2, out_filename_1, out_filename_2,
             append=False, block_order=False, block_size=8):
    """Extract pixel data from stereo grayscale images filename_1,
    filename_2, perform normalized DCT-II on block_size x block_size
    blocks and output to 2 csv files of DCT coefficients

    .. note::
        If size of images is not multiple of block_size, will crop from
        the right and bottom such that the two images have dimensions
        which are multiples of block_size, and are the same size.

    If block_order=True, serialize data per block first.
    """
    from PIL import Image
    import numpy as np
    print("append={}".format(append))

    ster_images = [Image.open(filename_1), Image.open(filename_2)]

    # Crop images to size multiple of block_size
    dim_x = min([im.size[0] for im in ster_images])
    dim_y = min([im.size[1] for im in ster_images])
    if dim_x % block_size != 0 or dim_y % block_size != 0:
        dim_x -= dim_x % block_size
        dim_y -= dim_y % block_size
        print("cropping images to width of {}x{}px".format(dim_x, dim_y))
        ster_images = [im.crop((0, 0, dim_x, dim_y)) for im in ster_images]

    # transform to np.array
    ster_images = [np.array(im, dtype=float) for im in ster_images]

    # put pixel blocks in range [-128, 127]
    ster_images = [im - 128 for im in ster_images]

    # Take DCT block by block
    for img in ster_images:
        for block in iter_array(img, (block_size, block_size)):
            block_dct = _2d_dct(block)
            for i in range(block_size):
                for j in range(block_size):
                    block[i,j] = block_dct[i,j]

    # Serialize the two matrices in a left-to-right, top-to-bottom fashion
    # & write to files
    if append:
        mode = 'a'
    else:
        mode = 'w'
    print("mode={}".format(mode))
    print("ster_images[0].flatten().shape={}".format(ster_images[0].flatten().shape))
    with open(out_filename_1, mode) as f:
        if block_order:
            for block in iter_array(ster_images[0], (block_size, block_size)):
                for i in range(block_size):
                    for j in range(block_size):
                        f.write(str(block[i,j]) + '\n')
        else:
            for n in ster_images[0].flatten():
                f.write(str(n) + '\n')

    with open(out_filename_2, mode) as f:
        if block_order:
            for block in iter_array(ster_images[1], (block_size, block_size)):
                for i in range(block_size):
                    for j in range(block_size):
                        f.write(str(block[i,j]) + '\n')
        else:
            for n in ster_images[1].flatten():
                f.write(str(n) + '\n')


    print("dim=({},{})".format(dim_x, dim_y))
    return (dim_x, dim_y)


def _var_block(imgs, block_size):
    """Use greedy steepest descent algorithm to find optimum bit
    allocation for DCT coefficients in ``imgs``
    """
    import numpy as np
    mean_block = np.zeros((block_size, block_size), dtype=float)
    var_block = np.zeros((block_size, block_size), dtype=float)
    n = 0
    for im in imgs:
        for block in iter_array(im, (block_size, block_size)):
            mean_block += block
            n += 1

    mean_block = np.divide(mean_block, n)
    mean_sq_block = np.multiply(mean_block, mean_block)

    for im in imgs:
        for block in iter_array(im, (block_size, block_size)):
            var_block += np.multiply(block, block) - mean_sq_block

    var_block = np.divide(var_block, n)

    return var_block

def dist_rate_gauss(var, n_bits):
    return var / (2**(2*n_bits))

def dist_rate_laplace(var, n_bits):
    """Assume quantization to within 3 std deviations"""
    from numpy import tanh, exp
    from math import sqrt
    lambda_ = sqrt(2/var)
    delta = (3 * sqrt(var)) / (2**n_bits)
    return (2/lambda_**2) - (delta/lambda_) * \
        (1 + (1/tanh((lambda_*delta)/2)))*exp((-lambda_*delta)/2)


def bit_distortion(var_block, bit_alloc):
    sum_ = 0
    
    for i in range(var_block.shape[0]):
        for j in range(var_block.shape[1]):
            if i == 0 and j == 0:
                sum_ += dist_rate_gauss(var_block[0, 0], bit_alloc[0, 0])
            else:
                sum_ += dist_rate_laplace(var_block[i, j], bit_alloc[i, j])
    return sum_ / (var_block.shape[0] * var_block.shape[1])

def bit_allocate(csv_filename_1, csv_filename_2, rate, dims,
                 block_size=8):
    """Return bit_alloc matrices along with variance matrix"""
    import numpy as np
    lines_1 = 0
    lines_2 = 0
    for line in open(csv_filename_1):
        lines_1 += 1
    for line in open(csv_filename_2):
        lines_2 += 1
    if lines_1 != lines_2:
        print("size of input csvs don't match")
        raise SystemExit(1)
    if lines_1 % (block_size * block_size) != 0:
        print("block_size does not divide the number of lines in " +
              "the csv file.")
        raise SystemExit(1)

    
    ster_images = []

    # Read in image data from csv
    ster_images.append(csv_read(csv_filename_1, dims,
                                block_order=False, block_size=block_size))
    ster_images.append(csv_read(csv_filename_2, dims,
                                block_order=False, block_size=block_size))
    for dim in dims:

        var_blocks = [_var_block(ster_images[0], block_size),
                      _var_block(ster_images[1], block_size)]

        bit_allocs = [np.zeros((block_size, block_size), dtype=int),
                      np.zeros((block_size, block_size), dtype=int)]

        for idx, var_block in enumerate(var_blocks):
            for bit in range(int(rate * block_size * block_size)):
                best_dist = np.inf
                best_step = None
                for i in range(block_size):
                    for j in range(block_size):
                        bit_allocs[idx][i, j] += 1
                        new_dist = bit_distortion(var_block, bit_allocs[idx])
                        if new_dist < best_dist:
                            # print("new_dist={} < {}=old_dist".format(new_dist,best_dist))
                            best_step = (i, j)
                            best_dist = new_dist
                        bit_allocs[idx][i, j] -= 1
                bit_allocs[idx][best_step] += 1
                print(bit_allocs[idx])
    return (bit_allocs, var_blocks)

def csv2ster(csv_filename_1, csv_filename_2, out_filenames_1, out_filenames_2,
             dims, block_order=False, block_size='8'):
    """Extract DCT coefficiencts from 2-column csv_filename, perform 
    inverse DCT on block_size x block_size blocks and output to two
    .png's"""
    from PIL import Image
    import numpy as np

    block_order = bool(block_order)
    block_size = int(block_size)

    # Find how many lines in csv_filename_{1,2}
    lines_1 = 0
    for line in open(csv_filename_1, 'r'):
        lines_1 += 1
    lines_2 = 0
    for line in open(csv_filename_2, 'r'):
        lines_2 += 1

    if lines_1 != lines_2:
        print("The provided csv files do not have the same number of " +
              "lines.")
        raise SystemExit(1)

    print("dims={}".format(dims))

    if lines_1 != sum([dim[0] * dim[1] for dim in dims]):
        print("The provided dimensions do not match the number of lines " +
              "in the input files")
        raise SystemExit(1)
    else:
        for dim in dims:
            if dim[0] % block_size != 0 or dim[1] % block_size != 0:
                print("The provided dimensions must be multiples of block " +
                      "size {}".format(block_size))
                raise SystemExit(1)

    ster_images = [None, None]
    ster_images[0] = csv_read(csv_filename_1, dims, block_order, block_size)
    ster_images[1] = csv_read(csv_filename_2, dims, block_order, block_size)
    ster_images = zip(ster_images[0], ster_images[1])

    for idx, dim in enumerate(dims):
        # Perform inverse DCT on blocks
        for img_pair in ster_images:
            for img in img_pair:
                for block in iter_array(img, (block_size, block_size)):
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

        # convert to uint8 & save (as png)
        Image.fromarray(out_im[0]).save(out_filenames_1[idx])
        Image.fromarray(out_im[1]).save(out_filenames_2[idx])


if __name__ == '__main__':
    parse_args()