import contextlib
import io
import os
from pathlib import Path
import runpy
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch


RUNNER_PATH = Path(__file__).with_name('run_unittests_parallel')
RUNNER = runpy.run_path(str(RUNNER_PATH))


class TestOptionalMPI(unittest.TestCase):
    def test_commands_with_and_without_mpi(self):
        for launcher in (None, '/mock/mpiexec'):
            for verbosity in (0, 1, 2):
                with self.subTest(launcher=launcher, verbosity=verbosity):
                    with patch('shutil.which', return_value=launcher):
                        command = RUNNER['test_command']('sample_test.py', verbosity)
                    expected = ['python3', 'sample_test.py']
                    if verbosity:
                        expected.append('-v')
                    if launcher:
                        expected = ['mpiexec', '-n', '1'] + expected
                    self.assertEqual(command, expected)

    def test_both_runners_preserve_test_failure_without_mpi(self):
        for runner_name in ('IsolatedTestRunner', 'ParallelTestRunner'):
            with self.subTest(runner=runner_name):
                runner = RUNNER[runner_name]()
                if runner_name == 'IsolatedTestRunner':
                    runner.init(verbosity=0)
                result = subprocess.CompletedProcess([], 37, '', 'test failed')
                with patch('shutil.which', return_value=None), \
                        patch('subprocess.run', return_value=result) as run, \
                        contextlib.redirect_stdout(io.StringIO()):
                    if runner_name == 'IsolatedTestRunner':
                        with patch.object(runner, 'find_test_files',
                                          return_value=['sample_test.py']):
                            self.assertFalse(runner.run())
                    else:
                        self.assertFalse(runner.run_test('sample_test.py'))
                self.assertEqual(run.call_args.args[0],
                                 ['python3', 'sample_test.py'])

    def test_parallel_cli_with_and_without_mpi(self):
        for mpi in (False, True):
            for status in (0, 37):
                with self.subTest(mpi=mpi, status=status), \
                        tempfile.TemporaryDirectory() as directory:
                    root = Path(directory)
                    (root / 'python3').symlink_to(sys.executable)
                    test_file = root / 'sample_test.py'
                    test_file.write_text(
                        "import sys\n"
                        "print('test-output')\n"
                        "print('test-error', file=sys.stderr)\n"
                        "assert sys.argv[1:] == ['-v']\n"
                        f"sys.exit({status})\n"
                    )
                    if mpi:
                        launcher = root / 'mpiexec'
                        launcher.write_text(
                            '#!/bin/sh\n'
                            '[ "$1" = -n ] && [ "$2" = 1 ] || exit 91\n'
                            'shift 2\n'
                            'exec "$@"\n'
                        )
                        launcher.chmod(0o755)
                    output_dir = root / 'output'
                    result = subprocess.run(
                        [sys.executable, str(RUNNER_PATH), '-v', '-w', '1',
                         '-s', directory, '-p', '*_test.py',
                         '-o', str(output_dir)],
                        env={**os.environ, 'PATH': directory},
                        capture_output=True, text=True,
                    )
                    self.assertEqual(result.returncode, 0 if status == 0 else 1,
                                     result.stdout + result.stderr)
                    self.assertEqual(
                        (output_dir / 'sample_test.out').read_text(),
                        'test-output\n')
                    self.assertEqual(
                        (output_dir / 'sample_test.err').read_text(),
                        'test-error\n')


if __name__ == '__main__':
    unittest.main()
