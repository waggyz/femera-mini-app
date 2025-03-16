#!/bin/python3
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

    def test_jobs_init(self):
        self.assertTrue(self.fmr.did_init())

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
    
