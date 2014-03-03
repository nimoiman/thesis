#!/usr/bin/env python
import matplotlib.pylab as plt

from api_docs import command, parse_args, help

@command
def plot_codevectors_vs_training_data(training_set_filename, codevector_filename):
    """Make scatter plot of training data vs. codevectors
    """
    training_set_fp = open(training_set_filename, 'r')

    training_set = [[float(n) for n in l.split(',')] for l in training_set_fp.readlines()]

    codevector_fp = open(codevector_filename, 'r')

    codevectors = [[float(n) for n in c.split(',')] for c in codevector_fp.readlines()]

    plt.scatter([t[0] for t in training_set], [t[1] for t in training_set], c='r')
    plt.scatter([t[0] for t in codevectors], [t[1] for t in codevectors], c='b')

    plt.show()
    
@command
def scatter_2d(*args):
    for filename in args:
        fp = open(filename, 'r')
        data = [[float(n) for n in l.split(',')] for l in fp.readlines()]
        plt.scatter([t[0] for t in data], [t[1] for t in data])
    plt.show()

if __name__ == '__main__':
    parse_args()