#!/usr/bin/env python3
import unittest
import numpy as np

import pymera

class TestPymeraJobs(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        """Initialize Pymera before tests"""
        cls.fmr = pymera.Jobs()
        cls.fmr.init()
        return super().setUpClass()

    @classmethod
    def tearDownClass(cls):
        """Clean up Pymera after tests"""
        print()
        cls.fmr.exit()

    def test_jobs_version(self):
        self.assertNotEqual(self.fmr.get_version(), None)
        self.assertNotEqual(self.fmr.get_version(), '')
        self.assertEqual(self.fmr.get_version()[:6], 'Femera')

    def test_jobs_name(self):
        self.assertNotEqual(self.fmr.get_name(), None)
        self.assertNotEqual(self.fmr.get_name(), '')
        self.assertTrue("job" in self.fmr.get_name().lower())
        print ('\nHello ' + self.fmr.get_version()+' '+self.fmr.get_name()
             + ', '+"it's testing time!")
        print ("This won't hurt a bit... ;)")

    def test_jobs_init(self):
        self.assertTrue(self.fmr.did_init())

    def test_get_sims_name(self):
        self.assertTrue("sim" in self.fmr.get_sims_name().lower())

    def test_get_sims_version(self):
        self.assertTrue("femera" in self.fmr.get_sims_version().lower())

    def test_set_sims_version(self):
        keep_vers = self.fmr.get_sims_version()
        self.fmr.set_sims_version("Testing...")
        self.assertEqual(self.fmr.get_sims_version(),"Testing...")
        self.fmr.set_sims_version(keep_vers)
        self.assertTrue("femera" in self.fmr.get_sims_version().lower())

    def test_set_sims_name(self):
        keep_name = self.fmr.get_sims_name()
        self.fmr.set_sims_name("Testing...")
        self.assertEqual(self.fmr.get_sims_name(),"Testing...")
        self.fmr.set_sims_name(keep_name)
        self.assertTrue("sim" in self.fmr.get_sims_name().lower())

    def test_jobs_verbosity(self):
        self.assertGreaterEqual( self.fmr.get_verbosity(), 0)
        verb = self.fmr.get_verbosity()
        self.assertEqual(verb, 3)# default
        #TODO Fix set_verbosity() in C++ source.
        #self.assertEqual(self.fmr.set_verbosity(-1), 0)
        #self.assertLess(self.fmr.set_verbosity(999), 256)
        #self.assertEqual(self.fmr.set_verbosity(verb), verb)
        #self.assertEqual(self.fmr.get_verbosity(), verb)


if __name__ == '__main__':
    unittest.main()
    
