#!/usr/bin/env python3
import unittest
import numpy as np

import sys, os
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
import pymera

class TestPymeraSims(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        """Initialize Pymera before tests"""
        cls.jobs = pymera.Jobs()
        cls.jobs.init()
        cls.jobs.add_sims()
        cls.sims = cls.jobs.sims[-1]
        return super().setUpClass()

    @classmethod
    def tearDownClass(cls):
        """Clean up Pymera after tests"""
        print()
        cls.jobs.exit()

    def test_trivial(self):
        self.assertTrue(True)

    def test_get_name(self):
        self.assertNotEqual(self.sims.get_name(), None)
        self.assertNotEqual(self.sims.get_name(), '')
        self.assertTrue("sim" in self.sims.get_name().lower())
    def test_set_name(self):
        name = 'test sims name'
        self.sims.set_name(name)
        self.assertEqual(self.sims.get_name(), name)

    def test_get_version(self):
        self.assertNotEqual(self.sims.get_version(), None)
        self.assertNotEqual(self.sims.get_version(), '')
        self.assertTrue("femera" in self.sims.get_version().lower())
    def test_set_version(self):
        version = '0.0.0'
        self.sims.set_name(version)
        self.assertEqual(self.sims.get_name(), version)

'''
    def test_get_sims_version(self):
        self.assertTrue("femera" in self.jobs.get_sims_version().lower())

    def test_set_sims_version(self):
        keep_vers = self.jobs.get_sims_version()
        self.jobs.set_sims_version("Testing...")
        self.assertEqual(self.jobs.get_sims_version(),"Testing...")
        self.jobs.set_sims_version(keep_vers)
        self.assertTrue("femera" in self.jobs.get_sims_version().lower())
'''


if __name__ == '__main__':
    unittest.main()
    
