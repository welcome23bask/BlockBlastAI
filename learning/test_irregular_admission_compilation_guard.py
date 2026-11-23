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
    def test_crowded_root_scarcity_order_is_active(self):
        # The hypothesis must be executable within the guarded fair-root path.
        raw = strip_c_comments(SOURCE.read_text())
        start = raw.index("static void search_fair_first(")
        end = raw.index("#endif", start)  # comments were stripped above
        body = raw[start:end]
        self.assertIn("int order[3]={0,1,2},legal[3]={0,0,0};", body)
        self.assertIn("legal[order[j]]<legal[order[i]]", body)
        self.assertIn("int p=order[oi];", body)
        self.assertIn("ctx->budget=ctx->visited+quota;", body)
        self.assertIn("ctx->budget=fullBudget;", body)

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
        # The C preprocessor expands UINT64_C(0x...) to suffixed integer
        # literals. Check the active predicate in preprocessed code without
        # requiring the unexpanded macro spelling (source test below does).
        compact = re.sub(r"\s+", "", active)
        self.assertIn("if(irregularA>=3&&irregularB>=3&&shiftedCross>=3&&", compact)
        self.assertIn("BLResearchFreeFragmentationPenalty(path->board)>=-400", compact)
        self.assertRegex(active, r"pair_fits_now\s*\(\s*path->board\s*,\s*0x20303(?:UL|LU|U|L)?\s*,\s*3\s*,\s*3\s*,\s*0x20702(?:UL|LU|U|L)?\s*,\s*3\s*,\s*3\s*\)")
        self.assertRegex(active, r"pair_fits_now\s*\(\s*path->board\s*,\s*0x20702(?:UL|LU|U|L)?\s*,\s*3\s*,\s*3\s*,\s*0x20303(?:UL|LU|U|L)?\s*,\s*3\s*,\s*3\s*\)")
        self.assertRegex(active, r"fit\s*\(\s*path->board\s*,\s*0x1f(?:UL|LU|U|L)?\s*,\s*5\s*,\s*1\s*\)\s*>=\s*2")
        self.assertRegex(active, r"fit\s*\(\s*path->board\s*,\s*0x0101010101(?:UL|LU|U|L)?\s*,\s*1\s*,\s*5\s*\)\s*>=\s*2")
        self.assertIn("path->broken==0&&path->misses==0&&path->lines>=2&&64-bits(path->board)>=12", compact)
        self.assertRegex(active, r"quality\s*\+=\s*90\s*\*")
        self.assertRegex(active, r"64\s*-\s*bits\s*\(\s*path->board\s*\)\s*<\s*24")

    def test_two_bar_landings_are_required_in_active_source(self):
        active = strip_c_comments(SOURCE.read_text())
        self.assertIn("fit(path->board,UINT64_C(0x1f),5,1)>=2", active)
        self.assertIn("fit(path->board,UINT64_C(0x0101010101),1,5)>=2", active)

    def test_negative_control_catches_commented_out_bonus(self):
        # Reproduce the historical failure mode without changing repository source.
        raw = SOURCE.read_text()
        needle = "if(irregularA>=3 && irregularB>=3 && shiftedCross>=3 &&"
        self.assertIn(needle, raw)
        self.assertIn("shiftedCross>=3", raw, "Third irregular family must remain required")
        sabotaged = raw.replace(needle, "// " + needle, 1)
        executable = strip_c_comments(sabotaged)
        self.assertNotRegex(executable, r"if\s*\(\s*irregularA\s*>=\s*3\s*&&\s*irregularB\s*>=\s*3\s*&&\s*shiftedCross\s*>=\s*3\s*&&\s*pair_fits_now\s*\(\s*path->board\s*,\s*UINT64_C\s*\(\s*0x20303\s*\)\s*,\s*3\s*,\s*3\s*,\s*UINT64_C\s*\(\s*0x20702\s*\)\s*,\s*3\s*,\s*3\s*\)\s*&&\s*pair_fits_now\s*\(\s*path->board\s*,\s*UINT64_C\s*\(\s*0x20702\s*\)\s*,\s*3\s*,\s*3\s*,\s*UINT64_C\s*\(\s*0x20303\s*\)\s*,\s*3\s*,\s*3\s*\)\s*&&\s*BLResearchDisjointSquarePair\s*\(\s*path->board\s*\)\s*>=\s*3\s*&&\s*BLResearchFreeFragmentationPenalty\s*\(\s*path->board\s*\)\s*>=\s*-400\s*&&\s*\(\s*fit\s*\(\s*path->board\s*,\s*UINT64_C\s*\(\s*0x1f\s*\)\s*,\s*5\s*,\s*1\s*\)\s*>=\s*2\s*\|\|\s*fit\s*\(\s*path->board\s*,\s*UINT64_C\s*\(\s*0x0101010101\s*\)\s*,\s*1\s*,\s*5\s*\)\s*>=\s*2\s*\)\s*&&\s*path->broken\s*==\s*0\s*&&\s*path->misses\s*==\s*0\s*&&\s*path->lines\s*>=\s*2\s*&&\s*64\s*-\s*bits\s*\(\s*path->board\s*\)\s*>=\s*12\s*\)")

    def test_both_family_condition_is_executable(self):
        raw = SOURCE.read_text()
        executable = strip_c_comments(raw)
        start = executable.index("record_afterstate(")
        end = executable.index("#if BL_RESEARCH_DUAL_POOL", start)
        body = executable[start:end]
        self.assertRegex(body, r"if\s*\(\s*irregularA\s*>=\s*3\s*&&\s*irregularB\s*>=\s*3\s*&&\s*shiftedCross\s*>=\s*3\s*&&\s*pair_fits_now\s*\(\s*path->board\s*,\s*UINT64_C\s*\(\s*0x20303\s*\)\s*,\s*3\s*,\s*3\s*,\s*UINT64_C\s*\(\s*0x20702\s*\)\s*,\s*3\s*,\s*3\s*\)\s*&&\s*pair_fits_now\s*\(\s*path->board\s*,\s*UINT64_C\s*\(\s*0x20702\s*\)\s*,\s*3\s*,\s*3\s*,\s*UINT64_C\s*\(\s*0x20303\s*\)\s*,\s*3\s*,\s*3\s*\)\s*&&\s*BLResearchDisjointSquarePair\s*\(\s*path->board\s*\)\s*>=\s*3\s*&&\s*BLResearchFreeFragmentationPenalty\s*\(\s*path->board\s*\)\s*>=\s*-400\s*&&\s*\(\s*fit\s*\(\s*path->board\s*,\s*UINT64_C\s*\(\s*0x1f\s*\)\s*,\s*5\s*,\s*1\s*\)\s*>=\s*2\s*\|\|\s*fit\s*\(\s*path->board\s*,\s*UINT64_C\s*\(\s*0x0101010101\s*\)\s*,\s*1\s*,\s*5\s*\)\s*>=\s*2\s*\)\s*&&\s*path->broken\s*==\s*0\s*&&\s*path->misses\s*==\s*0\s*&&\s*path->lines\s*>=\s*2\s*&&\s*64\s*-\s*bits\s*\(\s*path->board\s*\)\s*>=\s*12\s*\)")
        self.assertRegex(body, r"quality\s*\+=\s*90\s*\*")
        self.assertRegex(body, r"quality\s*\+=\s*90\s*\*\s*\(\s*path->streak\s*>=\s*3\s*\)")
        self.assertRegex(body, r"64\s*-\s*bits\s*\(\s*path->board\s*\)\s*<\s*24")
        self.assertIn("irregularA<shiftedCross?irregularA:shiftedCross", body)
        self.assertIn("if (64-bits(path->board)<24)", body)
        self.assertNotIn("shiftedCross>=3 && 64-bits(path->board)<32", body)
        self.assertIn("path->broken==0", body)
        self.assertIn("64-bits(path->board)>=12", body)
        self.assertIn("irregularB<shiftedCross?irregularB:shiftedCross", body)
        self.assertRegex(body, r"\+\s*75\s*\*\s*\(\s*shiftedCross\s*>=\s*3\s*&&\s*irregularA\s*>=\s*3\s*&&\s*irregularB\s*>=\s*3\s*&&\s*path->broken\s*==\s*0\s*&&\s*path->streak\s*>=\s*2\s*&&\s*path->misses\s*==\s*0\s*&&\s*path->lines\s*>=\s*2\s*&&\s*64\s*-\s*bits\s*\(\s*path->board\s*\)\s*<\s*20\s*\)")
        self.assertNotIn(r"\n", raw[raw.index("record_afterstate("):raw.index("#if BL_RESEARCH_DUAL_POOL")])

    def test_signed_fragmentation_gate_has_real_rejecting_cases(self):
        # Numeric boundary regression: -100 per stranded cell, with the
        # admission predicate >= -400. The old <= 4 accepted every case.
        raw = strip_c_comments(SOURCE.read_text())
        fn = raw[raw.index("int BLResearchFreeFragmentationPenalty("):
                 raw.index("int BLResearchFutureClearAccess(")]
        self.assertRegex(fn, r"int\s+fragmented\s*=\s*total\s*-\s*largest")
        self.assertRegex(fn, r"return\s+-100\s*\*\s*fragmented\s*;")
        self.assertRegex(raw, r"BLResearchFreeFragmentationPenalty\s*\(\s*path->board\s*\)\s*>=\s*-400")
        for stranded in (0, 1, 4):
            self.assertGreaterEqual(-100 * stranded, -400)
        for stranded in (5, 8, 16):
            self.assertLess(-100 * stranded, -400)
        self.assertTrue(all(-100 * n <= 4 for n in range(17)),
                        "Historical <=4 check was a no-op")

    def test_disjoint_square_rerank_is_disabled_for_current_ablation(self):
        # Keep admission's exact geometric predicate executable, but do not
        # accidentally enable the late utility bonus during this ablation.
        raw = SOURCE.read_text()
        defines = re.findall(r"(?m)^\s*#define\s+BL_RESEARCH_DISJOINT_SQUARE_PAIR\s+(\d+)\s*$", raw)
        self.assertEqual(defines, ["0"], "Current research ablation must remain OFF")
        active = strip_c_comments(raw)
        for pool in ("topAll", "topCombo"):
            self.assertIn(f"if(64-bits(ctx->{pool}[i].board)<32)", active)
            self.assertIn(f"utility+=BLResearchDisjointSquarePair(ctx->{pool}[i].board);", active)

    def test_exact_afterstate_dedup_preserves_small_legal_differences(self):
        raw = SOURCE.read_text()
        active = strip_c_comments(raw)
        self.assertIn("if(pool[i].board==path->board &&", active)
        for field in ("misses", "streak", "broken"):
            self.assertIn(f"pool[i].{field}==path->{field}", active)
        self.assertNotIn("bits(pool[i].board^path->board)<=", active)
        self.assertIn("if(*count<BL_FUTURE_TOP)", active)

    def test_crowded_only_fair_root_experiment_is_active(self):
        raw = SOURCE.read_text()
        active = strip_c_comments(raw)
        self.assertRegex(raw, r"(?m)^#define BL_RESEARCH_FAIR_ROOT 1$")
        self.assertIn("if(64-bits(board)<20)search_fair_first(&ctx,&start);", active)
        self.assertIn("else search(&ctx,&start);", active)
        self.assertIn("if(ctx->visited>=fullBudget)", active)
        self.assertIn("ctx->budget=fullBudget;", active)

    def test_twenty_distinct_legal_finalists_are_configured(self):
        raw = SOURCE.read_text()
        defines = [line.strip() for line in raw.splitlines() if line.startswith("#define BL_FUTURE_TOP ")]
        self.assertEqual(defines, ["#define BL_FUTURE_TOP 20"])
        active = strip_c_comments(raw)
        self.assertIn('if(*count<BL_FUTURE_TOP)', active)
        self.assertIn('for(int i=1;i<BL_FUTURE_TOP;++i)', active)

if __name__ == "__main__":
    unittest.main()
