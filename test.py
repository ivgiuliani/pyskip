#!/usr/bin/env python
import sys
import time
import gc
from skip import SkipDict

def speed_test():
    s = SkipDict()
    print "Skipdict: ",
    start = time.time()
    for i in xrange(1, 1000000):
        s.set(i, i)
    stop = time.time()
    print "Time to add 1000000 items: %d secs" % (stop - start, )

    d = {}

    start = time.time()
    print "Dict: ",
    for i in xrange(1, 1000000):
        d[i] = i
    stop = time.time()
    print "Time to add 1000000 items: %d secs" % (stop - start, )

    start = time.time()
    print "Skipdict: ",
    for i in xrange(1, 1000000):
        assert(s.get(i) == i)
    stop = time.time()
    print "Time to find 1000000 items: %d secs" % (stop - start, )

    start = time.time()
    print "Dict: ",
    for i in xrange(1, 1000000):
        d[i]
    stop = time.time()
    print "Time to find 1000000 items: %d secs" % (stop - start, )

    #del s
    del d

    gc.collect()
    print sys.getrefcount(s)

def main():
    speed_test()

if __name__ == '__main__':
    main()
