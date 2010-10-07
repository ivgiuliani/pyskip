#!/usr/bin/env python

import os
import sys

try:
    # Python < 2.7 import
    import unittest2 as unittest
except ImportError:
    try:
        assert(sys.version_info >= (2, 7))
    except AssertionError:
        sys.stderr.write("Either install unittest2 or upgrade your Python version to 2.7\n")
        sys.exit(-1)
    import unittest

def main(args):
    loader = unittest.TestLoader()
    cwd = os.path.abspath(os.path.dirname(__file__))
    test_dir = os.path.join(cwd, "tests")
    suite = loader.discover(start_dir=test_dir, pattern="*.py")
    unittest.TextTestRunner().run(suite)
    return False

if __name__ == '__main__':
    sys.exit(main(sys.argv))

