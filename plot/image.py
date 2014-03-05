#!/usr/bin/env python
from api_docs import command, parse_args, help

@command
def ster2csv(filename_1, filename_2, out_filename):
    """Extract pixel data from stereo grayscale images filename_1,
    filename_2, perform normalized DCT-II on 8x8 blocks and output to a
    2-column csv of DCT coefficient pairs

    .. note::
        If size of images is not multiple of 8, will crop from the right
        and bottom such that the two images have dimensions which are
        multiples of 8, and are the same size.
    """
    from PIL import Image
    import numpy as np
    from scipy.fftpack import dct

    im_1 = Image.open(filename_1)
    im_2 = Image.open(filename_2)

    dim_x = min((im_1.size[0], im_2.size[0]))
    if dim_x % 8 != 0:
        dim_x -= dim_x % 8
        print("cropping images to width of {}px".format(dim_x))
    dim_y = min((im_1.size[1], im_2.size[1]))
    if dim_y % 8 != 0:
        dim_y -= dim_y % 8
        print("cropping images to width of {}px".format(dim_y))

    im_1 = im_1.crop((0, 0, dim_x, dim_y))
    im_2 = im_2.crop((0, 0, dim_x, dim_y))

    # allocated, but uninitialized
    dct_out = np.empty((2, dim_x * dim_y))

    for i in range(0, dim_x, 8):
        for j in range(0, dim_y, 8):
            # 8x8 pixel block is in range [-127.5, 127.5]
            dct_out[0][i*dim_x + 8*j: i*dim_x + 8*j + 64] = \
                dct(np.array(im_1.crop((i, j, i+8, j+8))).astype(np.float64) -
                    127.5, norm='ortho').flatten()
            dct_out[1][i*dim_x + 8*j: i*dim_x + 8*j + 64] = \
                dct(np.array(im_2.crop((i, j, i+8, j+8))).astype(np.float64) -
                    127.5, norm='ortho').flatten()

    # write to file
    with open(out_filename, 'w') as f:
        [f.write(str(dct_out[0][i]) + ',' + str(dct_out[1][i]) + '\n')
         for i in range(dct_out[0].size)]


@command
def csv2ster(csv_filename, out_filename_1, out_filename_2, dim_x, dim_y):
    """Extract DCT coefficiencts from 2-column csv_filename, perform 
    inverse DCT on 8x8 blocks and output to two .png's"""
    from PIL import Image
    import numpy as np
    from scipy.fftpack import idct

    dim_x = int(dim_x)
    dim_y = int(dim_y)

    im_1 = np.empty((dim_x, dim_y), dtype=np.uint8)
    im_2 = np.empty((dim_x, dim_y), dtype=np.uint8)
    block_1 = np.empty((8, 8), dtype=np.float64)
    block_2 = np.empty((8, 8), dtype=np.float64)
    with open(csv_filename) as f:
        for j in range(0, dim_x, 8):
            for k in range(0, dim_y, 8):
                for i in range(64):
                    line = f.readline().split(',')
                    block_1[int(i/8)][i % 8] = float(line[0])
                    block_2[int(i/8)][i % 8] = float(line[1])
                im_1[k:k+8, j:j+8] = (idct(block_1, norm='ortho') + 128.5).astype(np.uint8)
                im_2[k:k+8, j:j+8] = (idct(block_2, norm='ortho') + 128.5).astype(np.uint8)
        im_1 = Image.fromarray(im_1)
        im_1.save(out_filename_1)
        im_2 = Image.fromarray(im_2)
        im_2.save(out_filename_2)



if __name__ == '__main__':
    parse_args()
