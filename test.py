#!/usr/bin/env python
import sys
import time
import gc
from skip import SkipDict

def speed_test(items_no=10000000):
    s = SkipDict()
    print "Skipdict: ",
    start = time.time()
    for i in xrange(1, items_no):
        s.set(i, i)
    stop = time.time()
    print "Time to add %d items: %d secs" % (items_no, stop - start, )

    print "Checking value correctness (skipdict)...",
    for i in xrange(1, items_no):
        assert(s.get(i) == i)
    print

    d = {}

    start = time.time()
    print "Dict: ",
    for i in xrange(1, items_no):
        d[i] = i
    stop = time.time()
    print "Time to add %d items: %d secs" % (items_no, stop - start, )

    print "Checking value correctness (dict)...",
    for i in xrange(1, items_no):
        assert(d[i] == i)
    print

    start = time.time()
    print "Skipdict: ",
    for i in xrange(1, items_no):
        try:
            assert(s.get(i) == i)
        except AssertionError:
            print "i: %d (s.get(%d) == %d)" % (i, i, s.get(i))
            raise
    stop = time.time()
    print "Time to find %d items: %d secs" % (items_no, stop - start, )

    start = time.time()
    print "Dict: ",
    for i in xrange(1, items_no):
        d[i]
    stop = time.time()
    print "Time to find %d items: %d secs" % (items_no, stop - start, )

    del s
    del d

    gc.collect()

def main():
    speed_test()

if __name__ == '__main__':
    main()
