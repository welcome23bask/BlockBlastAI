"""Regression guard: crowded irregular finalist bonus must be executable C, not a // comment.

Research-only; no simulator seed consumption or production changes.
"""
import pathlib
import re
import shutil
import subprocess
import tempfile
import unittest

SOURCE = pathlib.Path(__file__).with_name("BLTrainableStrategy.c")

def strip_c_comments(source):
    return re.sub(r"/\*.*?\*/|//[^\n]*", "", source, flags=re.S)

class AdmissionCompilationGuard(unittest.TestCase):
    def test_research_source_compiles_with_warnings_as_errors(self):
        compiler = shutil.which('cc')
        self.assertIsNotNone(compiler, 'C compiler required; do not silently skip research gate')
        root = SOURCE.parent.parent
        result = subprocess.run(
            [compiler, '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror',
             '-fsyntax-only', '-Isrc', str(SOURCE)],
            cwd=root, capture_output=True, text=True, timeout=30)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def test_preprocessed_object_contains_active_bonus(self):
        # Source text alone is insufficient: #if could remove the entire feature.
        compiler = shutil.which('cc')
        self.assertIsNotNone(compiler, 'C compiler required for active-code verification')
        root = SOURCE.parent.parent
        result = subprocess.run(
            [compiler, '-std=c11', '-E', '-P', '-Isrc', str(SOURCE)],
            cwd=root, capture_output=True, text=True, timeout=30)
        self.assertEqual(result.returncode, 0, result.stderr)
        active = result.stdout
        self.assertRegex(active, r"if\s*\(\s*irregularA\s*>=\s*2\s*&&\s*irregularB\s*>=\s*2\s*&&\s*fit\s*\(\s*path->board\s*,[^;]*0x70707[^;]*\)\s*>\s*0\s*\)")
        self.assertRegex(active, r"quality\s*\+=\s*90\s*\*")
        self.assertRegex(active, r"64\s*-\s*bits\s*\(\s*path->board\s*\)\s*<\s*24")

    def test_negative_control_catches_commented_out_bonus(self):
        # Reproduce the historical failure mode without changing repository source.
        raw = SOURCE.read_text()
        needle = "if(irregularA>=2 && irregularB>=2 &&"
        self.assertIn(needle, raw)
        sabotaged = raw.replace(needle, "// " + needle, 1)
        executable = strip_c_comments(sabotaged)
        self.assertNotRegex(executable, r"if\s*\(\s*irregularA\s*>=\s*2\s*&&\s*irregularB\s*>=\s*2\s*&&\s*fit\s*\(\s*path->board\s*,\s*UINT64_C\s*\(\s*0x70707\s*\)\s*,\s*3\s*,\s*3\s*\)\s*>\s*0\s*\)")

    def test_both_family_condition_is_executable(self):
        raw = SOURCE.read_text()
        executable = strip_c_comments(raw)
        start = executable.index("record_afterstate(")
        end = executable.index("#if BL_RESEARCH_DUAL_POOL", start)
        body = executable[start:end]
        self.assertRegex(body, r"if\s*\(\s*irregularA\s*>=\s*2\s*&&\s*irregularB\s*>=\s*2\s*&&\s*fit\s*\(\s*path->board\s*,\s*UINT64_C\s*\(\s*0x70707\s*\)\s*,\s*3\s*,\s*3\s*\)\s*>\s*0\s*\)")
        self.assertRegex(body, r"quality\s*\+=\s*90\s*\*")
        self.assertRegex(body, r"64\s*-\s*bits\s*\(\s*path->board\s*\)\s*<\s*24")
        self.assertNotIn(r"\n", raw[raw.index("record_afterstate("):raw.index("#if BL_RESEARCH_DUAL_POOL")])

if __name__ == "__main__":
    unittest.main()
