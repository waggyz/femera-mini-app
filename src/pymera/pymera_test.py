#!/bin/python3
import unittest
import numpy as np

import pymera as fmr

class TestPymera(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        """Initialize Pymera before tests"""
        cls.fmr_jobs = fmr.Jobs()
        cls.fmr_jobs.init()
        return super().setUpClass()

    @classmethod
    def tearDownClass(cls):
        """Clean up Pymera after tests"""
        cls.fmr_jobs.exit()

    def test_version(self):
        self.assertNotEqual(self.fmr_jobs.get_version(), '')
        print ('Hello ' + self.fmr_jobs.get_version() + '!')
        print ("This won't hurt a bit...")

    def test_verbosity(self):
        self.assertGreaterEqual( self.fmr_jobs.get_verbosity(), 0)
        verb = self.fmr_jobs.get_verbosity()
        self.assertEqual(self.fmr_jobs.set_verbosity(-1), 0)
        self.assertLess(self.fmr_jobs.set_verbosity(999), 256)
        self.assertEqual(self.fmr_jobs.set_verbosity(verb), verb)
        self.assertEqual(self.fmr_jobs.get_verbosity(), verb)

if __name__ == '__main__':
    unittest.main()
    
