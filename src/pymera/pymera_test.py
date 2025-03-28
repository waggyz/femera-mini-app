#!/usr/bin/env python3
import unittest
import numpy as np

import sys, os
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
import pymera

class TestPymeraJobs(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        """Initialize Pymera before tests"""
        cls.jobs = pymera.Jobs()
        cls.jobs.init()
        return super().setUpClass()

    @classmethod
    def tearDownClass(cls):
        """Clean up Pymera after tests"""
        print()
        cls.jobs.exit()

    def test_trivial(self):
        self.assertTrue(True)

if __name__ == '__main__':
    unittest.main()
    
