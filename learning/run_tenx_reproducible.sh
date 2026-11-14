#!/usr/bin/env bash
# v2.2 reproducible offline challenge. Run from a clean checkout of the
# research branch; no production files, GitHub Actions, or iPhone builds.
set -euo pipefail
cd "$(dirname "$0")/.."
branch="$(git branch --show-current)"
if [[ "$branch" != "feature/v2.2-10x-research-gate" ]]; then
  echo "ERROR: expected research branch, got $branch" >&2; exit 2
fi
if ! git diff --quiet || ! git diff --cached --quiet; then
  echo "ERROR: working tree must be clean for reproducibility" >&2; exit 2
fi
rev="$(git rev-parse HEAD)"
work="$(mktemp -d)"
trap 'rm -rf "$work"' EXIT
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
# One bounded epoch; training seeds are derived from the checkpoint epoch 16.
./tenx_lab train 1 1 | tee training.txt
echo "TRAINED_CHECKPOINT=$(cat tenx_checkpoint.txt)"
echo "CANDIDATE_WEIGHTS=$(cat tenx_candidate_weights.txt)"
# Held-out epochs 1201 and 1202 are distinct from training epochs 0..16
# and prior documented holdouts 92,93,989,1201/1202 only if they have
# NOT previously been executed. Choose fresh unused epochs before running.
if [[ "${1:-}" == "" || "${2:-}" == "" ]]; then
  echo "Supply TWO previously unused distinct holdout epochs (0..2000)." >&2
  exit 2
fi
a="$1"; b="$2"
[[ "$a" =~ ^[0-9]+$ && "$b" =~ ^[0-9]+$ ]] || exit 2
(( a >= 17 && a <= 2000 && b >= 17 && b <= 2000 && a != b )) || exit 2
for e in "$a" "$b"; do
  case "$e" in 92|93|989|1201|1202) echo "Previously used/planned epoch: $e" >&2; exit 2;; esac
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
    assert "TEN_X_VERDICT=" in s
    print(f"HOLDOUT_{epoch}="+s.split("TEN_X_VERDICT=")[-1].strip())
print("No result is an official Block Blast score.")
PY
echo "Research run completed in temporary directory; no checkpoint was promoted."
