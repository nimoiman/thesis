#!/usr/bin/env python

if __name__ == '__main__':
    """Accept RGB image as input, output csv for red, blue"""
    import sys, os
    from PIL import Image

    for file_name in sys.argv[1:]:
        base, ext = os.path.splitext(file_name)
        print(file_name)
        im = Image.open(file_name)
        r, g, b = im.split()

        with open(base + '.raw', 'w') as f:
            for col in range(r.size[0]):
                for row in range(r.size[1]):
                    f.write(str(r.getpixel((col, row))) + ',' +
                            str(b.getpixel((col, row))) + '\n')

