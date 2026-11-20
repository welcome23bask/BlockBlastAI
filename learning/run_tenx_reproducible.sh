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
# One cross-role epoch ledger prevents train/holdout exclusion lists from drifting.
# The audit covers documented runs, not unrecorded local sessions.
ledger="learning/tenx_used_epochs.txt"
if [[ ! -f "$ledger" ]]; then
  echo "ERROR: missing required cross-role seed ledger: $ledger" >&2
  exit 2
fi
if ! awk 'NF && $1 !~ /^#/ {if(NF!=1 || $1 !~ /^[0-9]+$/ || seen[$1]++) exit 1}' "$ledger"; then
  echo "ERROR: malformed or duplicate epoch in $ledger" >&2
  exit 2
fi
# Load reserved epochs once, then enforce the same exclusions for every role.
mapfile -t known_used < <(awk 'NF && $1 !~ /^#/ {print $1}' "$ledger")
# A source checkpoint can remain unchanged after failed gates; never mistake
# the old logical checkpoint epoch for a fresh training namespace.
train_epoch="${TENX_TRAIN_SEED_EPOCH:-}"
if [[ ! "$train_epoch" =~ ^[0-9]+$ ]] ||
   ((train_epoch < 17 || train_epoch > 2000 ||
      train_epoch == a || train_epoch == b)); then
  echo "Supply a distinct provisional TENX_TRAIN_SEED_EPOCH in 17..2000." >&2
  exit 2
fi
for e in "$a" "$b" "$train_epoch"; do
  for used in "${known_used[@]}"; do
    if ((e == used)); then
      echo "ERROR: seed epoch $e already reserved (training/holdout cross-role)" >&2
      exit 2
    fi
  done
done
# Additional research-only fail-closed audit: distinct epoch IDs do NOT
# guarantee distinct legacy 32-bit RNG starts across scenario offsets 20/40.
# Conservatively compare all three proposed role epochs and all reserved
# historical epochs, without modifying the executing seed snapshot.
python3 learning/audit_raw_seed_aliases.py "$train_epoch" "$a" "$b"
# Fail closed on the previously observed commented-out finalist-admission defect.
# Run the actual compiler/preprocessor regression before either preflight success
# or training. This consumes no simulator seeds and never edits production code.
PYTHONDONTWRITEBYTECODE=1 python3 learning/test_irregular_admission_compilation_guard.py -v
# No-compile validation before Actions minutes are spent on training.
if [[ "${TENX_PREFLIGHT_ONLY:-0}" == "1" ]]; then
  printf 'SEED_PREFLIGHT=OK training=%s holdouts=%s,%s (documented-ledger + source-verified raw-start audit; undocumented history unverified)\n' "$train_epoch" "$a" "$b"
  exit 0
fi
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
  printf "EVIDENCE_DIRECTORY=%s\n" "$evidence_dir"
  printf "SOURCE_REVISION=%s\n" "$rev"
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
cc -O2 -std=c11 -Wall -Wextra -Isrc learning/tenx_rules_selftest.c v17.o trained.o solver.o -lm -o tenx_rules_selftest
./tenx_rules_selftest
cp learning/tenx_checkpoint.txt tenx_checkpoint.txt
echo "SOURCE_REVISION=$rev"
echo "BASELINE_CHECKPOINT=$(cat tenx_checkpoint.txt)"
echo "PROVISIONAL_TRAIN_SEED_EPOCH=$train_epoch"
echo "PROVISIONAL_HOLDOUT_EPOCHS=$a,$b"
# One bounded epoch with a new training namespace, even though unchanged
# checkpoint weights may still resume at logical epoch 16.
train_games="${TENX_TRAIN_GAMES:-3}"
if [[ ! "$train_games" =~ ^[0-9]+$ ]] || ((train_games<1 || train_games>30)); then
  echo "ERROR: TENX_TRAIN_GAMES must be 1..30" >&2
  exit 2
fi
echo "TRAINING_GAMES_PER_GENERATOR=$train_games"
TENX_TRAIN_SEED_EPOCH="$train_epoch" ./tenx_lab train 1 "$train_games" | tee training.txt
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
    assert s.count("capped_games=0")==6, "Right-censored game; increase cap before certification"
    assert "nan" not in s.lower()
    verdicts=re.findall(r"^TEN_X_VERDICT=(.+)$",s,re.M)
    assert len(verdicts)==1, "Missing or duplicated verdict"
    assert verdicts[0]=="ALL HELDOUT GATES PASSED", f"Holdout {epoch} failed: {verdicts[0]}"
    print(f"HOLDOUT_{epoch}=ALL HELDOUT GATES PASSED")
print("Both synthetic holdouts passed the built-in gates; independent replication and real-game validation remain separate requirements.")
print("No result is an official Block Blast score.")
PY
echo "Research run completed; evidence retained outside the checkout. No checkpoint was promoted."
