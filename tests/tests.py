#!/usr/bin/env python

import unittest, sys

import test_hashkeys
import test_hashtable
import test_markers
import test_ibdstructures

if __name__ == '__main__':
    dtl = unittest.defaultTestLoader

    ts = unittest.TestSuite([
            dtl.loadTestsFromModule(test_hashkeys),
            dtl.loadTestsFromModule(test_markers),
            dtl.loadTestsFromModule(test_ibdstructures),
            dtl.loadTestsFromModule(test_hashtable)])

    if '--verbose' in sys.argv:
        unittest.TextTestRunner(verbosity=2).run(ts)
    else:
        unittest.TextTestRunner().run(ts)
