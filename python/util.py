#!/usr/bin/env python3
"""A collection of utilities for csv I/O for use with COVQ training
and testing, and some generic Python I/O utility functions
"""

# Python 2 compat
from __future__ import print_function

import traceback, sys, os
from api_docs import command, parse_args


def exit_(msg=1):
    traceback.print_stack()
    raise SystemExit(msg)


def stderr_(msg):
    print(msg, file=sys.stderr)


def mkdir_(path):
    try:
        os.mkdir(path)
    except FileExistsError as e:
        if os.path.isdir(path):
            return
        else:
            stderr_(("File by the name {} exists - " +
                     "cannot write over").format(path))
            raise e

def count_lines(f):
    """Return number of newlines in f"""
    n_lines = 0
    for line in open(f):
        n_lines += 1
    return n_lines


def count_dim(f):
    """Return number of entries in first line of f"""
    dim = 0
    with open(f) as fp:
        return len(fp.readline().split(","))


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


def csv_read(filename, dims, block_order=False, block_size=None):
    """Read a single column newline-separated file of floats, return
    a list of numpy arrays of dimensions specified by dims
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

    if lines != sum([dim[0] * dim[1] for dim in dims]):
        stderr_("The provided dimensions do not match the number of lines " +
              "in the input file")
        exit_(1)

    arrs = []
    for dim in dims:
        
        if not block_order:
            arr = np.empty(dim[0] * dim[1], dtype=float)
            with open(filename, 'r') as f:
                for i in range(dim[0] * dim[1]):
                    arr[i] = float(f.readline())

            arr = arr.reshape((dim[0], dim[1]))
            arrs.append(arr)
        else: # block_order
            arr = np.empty(dim, dtype=float)
            
            if block_size is not None:
                block_size = int(block_size)
                block_shape = (block_size, block_size)
                # Read as block by block serial data
                with open(filename, 'r') as f:
                    for block in iter_array(arr, block_shape):
                        for i in range(block_size):
                            for j in range(block_size):
                                arr[i,j] = float(f.readline())
            else: # block_size is None
                raise TypeError("No block size provided with option " +
                                "block_order=True")
            
    return arrs


def csv_write(filename, arr, mode='w', block_order=False,
              block_size=None):
    """Write a single column newline-separated file of floats from list
    of 2D numpy arrays :param:`arrs`
    """

    if len(arr.shape) != 2:
        raise ValueError('array should have dimension 2')

    if not block_order:
        with open(filename, mode) as f:
            for i in range(arr.shape[0]):
                for j in range(arr.shape[1]):
                    f.write(str(arr[i,j]) + "\n")
    else: # block_order
        if block_size is not None:
            with open(filename, mode) as f:
                for block in iter_array(arr, (block_size, block_size)):
                    for i in range(block_size):
                        for j in range(block_size):
                            f.write(str(arr[i,j]) + "\n")
        else: # block_size is None
            raise TypeError('block_size must be specified for block_order ' +
                            'output')


@command
def csv_zip(filename_1, filename_2, out_file):
    """Zip two files together line-by-line"""

    # Check that the number of lines is the same
    if count_lines(filename_1) != count_lines(filename_2):
        stderr_("Files provided do not have the same number of lines")
        exit_(1)

    lines = []
    with open(filename_1, 'r') as f_1:
        lines.append(f_1.readlines())
    with open(filename_2, 'r') as f_2:
        lines.append(f_2.readlines())

    with open(out_file, 'w') as f:
        for zipped_line in zip(lines[0], lines[1]):
            f.write("{},{}\n".format(zipped_line[0].strip(),
                                     zipped_line[1].strip()))


@command
def csv_unzip(filename, out_file_1, out_file_2, slice_index=1):
    """Unzip file into two files containing slice_index, and
    (original dim - slice_index) columns in each respectively"""
    with open(filename, 'r') as f:
        zipped_lines = [l.strip() for l in f.readlines()]

    with open(out_file_1, 'w') as f_1:
        for zipped_line in zipped_lines:
            f_1.write(','.join(zipped_line.split(',')[:slice_index]) + '\n')

    with open(out_file_2, 'w') as f_2:
        for zipped_line in zipped_lines:
            f_2.write(','.join(zipped_line.split(',')[slice_index:]) + '\n')

if __name__ == '__main__':
    parse_args()