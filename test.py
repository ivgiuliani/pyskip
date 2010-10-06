#!/usr/bin/env python
import sys
import time
import gc
from skip import SkipDict

def speed_test(output=sys.stdout):
    s = SkipDict()
    output.write("Skipdict: ")
    start = time.time()
    for i in xrange(1, 1000000):
        s.set(i, i)
    stop = time.time()
    output.write("Time to add 1000000 items: %d secs\n" % (stop - start, ))

    d = {}

    start = time.time()
    output.write("Dict: ")
    for i in xrange(1, 1000000):
        d[i] = i
    stop = time.time()
    output.write("Time to add 1000000 items: %d secs\n" % (stop - start, ))

    start = time.time()
    output.write("Skipdict: ")
    for i in xrange(1, 1000000):
        assert(s.get(i) == i)
    stop = time.time()
    output.write("Time to find 1000000 items: %d secs\n" % (stop - start, ))

    start = time.time()
    output.write("Dict: ")
    for i in xrange(1, 1000000):
        d[i]
    stop = time.time()
    output.write("Time to find 1000000 items: %d secs\n" % (stop - start, ))

    start = time.time()
    output.write("SkipDict: ")
    assert(len(s) == 999999)
    stop = time.time()
    output.write("Time to calculate length of 1000000 items: %d secs\n" % (stop - start, ))

    start = time.time()
    output.write("Dict: ")
    assert(len(d) == 999999)
    stop = time.time()
    output.write("Time to calculate length of 1000000 items: %d secs\n" % (stop - start, ))

    del d

    gc.collect()
    print sys.getrefcount(s)

def main(args):
    speed_test()
    return False

if __name__ == '__main__':
    sys.exit(main(sys.argv))
