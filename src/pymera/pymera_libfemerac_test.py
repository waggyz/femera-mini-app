#!/usr/bin/env python3
import unittest

from pymera_libfemerac import fmr

class TestPymeraJobs(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        """Initialize Pymera before tests"""
        cls.obj =  fmr.fmr_new_jobs()
        fmr.fmr_jobs_init(cls.obj)
        #cls.jobs = pymera.Jobs()
        #cls.jobs.init()
        return super().setUpClass()

    @classmethod
    def tearDownClass(cls):
        """Clean up Pymera after tests"""
        print()
        fmr.fmr_jobs_exit(cls.obj)
        #cls.jobs.exit()

    def test_fmr_new_jobs(self): #TODO
        pass
    def test_fmr_delete_jobs(self): #TODO
        pass

    def test_fmr_jobs_init(self): #TODO
        #self.assertFalse(fmr.fmr_jobs_init(self.obj))
        # void return C functions return False
        pass
    def test_fmr_jobs_exit(self): #TODO
        pass
    def test_fmr_jobs_did_init(self):
        self.assertTrue(fmr.fmr_jobs_did_init(self.obj))

    def test_fmr_get_version(self):
        self.assertTrue(fmr.fmr_get_version(self.obj)[:6], 'Femera')

    def test_fmr_get_jobs_name(self):
        self.assertTrue('job' in str(fmr.fmr_get_jobs_name(self.obj)).lower())

    def test_fmr_get_verbosity(self):
        self.assertGreaterEqual(fmr.fmr_get_verbosity(self.obj), 0)
    def test_fmr_set_verbosity(self): #TODO
        pass

    def test_fmr_get_sims_n(self):
        n = fmr.fmr_get_sims_n(self.obj)
        self.assertGreaterEqual(n, 0)
    def test_fmr_add_sims(self):
        n = fmr.fmr_get_sims_n(self.obj)
        fmr.fmr_add_sims(self.obj)
        self.assertEqual(fmr.fmr_get_sims_n(self.obj), n+1)

    def test_fmr_get_sims_name(self):
        n=fmr.fmr_add_sims(self.obj)
        if (n>0):
            name = fmr.fmr_get_sims_name(self.obj, n-1).decode('utf-8')
            self.assertTrue('sim' in name.lower())
        else:
            self.assertTrue(False)
    def test_fmr_set_sims_name(self):
        n=fmr.fmr_add_sims(self.obj)
        if(n>0):
            name = 'test sim name'
            fmr.fmr_set_sims_name(self.obj, n, name.encode('utf-8'))
            name_set=fmr.fmr_get_sims_name(self.obj, n).decode('utf-8')
            self.assertEqual(name_set, name)
        else:
            self.assertTrue(False)

    def test_fmr_get_sims_version(self):
        n=fmr.fmr_add_sims(self.obj)
        self.assertGreater(fmr.fmr_get_sims_version(self.obj, n).len(), 0)
    def test_fmr_set_sims_version(self):
        pass

if __name__ == '__main__':
    unittest.main()
