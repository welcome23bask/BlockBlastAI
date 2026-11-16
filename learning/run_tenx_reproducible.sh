#!/usr/bin/env bash
# v2.2 reproducible offline challenge. Run from a clean checkout of the
# research branch; no production files, GitHub Actions, or iPhone builds.
set -euo pipefail
cd "$(dirname "$0")/.."
branch="$(git branch --show-current)"
if [[ "$branch" != "feature/v2.2-10x-research-gate" ]]; then
  echo "ERROR: expected research branch, got $branch" >&2; exit 2
fi
if ! git diff --quiet || ! git diff --cached --quiet || [[ -n "$(git ls-files --others --exclude-standard)" ]]; then
  echo "ERROR: working tree must be clean for reproducibility" >&2; exit 2
fi
# Validate holdout arguments BEFORE spending time on training.
if [[ "$#" != 2 || ! "${1:-}" =~ ^[0-9]+$ || ! "${2:-}" =~ ^[0-9]+$ ]]; then
  echo "Supply TWO distinct, previously unused holdout epochs (17..2000)." >&2; exit 2
fi
a="$1"; b="$2"
if (( a < 17 || a > 2000 || b < 17 || b > 2000 || a == b )); then
  echo "Holdout epochs must be distinct and between 17 and 2000." >&2; exit 2
fi
for e in "$a" "$b"; do
  case "$e" in 92|93|989|1201|1202|1901|1902|1903|1904|1905|1906|1907|1908|1909|1910|1911|1912|1913|1914|1915|1916|1822)
    echo "Previously used/planned epoch: $e" >&2; exit 2;;
  esac
done
# Seed namespaces are cross-role: never reuse a previously planned holdout as training.\n# A source checkpoint may deliberately remain unchanged after failed gates.
# Never mistake a repeated checkpoint epoch for a fresh training seed.
train_epoch="${TENX_TRAIN_SEED_EPOCH:-}"
if [[ ! "$train_epoch" =~ ^[0-9]+$ ]] ||
   ((train_epoch < 17 || train_epoch > 2000 ||
      train_epoch == a || train_epoch == b)); then
  echo "Supply a distinct provisional TENX_TRAIN_SEED_EPOCH in 17..2000." >&2
  exit 2
fi
for used in 1716 1901 1902 1903 1904 1905 1906 1907 1908 1818 1819 1820 1821 1822 1915 1916 1909 1910 1911 1912 1913 1914; do
  if (( train_epoch == used )); then
    echo "Training seed epoch previously used/planned: $used" >&2; exit 2
  fi
done
rev="$(git rev-parse HEAD)"
work="$(mktemp -d)"
evidence_dir="$(mktemp -d "${TMPDIR:-/tmp}/blockblast-tenx-evidence.XXXXXX")"
# Preserve evidence even when a gate fails. Never write generated files into the checkout.
cleanup() {
  local status=$?
  for file in training.txt holdout-*.txt tenx_checkpoint.txt tenx_candidate_weights.txt; do
    for source in "$work"/$file; do
      if [[ -f "$source" ]]; then cp "$source" "$evidence_dir/"; fi
    done
  done
  printf "EVIDENCE_DIRECTORY=%s\\n" "$evidence_dir"
  printf "SOURCE_REVISION=%s\\n" "$rev"
  rm -rf "$work"
  return "$status"
}
trap cleanup EXIT
mkdir -p "$work/src" "$work/learning"
cp src/BLDualStrategy.c src/BLDualStrategy.h src/BlockBlastSolver.c src/BlockBlastSolver.h "$work/src/"
cp learning/BLTrainableStrategy.c learning/train_tenx.c learning/learn_combo_guarded.c learning/tenx_checkpoint.txt "$work/learning/"
# Included legacy trainer may use further includes from learning.
cp learning/*.c "$work/learning/"
cd "$work"
cc -O3 -std=c11 -Wall -Wextra -Isrc -DBLFindTwoStrategies=BLV17FindTwoStrategies -c src/BLDualStrategy.c -o v17.o
cc -O3 -std=c11 -Wall -Wextra -Isrc -c learning/BLTrainableStrategy.c -o trained.o
cc -O3 -std=c11 -Wall -Wextra -Isrc -c src/BlockBlastSolver.c -o solver.o
cc -O3 -std=c11 -Wall -Wextra -Isrc learning/train_tenx.c v17.o trained.o solver.o -lm -o tenx_lab
cp learning/tenx_checkpoint.txt tenx_checkpoint.txt
echo "SOURCE_REVISION=$rev"
echo "BASELINE_CHECKPOINT=$(cat tenx_checkpoint.txt)"
echo "PROVISIONAL_TRAIN_SEED_EPOCH=$train_epoch"
echo "PROVISIONAL_HOLDOUT_EPOCHS=$a,$b"
# One bounded epoch with a new training namespace, even though unchanged
# checkpoint weights may still resume at logical epoch 16.
TENX_TRAIN_SEED_EPOCH="$train_epoch" ./tenx_lab train 1 3 | tee training.txt
echo "TRAINED_CHECKPOINT=$(cat tenx_checkpoint.txt)"
echo "CANDIDATE_WEIGHTS=$(cat tenx_candidate_weights.txt)"
# Caller must verify these epoch IDs were never used for training or testing.
# The exclusions below cover only previously documented epochs, not every log.
for e in "$a" "$b"; do
  ./tenx_lab test 30 tenx_candidate_weights.txt "$e" | tee "holdout-$e.txt"
done
python3 - "$a" "$b" <<'PY'
import pathlib,re,sys
for epoch in sys.argv[1:]:
    s=pathlib.Path(f"holdout-{epoch}.txt").read_text()
    assert s.count("survival_ratio=")==3
    assert s.count("score_ratio=")==3
    assert s.count("p10_ratio=")==3
    assert s.count("illegal=0")==6
    assert "nan" not in s.lower()
    verdicts=re.findall(r"^TEN_X_VERDICT=(.+)$",s,re.M)
    assert len(verdicts)==1, "Missing or duplicated verdict"
    assert verdicts[0]=="ALL HELDOUT GATES PASSED", f"Holdout {epoch} failed: {verdicts[0]}"
    print(f"HOLDOUT_{epoch}=ALL HELDOUT GATES PASSED")
print("Both synthetic holdouts passed the built-in gates; independent replication and real-game validation remain separate requirements.")
print("No result is an official Block Blast score.")
PY
echo "Research run completed; evidence retained outside the checkout. No checkpoint was promoted."
