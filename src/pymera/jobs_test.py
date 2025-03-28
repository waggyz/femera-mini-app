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

    def test_jobs_version(self):
        self.assertNotEqual(self.jobs.get_version(), None)
        self.assertNotEqual(self.jobs.get_version(), '')
        self.assertEqual(self.jobs.get_version()[:6], 'Femera')

    def test_jobs_name(self):
        self.assertNotEqual(self.jobs.get_name(), None)
        self.assertNotEqual(self.jobs.get_name(), '')
        self.assertTrue("job" in self.jobs.get_name().lower())
        print ('\nHello ' + self.jobs.get_version()
               +' '+self.jobs.get_name()
               +', '+"it's testing time!")
        print ("This won't hurt a bit... ;)")

    def test_jobs_init(self):
        self.assertTrue(self.jobs.did_init())

    def test_jobs_get_verbosity(self):
        self.assertGreaterEqual( self.jobs.get_verbosity(), 0)
        verb = self.jobs.get_verbosity()
        self.assertEqual(verb, 3)# default


    @unittest.expectedFailure
    def test_jobs_set_verbosity(self):
        #TODO Fix set_verbosity() in C++ source.
        self.assertEqual(self.jobs.set_verbosity(-1), 0)
        self.assertLess(self.jobs.set_verbosity(999), 256)
        self.assertEqual(self.jobs.set_verbosity(verb), verb)
        self.assertEqual(self.jobs.get_verbosity(), verb)

    def test_jobs_get_sims_n(self):
        self.assertGreaterEqual(self.jobs.get_sims_n(), 0)

    def test_jobs_add_sims(self):
        n=self.jobs.get_sims_n()
        sims = self.jobs.add_sims("New simulation")
        self.assertEqual(self.jobs.get_sims_n(),n+1)
        #self.assertIsInstance(sims, pymera.Sims) #TODO why failing?
        self.assertIsInstance(sims.jobs, pymera.Jobs)
        self.assertEqual(sims.get_name(), "New simulation")

if __name__ == '__main__':
    unittest.main()

