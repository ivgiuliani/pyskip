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

    def test_ordered_string_keys(self):
        """
        Test that .keys() returns keys in sorted order with
        classic strings
        """
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

    def test_ordered_number_keys(self):
        """
        Test that .keys() returns keys in sorted order with numbers
        """
        self.skip["4"] = True
        self.skip["2"] = True
        self.skip["5"] = True
        self.skip["1"] = True
        self.skip["3"] = True

        keys = self.skip.keys()
        self.assertEqual(keys[0], "1")
        self.assertEqual(keys[1], "2")
        self.assertEqual(keys[2], "3")
        self.assertEqual(keys[3], "4")
        self.assertEqual(keys[4], "5")

    def test_ordered_instance_keys(self):
        """
        Test that .keys() returns instances in the right order when
        a __lt__ method has been defined on the class
        """

        class Key(object):
            def __init__(self, val):
                self.val = val
            def __lt__(self, other):
                return self.val < other.val
            def __repr__(self):
                return str(self.val)

        objects = [Key(k) for k in range(0, 5)]
        self.skip[objects[4]] = 4
        self.skip[objects[0]] = 0
        self.skip[objects[2]] = 2
        self.skip[objects[1]] = 1
        self.skip[objects[3]] = 3
        
        keys = self.skip.keys()
        self.assertEqual(keys[0], objects[0])
        self.assertEqual(keys[1], objects[1])
        self.assertEqual(keys[2], objects[2])
        self.assertEqual(keys[3], objects[3])
        self.assertEqual(keys[4], objects[4])
