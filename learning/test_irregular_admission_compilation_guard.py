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
        self.assertRegex(active, r"if\s*\(\s*irregularA\s*>=\s*2\s*&&\s*irregularB\s*>=\s*2\s*&&\s*BLResearchDisjointSquarePair\s*\(\s*path->board\s*\)\s*>\s*0\s*\)")
        self.assertRegex(active, r"quality\s*\+=\s*90\s*\*")
        self.assertRegex(active, r"64\s*-\s*bits\s*\(\s*path->board\s*\)\s*<\s*24")

    def test_negative_control_catches_commented_out_bonus(self):
        # Reproduce the historical failure mode without changing repository source.
        raw = SOURCE.read_text()
        needle = "if(irregularA>=2 && irregularB>=2 &&"
        self.assertIn(needle, raw)
        sabotaged = raw.replace(needle, "// " + needle, 1)
        executable = strip_c_comments(sabotaged)
        self.assertNotRegex(executable, r"if\s*\(\s*irregularA\s*>=\s*2\s*&&\s*irregularB\s*>=\s*2\s*&&\s*BLResearchDisjointSquarePair\s*\(\s*path->board\s*\)\s*>\s*0\s*\)")

    def test_both_family_condition_is_executable(self):
        raw = SOURCE.read_text()
        executable = strip_c_comments(raw)
        start = executable.index("record_afterstate(")
        end = executable.index("#if BL_RESEARCH_DUAL_POOL", start)
        body = executable[start:end]
        self.assertRegex(body, r"if\s*\(\s*irregularA\s*>=\s*2\s*&&\s*irregularB\s*>=\s*2\s*&&\s*BLResearchDisjointSquarePair\s*\(\s*path->board\s*\)\s*>\s*0\s*\)")
        self.assertRegex(body, r"quality\s*\+=\s*90\s*\*")
        self.assertRegex(body, r"64\s*-\s*bits\s*\(\s*path->board\s*\)\s*<\s*24")
        self.assertNotIn(r"\n", raw[raw.index("record_afterstate("):raw.index("#if BL_RESEARCH_DUAL_POOL")])

    def test_disjoint_square_rerank_is_enabled_exactly_once(self):
        raw = SOURCE.read_text()
        defines = re.findall(r"(?m)^\s*#define\s+BL_RESEARCH_DISJOINT_SQUARE_PAIR\s+(\d+)\s*$", raw)
        self.assertEqual(defines, ["1"], "Disjoint-square research flag must not be overridden")
        active = strip_c_comments(raw)
        for pool in ("topAll", "topCombo"):
            self.assertIn(f"if(64-bits(ctx->{pool}[i].board)<32)", active)
            self.assertIn(f"utility+=BLResearchDisjointSquarePair(ctx->{pool}[i].board);", active)

    def test_balanced_shifted_rerank_is_active_for_both_objectives(self):
        raw = SOURCE.read_text()
        self.assertEqual(raw.count("#define BL_RESEARCH_BALANCED_SHIFTED_RERANK 1"), 1)
        active = strip_c_comments(raw)
        self.assertIn("static int BLResearchBalancedShiftedRerank(uint64_t board)", active)
        self.assertIn("BLResearchDisjointSquarePair(board)==0", active)
        for pool in ("topAll", "topCombo"):
            self.assertIn(f"utility+=BLResearchBalancedShiftedRerank(ctx->{pool}[i].board);", active)

if __name__ == "__main__":
    unittest.main()
