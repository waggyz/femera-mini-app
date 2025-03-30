#!/usr/bin/env python3
import unittest
import numpy as np

import sys, os
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
import pymera
from pymera.enumerators import Sims_application

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
        self.assertIsNotNone(self.sims.get_name())
        self.assertNotEqual(self.sims.get_name(), '')
        self.assertTrue("sim" in self.sims.get_name().lower())
    def test_set_name(self):
        name = 'test sims name'
        self.sims.set_name(name)
        self.assertEqual(self.sims.get_name(), name)

    def test_get_version(self):
        self.assertIsNotNone(self.sims.get_version())
        self.assertNotEqual(self.sims.get_version(), '')
        self.assertTrue("femera" in self.sims.get_version().lower())
    def test_set_version(self):
        version = '0.0.0'
        self.sims.set_version(version)
        self.assertEqual(self.sims.get_version(), version)

    def test_get_application(self):
        self.assertIsNotNone(self.sims.get_application())
        self.assertEqual(self.sims.get_application(), Sims_application.UNKNOWN)
    def test_set_application(self):
        app = Sims_application.RESEARCH
        self.sims.set_application(app)
        self.assertEqual(self.sims.get_application(), app)
        app = Sims_application.UQ
        self.assertEqual(self.sims.set_application(app), app)
        self.assertEqual(self.sims.get_application(), app)

if __name__ == '__main__':
    unittest.main()
    
