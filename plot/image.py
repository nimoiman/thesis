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


def _csv_read(filename, dim_x, dim_y):
    """Read a single column newline-separated file of floats, return
    an numpy array of dim_x by dim_y
    .. note::
        The single column newline-separated file is the standard that
        we use because we wrote C code that likes it that way. Ideally,
        the input/output format would be a .raw file"""
    # Check that num lines == dim_x * dim_y
    # Find how many lines in csv_filename
    import numpy as np
    lines = 0
    for line in open(filename, 'r'):
        lines += 1

    if lines != dim_x * dim_y:
        print("The provided dimensions do not match the number of lines " +
              "in the input file")
        raise SystemExit(1)

    arr = np.empty(dim_x * dim_y, dtype=float)

    with open(filename, 'r') as f:
        for i in range(dim_x * dim_y):
            arr[i] = float(f.readline())

    arr = arr.reshape((dim_x, dim_y))
    return arr


def _iter_array(arr, vec_shape=(8,1)):
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
def ster2csv(filename_1, filename_2, out_filename_1, out_filename_2,
             block_size=8):
    """Extract pixel data from stereo grayscale images filename_1,
    filename_2, perform normalized DCT-II on block_size x block_size
    blocks and output to 2 csv files of DCT coefficients

    .. note::
        If size of images is not multiple of block_size, will crop from
        the right and bottom such that the two images have dimensions
        which are multiples of block_size, and are the same size.
    """
    from PIL import Image
    import numpy as np

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
        for block in _iter_array(img, (block_size, block_size)):
            block_dct = _2d_dct(block)
            for i in range(block_size):
                for j in range(block_size):
                    block[i,j] = block_dct[i,j]

    # Serialize the two matrices in a left-to-right, top-to-bottom fashion
    # & write to files
    with open(out_filename_1, 'w') as f:
        for n in ster_images[0].flatten():
            f.write(str(n) + '\n')
    with open(out_filename_2, 'w') as f:
        for n in ster_images[1].flatten():
            f.write(str(n) + '\n')

    return (dim_x, dim_y)


@command
def csv2ster(csv_filename_1, csv_filename_2, out_filename_1, out_filename_2,
             dim_x, dim_y, block_size=8):
    """Extract DCT coefficiencts from 2-column csv_filename, perform 
    inverse DCT on block_size x block_size blocks and output to two
    .png's"""
    from PIL import Image
    import numpy as np

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

    dim_x = int(dim_x)
    dim_y = int(dim_y)

    if lines_1 != dim_x * dim_y:
        print("The provided dimensions do not match the number of lines " +
              "in the input files")
        raise SystemExit(1)
    elif dim_x % block_size != 0 or dim_y % block_size != 0:
        print("The provided dimensions must be multiples of block " +
              "size {}".format(block_size))
        raise SystemExit(1)

    ster_images = [np.empty((dim_x, dim_y), dtype=float),
                   np.empty((dim_x, dim_y), dtype=float)]

    # Read in image data from csv
    ster_images[0] = _csv_read(csv_filename_2, dim_x, dim_y)
    ster_images[1] = _csv_read(csv_filename_1, dim_x, dim_y)

    # Perform inverse DCT on blocks
    for img in ster_images:
        for block in _iter_array(img, (block_size, block_size)):
            block_dct = _2d_idct(block)
            for i in range(block_size):
                for j in range(block_size):
                    block[i,j] = block_dct[i,j]

    # Convert to rounded signed integers
    ster_images = [np.around(im).astype(int) for im in ster_images]

    # Ensure no overflow errors
    for im in ster_images:
        for i in range(dim_x):
            for j in range(dim_y):
                if int(im[i,j]) < -128:
                    im[i,j] = -128
                elif int(im[i,j]) > 127:
                    im[i,j] = 127


    # Add 128, convert to 8bit ints
    ster_images = [(a + 128).astype(np.uint8) for a in ster_images]

    # convert to uint8 & save (as png)
    Image.fromarray(ster_images[0]).save(out_filename_1)
    Image.fromarray(ster_images[1]).save(out_filename_2)


if __name__ == '__main__':
    parse_args()
