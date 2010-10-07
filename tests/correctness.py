#!/usr/bin/env python
import sys
from skip import SkipDict

try:
    import unittest2 as unittest
except ImportError:
    assert(sys.version_info >= (2, 7))
    import unittest


class BasicTest(unittest.TestCase):
    def setUp(self):
        self.skip = SkipDict()

    def test_insert(self):
        "Test insert of new elements"
        self.skip["hello"] = "world"
        self.assertEqual(self.skip["hello"], "world")

        self.skip["buenos"] = "dias"
        self.assertEqual(self.skip["buenos"], "dias")

    def test_replace(self):
        "Check that replacement of objects works correctly"
        self.skip["key"] = "value"
        self.skip["key"] = "modified"
        self.assertEqual(self.skip["key"], "modified")


