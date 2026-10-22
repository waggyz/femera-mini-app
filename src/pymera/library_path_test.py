#!/usr/bin/env python3
import os
from pathlib import Path
import runpy
import subprocess
import unittest
from unittest.mock import patch


WRAPPER = Path(__file__).resolve().with_name('pymera_libfemerac.py')
ROOT = WRAPPER.parents[2]


class TestLibraryPath(unittest.TestCase):
    def test_detected_cpu(self):
        with patch.dict(os.environ, {}, clear=True), \
                patch('subprocess.check_output', return_value='Ultra-7-255H\n') as detect, \
                patch('ctypes.CDLL') as load:
            runpy.run_path(str(WRAPPER))
        detect.assert_called_once_with(
            [str(ROOT / 'tools' / 'fmrmodel.sh')], text=True)
        load.assert_called_once_with(
            str(ROOT / 'build' / 'stage' / 'Ultra-7-255H' / 'lib' / 'libfemerac.so'))

    def test_explicit_library_directory(self):
        with patch.dict(os.environ, {'FMR_LIBDIR': '/custom/build/lib'}), \
                patch('subprocess.check_output') as detect, \
                patch('ctypes.CDLL') as load:
            runpy.run_path(str(WRAPPER))
        detect.assert_not_called()
        load.assert_called_once_with('/custom/build/lib/libfemerac.so')

    def test_detection_error_is_not_hidden(self):
        error = subprocess.CalledProcessError(1, 'fmrmodel.sh')
        with patch.dict(os.environ, {}, clear=True), \
                patch('subprocess.check_output', side_effect=error), \
                patch('ctypes.CDLL') as load:
            with self.assertRaises(subprocess.CalledProcessError):
                runpy.run_path(str(WRAPPER))
        load.assert_not_called()

    def test_loading_error_is_not_hidden(self):
        with patch.dict(os.environ, {'FMR_LIBDIR': '/missing/lib'}), \
                patch('ctypes.CDLL', side_effect=OSError('missing library')):
            with self.assertRaisesRegex(OSError, 'missing library'):
                runpy.run_path(str(WRAPPER))


if __name__ == '__main__':
    unittest.main()
