#!/usr/bin/env python3
import unittest
import numpy as np

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
             + ', '+"it's testing time!")
        print ("This won't hurt a bit... ;)")

    def test_jobs_init(self):
        self.assertTrue(self.jobs.did_init())

    def test_jobs_verbosity(self):
        self.assertGreaterEqual( self.jobs.get_verbosity(), 0)
        verb = self.jobs.get_verbosity()
        self.assertEqual(verb, 3)# default
        #TODO Fix set_verbosity() in C++ source.
        #self.assertEqual(self.jobs.set_verbosity(-1), 0)
        #self.assertLess(self.jobs.set_verbosity(999), 256)
        #self.assertEqual(self.jobs.set_verbosity(verb), verb)
        #self.assertEqual(self.jobs.get_verbosity(), verb)

    def test_jobs_add_sims(self):
        self.assertEqual(self.jobs.get_sims_n(),1)
        sims = self.jobs.add_sims("New simulation")
        self.assertEqual(self.jobs.get_sims_n(),2)
        self.assertIsInstance(sims, pymera.Sims)
        self.assertIsInstance(sims.jobs, pymera.Jobs)
        self.assertEqual(sims.get_name(), "New simulation")
        sims.set_name ("Test Sims name")
        sims.set_version ("0.0.1")
        self.assertEqual(sims.get_version(), "0.0.1")
'''
class TestPymeraSims(unittest.TestCase):
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

'''
'''
    def test_get_sims_name(self):
        self.assertTrue("sim" in self.jobs.get_sims_name().lower())

    def test_get_sims_version(self):
        self.assertTrue("femera" in self.jobs.get_sims_version().lower())

    def test_set_sims_version(self):
        keep_vers = self.jobs.get_sims_version()
        self.jobs.set_sims_version("Testing...")
        self.assertEqual(self.jobs.get_sims_version(),"Testing...")
        self.jobs.set_sims_version(keep_vers)
        self.assertTrue("femera" in self.jobs.get_sims_version().lower())

    def test_set_sims_name(self):
        keep_name = self.jobs.get_sims_name()
        self.jobs.set_sims_name("Testing...")
        self.assertEqual(self.jobs.get_sims_name(),"Testing...")
        self.jobs.set_sims_name(keep_name)
        self.assertTrue("sim" in self.jobs.get_sims_name().lower())
'''


if __name__ == '__main__':
    unittest.main()
    
