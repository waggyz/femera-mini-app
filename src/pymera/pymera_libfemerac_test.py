#!/usr/bin/env python3
import unittest

from pymera_libfemerac import fmr

class TestPymeraJobs(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        """Initialize Pymera before tests"""
        cls.obj_create_ok = False
        try:
            cls.obj = fmr.fmr_new_jobs()
            cls.obj_create_ok = True
        except Exception as e:
            print(f"ERROR: {e}")
        #
        cls.jobs_init_ok = False
        try:
            fmr.fmr_jobs_init(cls.obj)
            cls.jobs_init_ok = True
        except Exception as e:
            print(f"ERROR: {e}")

        return super().setUpClass()

    @classmethod
    def tearDownClass(cls):
        """Clean up Pymera after tests"""
        print()
        fmr.fmr_jobs_exit(cls.obj)

    def test_fmr_new_jobs(self):
        self.assertTrue(self.obj_create_ok)
    if False:
        def test_fmr_delete_jobs(self):
            # not testing this here
            pass

    def test_fmr_jobs_init(self):
        self.assertTrue(self.jobs_init_ok)
    if False:
        def test_fmr_jobs_exit(self):
            # not testing this here
            pass
    def test_fmr_jobs_did_init(self):
        self.assertTrue(fmr.fmr_jobs_did_init(self.obj))

    def test_fmr_get_version(self):
        self.assertTrue(fmr.fmr_get_version(self.obj)[:6], 'Femera')

    def test_fmr_get_jobs_name(self):
        self.assertTrue('job' in str(fmr.fmr_get_jobs_name(self.obj)).lower())

    def test_fmr_get_verbosity(self):
        self.assertGreaterEqual(fmr.fmr_get_verbosity(self.obj), 0)
    if False:
        def test_fmr_set_verbosity(self): #TODO Femera verbosity is broken.
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
        vers = fmr.fmr_get_sims_version(self.obj, n).decode('utf-8')
        self.assertGreater(len(vers), 0)
    def test_fmr_set_sims_version(self):
        n=fmr.fmr_add_sims(self.obj)
        if(n>0):
            vers = 'test version name'
            fmr.fmr_set_sims_version(self.obj, n, vers.encode('utf-8'))
            vers_set=fmr.fmr_get_sims_version(self.obj, n).decode('utf-8')
            self.assertEqual(vers_set, vers)
        else:
            self.assertTrue(False)

if __name__ == '__main__':
    unittest.main()
