#!/usr/bin/env python
import random
import sys
import time
from skip import SkipDict

class timed(object):
    def __init__(self, output=sys.stdout):
        self.output = output

    def __call__(self, f, *args, **kwargs):
        def wrapper(test_name, *args, **kwargs):
            start = time.time()
            f(*args, **kwargs)
            stop = time.time()
            self.output.write("{0:50}: {1:6.6f} seconds\n".format(test_name, stop - start, ))
        return wrapper

def benchmark(items=1000000, output=sys.stdout):
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

    test_add("SkipDict item add %d keys" % items, skip, items)
    test_add("Dict item add %d keys" % items, dictionary, items)

    test_find("SkipDict item find %d keys" % items, skip)
    test_find("Dict item find %d keys" % items, dictionary)

    test_length("SkipDict len", skip, items - 1)
    test_length("Dict len", dictionary, items - 1)

    test_add("SkipDict item successive add of %d keys" % (items * 2), skip, items * 2)
    test_add("Dict item successive add of %d keys" % (items * 2), dictionary, items * 2)

    test_length("SkipDict len", skip, items * 2 - 1)
    test_length("Dict len", dictionary, items * 2 - 1)

def sort_benchmarks(items=100000, output=sys.stdout):
    random.seed()
    randints = [random.randint(0, 100) for x in xrange(0, items)]
    randints = [(i, i) for i in randints]

    @timed(output=output)
    def test_sort_skipdict():
        s = SkipDict(randints).keys()

    @timed(output=output)
    def test_sort_dict():
        d = sorted(dict(randints).keys())

    test_sort_skipdict("Test insert and sort %d keys (skipdict)" % items)
    test_sort_dict("Test insert and sort %d keys (dict)" % items)

def main(args):
    benchmark()
    #sort_benchmarks(100)
    #sort_benchmarks(1000000)
    #sort_benchmarks(2000000)
    return False

if __name__ == '__main__':
    sys.exit(main(sys.argv))
