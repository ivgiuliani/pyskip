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
            self.output.write("{0:35}: {1:6.6f} seconds\n".format(test_name, stop - start, ))
        return wrapper


def speed_test(items=1000000, output=sys.stdout):
    skip, dictionary = SkipDict(), {}

    @timed(output=output)
    def test_add(d, items):
        "Add 1000000 items to the dictionary"
        for i in xrange(1, items):
            d[i] = i

    @timed(output=output)
    def test_find(d):
        "Find 100000 items in the dictionary"
        for i in xrange(1, items):
            assert(d[i] == i)

    @timed(output=output)
    def test_length(d, items):
        "Count items in the dictionary"
        assert(len(d) == items)

    test_add("SkipDict item add", skip, items)
    test_add("Dict item add", dictionary, items)

    test_find("SkipDict item find", skip)
    test_find("Dict item find", dictionary)

    test_length("SkipDict len", skip, items - 1)
    test_length("Dict len", dictionary, items - 1)

    test_add("SkipDict item successive large add", skip, items * 2)
    test_add("Dict item successive large add", dictionary, items * 2)

    test_length("SkipDict len", skip, items * 2 - 1)
    test_length("Dict len", dictionary, items * 2 - 1)


def correctness_test(output=sys.stdout):
    skip = SkipDict()

    try:
        skip["unknown key"]
        sys.stdout.write("KeyError check NOT PASSED\n")
    except KeyError:
        sys.stdout.write("KeyError check ok\n")
        pass

    sys.stdout.write("Repr format: ")
    if not repr(skip).startswith("<SkipDict"):
        sys.stdout.write("FAILED\n")
    else:
        sys.stdout.write("OK\n")

def main(args):
    speed_test()
    correctness_test()
    return False

if __name__ == '__main__':
    sys.exit(main(sys.argv))
