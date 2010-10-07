"""
SkipDict specific tests
"""
from skip import SkipDict

try:
    import unittest2 as unittest
except ImportError:
    assert(sys.version_info >= (2, 7))
    import unittest

class SkipDictTest(unittest.TestCase):
    def setUp(self):
        self.skip = SkipDict()

    def test_ordered_keys(self):
        "Test that .keys() returns keys in sorted order"
        self.skip["chile"] = True
        self.skip["norway"] = True
        self.skip["zimbabwe"] = True
        self.skip["russia"] = True
        self.skip["australia"] = True
        self.skip["italy"] = True
        self.skip["germany"] = True

        keys = self.skip.keys()
        self.assertEqual(keys[0], "australia")
        self.assertEqual(keys[1], "chile")
        self.assertEqual(keys[2], "germany")
        self.assertEqual(keys[3], "italy")
        self.assertEqual(keys[4], "norway")
        self.assertEqual(keys[5], "russia")
        self.assertEqual(keys[6], "zimbabwe")
