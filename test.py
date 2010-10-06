#!/usr/bin/env python
import gc
import sys
import time
from functools import wraps
from skip import SkipDict

class timed(object):
    def __init__(self, output=sys.stdout):
        self.output = output

    def __call__(self, f, *args, **kwargs):
        def wrapper(test_name, *args, **kwargs):
            start = time.time()
            f(*args, **kwargs)
            stop = time.time()
            self.output.write("{0:20}: {1} seconds\n".format(test_name, stop - start, ))
        return wrapper


def speed_test(items=1000000, output=sys.stdout):
    skip, dictionary = SkipDict(), {}

    @timed(output=output)
    def test_add(d):
        "Add 1000000 items to the dictionary"
        for i in xrange(1, items):
            d[i] = i

    @timed(output=output)
    def test_find(d):
        "Find 100000 items in the dictionary"
        for i in xrange(1, items):
            assert(d[i] == i)

    @timed(output=output)
    def test_length(d):
        "Count items in the dictionary"
        assert(len(d) == (items - 1))

    test_add("SkipDict item add", skip)
    test_add("Dict item add", dictionary)

    test_find("SkipDict item find", skip)
    test_find("Dict item find", dictionary)

    test_length("SkipDict len", skip)
    test_length("Dict len", dictionary)

def main(args):
    speed_test()
    return False

if __name__ == '__main__':
    sys.exit(main(sys.argv))
