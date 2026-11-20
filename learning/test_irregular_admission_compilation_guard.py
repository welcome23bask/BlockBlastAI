"""Regression guard: crowded irregular finalist bonus must be executable C, not a // comment.

Research-only; no simulator seed consumption or production changes.
"""
import pathlib
import re
import unittest

SOURCE = pathlib.Path(__file__).with_name("BLTrainableStrategy.c")

def strip_c_comments(source):
    return re.sub(r"/\\*.*?\\*/|//[^\\n]*", "", source, flags=re.S)

class AdmissionCompilationGuard(unittest.TestCase):
    def test_both_family_condition_is_executable(self):
        raw = SOURCE.read_text()
        executable = strip_c_comments(raw)
        start = executable.index("record_afterstate(")
        end = executable.index("#if BL_RESEARCH_DUAL_POOL", start)
        body = executable[start:end]
        self.assertRegex(body, r"if\\s*\\(\\s*irregularA\\s*>=\\s*2\\s*&&\\s*irregularB\\s*>=\\s*2\\s*\\)")
        self.assertRegex(body, r"quality\\s*\\+=\\s*90\\s*\\*")
        self.assertRegex(body, r"64\\s*-\\s*bits\\s*\\(\\s*path->board\\s*\\)\\s*<\\s*24")
        self.assertNotIn(r"\\n", raw[raw.index("record_afterstate("):raw.index("#if BL_RESEARCH_DUAL_POOL")])

if __name__ == "__main__":
    unittest.main()
