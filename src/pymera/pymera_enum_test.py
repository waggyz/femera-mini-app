#!/usr/bin/env python3
import unittest

import pymera_enum as pnm

class TestPymeraJobs(unittest.TestCase):

    def test_standard_enum_members(self):
        for key, enum in pnm.fmr_enum.items():
            self.assertEqual(enum['NONE'].value , 0 )
            self.assertEqual(enum['ERROR'].value, 1 )
            self.assertEqual(enum['UNKNOWN'].value, 2 )
            self.assertGreater(enum['END'].value, 2)

if __name__ == '__main__':
    unittest.main()
