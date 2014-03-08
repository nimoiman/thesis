#!/usr/bin/env python
from api_docs import command, parse_args, help

@command
def ster2csv(filename_1, filename_2, out_filename, block_size=8):
    """Extract pixel data from stereo grayscale images filename_1,
    filename_2, perform normalized DCT-II on block_size x block_size
    blocks and output to a 2-column csv of DCT coefficient pairs

    .. note::
        If size of images is not multiple of block_size, will crop from
        the right and bottom such that the two images have dimensions
        which are multiples of block_size, and are the same size.
    """
    from PIL import Image
    import numpy as np
    from scipy.fftpack import dct

    ster_images = [Image.open(filename_1), Image.open(filename_2)]

    # Crop images to size multiple of block_size
    dim_x = min([im.size[0] for im in ster_images])
    dim_y = min([im.size[1] for im in ster_images])
    if dim_x % block_size != 0 or dim_y % block_size != 0:
        dim_x = round(dim_x, block_size)
        dim_y = round(dim_y, block_size)
        print("cropping images to width of {}x{}px".format(dim_x, dim_y))
        ster_images = [im.crop(0, 0, dim_x, dim_y) for im in ster_images]

    # transform to np.array
    ster_images = [np.array(im, dtype=float) for im in ster_images]

    # allocate empty arrays for dct coefficients
    matrix_dcts = [np.empty((dim_x, dim_y), dtype=float) for im in ster_images]

    # put pixel blocks in range [-127.5, 127.5]
    ster_images = [im - 127.5 for im in ster_images]

    for row in range(0, dim_x, block_size):
        for col in range(0, dim_y, block_size):
            # Take DCT block by block
            matrix_dcts[0][row:row+block_size, col:col+block_size] = \
                dct(ster_images[0][row:row+block_size, col:col+block_size],
                    norm='ortho')
            matrix_dcts[1][row:row+block_size, col:col+block_size] = \
                dct(ster_images[1][row:row+block_size, col:col+block_size],
                    norm='ortho')

    # Serialize the two matrices in a left-to-right, top-to-bottom fashion
    # & write to file
    with open(out_filename, 'w') as f:
        [f.write(str(matrix_dcts[0].flatten()[i]) + ',' +
                 str(matrix_dcts[1].flatten()[i]) + '\n')
         for i in range(matrix_dcts[0].size)]

@command
def csv2ster(csv_filename, out_filename_1, out_filename_2, dim_x, dim_y,
             block_size=8):
    """Extract DCT coefficiencts from 2-column csv_filename, perform 
    inverse DCT on block_size x block_size blocks and output to two
    .png's"""
    from PIL import Image
    import numpy as np
    from scipy.fftpack import idct

    # Find how many lines in csv_filename
    lines = 0
    for line in open(csv_filename, 'r'):
        lines += 1

    dim_x = int(dim_x)
    dim_y = int(dim_y)

    if lines != dim_x * dim_y:
        print("The provided dimensions do not match the number of lines " +
              "in the input file")
        exit(1)
    elif dim_x % block_size != 0 or dim_y % block_size != 0:
        print("The provided dimensions must be multiples of block " +
              "size {}".format(block_size))
        exit(1)

    ster_images = [np.empty((dim_x, dim_y), dtype=float),
                   np.empty((dim_x, dim_y), dtype=float)]

    # Reshape images to read in serial data
    ster_images = [a.reshape(dim_x*dim_y) for a in ster_images]

    # Read in image data from csv
    with open(csv_filename, 'r') as f:
        for i in range(dim_x * dim_y):
            line = f.readline().split(',')
            ster_images[0][i] = float(line[0])
            ster_images[1][i] = float(line[1])

    # Reshape images to correct dimensions
    ster_images = [a.reshape(dim_x, dim_y) for a in ster_images]

    # Perform inverse DCT on blocks
    for i in range(0, dim_x, block_size):
        for j in range(0, dim_y, block_size):
            ster_images[0][i:i+block_size, j:j+block_size] = \
                idct(ster_images[0][i:i+block_size, j:j+block_size],
                     norm='ortho')
            ster_images[1][i:i+block_size, j:j+block_size] = \
                idct(ster_images[1][i:i+block_size, j:j+block_size],
                     norm='ortho')

    # Add 127.5, convert to ints
    ster_images = [(a + 127.5).astype(int) for a in ster_images]

    # Ensure no overflow errors
    assert(ster_images[0].min() >= 0)
    assert(ster_images[1].min() >= 0)
    assert(ster_images[0].max() <= 255)
    assert(ster_images[1].max() <= 255)

    # convert to uint8 & save (as png)
    Image.fromarray(ster_images[0].astype(np.uint8)).save(out_filename_1)
    Image.fromarray(ster_images[1].astype(np.uint8)).save(out_filename_2)



if __name__ == '__main__':
    parse_args()
