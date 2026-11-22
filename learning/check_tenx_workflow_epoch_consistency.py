#!/usr/bin/env python3
"""Fail closed when bounded 10x workflow has inconsistent epoch references.

Research-only static check. Run BEFORE editing workflow to trigger Actions.
It intentionally does not reserve seeds or certify historical independence.
"""
import pathlib
import re
import sys

# Audit the same legacy PRNG starting states used by the bounded runner,
# BEFORE a workflow-only commit can enqueue a job.
from audit_raw_seed_aliases import check as audit_raw_starts

LEDGER = pathlib.Path(__file__).with_name('tenx_used_epochs.txt')

def reserved_epochs() -> set[int]:
    if not LEDGER.is_file():
        raise ValueError(f'Missing required seed ledger: {LEDGER}')
    used = set()
    for line_no, raw in enumerate(LEDGER.read_text().splitlines(), 1):
        row = raw.strip()
        if not row or row.startswith('#'):
            continue
        if not re.fullmatch(r'[0-9]+', row):
            raise ValueError(f'Malformed ledger epoch at line {line_no}')
        epoch = int(row)
        if epoch in used:
            raise ValueError(f'Duplicate ledger epoch {epoch}')
        used.add(epoch)
    return used

def check(text: str) -> tuple[int, int, int, int, int]:
    pre = re.search(r'TENX_HAZARD_TRAIN_EPOCH=(\d+) TENX_HAZARD_VALIDATION_EPOCH=(\d+) TENX_TRAIN_SEED_EPOCH=(\d+) bash learning/run_tenx_reproducible.sh (\d+) (\d+)', text)
    if not pre:
        raise ValueError("Missing exact five-role preflight")
    if len(re.findall(r'TENX_PREFLIGHT_ONLY=1 TENX_HAZARD_TRAIN_EPOCH=', text)) != 1:
        raise ValueError('Expected exactly one executable five-role preflight')
    ids = tuple(map(int, pre.groups()))
    if len(set(ids)) != 5 or any(not 17 <= n <= 2000 for n in ids):
        raise ValueError("Invalid five-role epochs")
    collisions = sorted(set(ids) & reserved_epochs())
    if collisions:
        raise ValueError(f'Workflow uses already-reserved epochs: {collisions}')
    # A distinct epoch can still alias another generator's raw PRNG start.
    # Reuse the source-verified, ledger-aware cross-role audit; never infer
    # uniqueness from epoch numbers alone.
    audit_raw_starts(*ids)
    hazard, validation, train, a, b = ids
    required = [
        rf'audit_raw_seed_aliases.py {hazard} {validation} {train} {a} {b}',
        rf'hazard_trace_sim" {hazard} {validation} ',
        rf"model\['train_epochs'\]==\[{hazard}\]",
        rf"model\['validation_epochs'\]==\[{validation}\]",
        rf'TENX_HAZARD_TRAIN_EPOCH={hazard} TENX_HAZARD_VALIDATION_EPOCH={validation} TENX_TRAIN_SEED_EPOCH={train} TENX_TRAIN_GAMES=5 bash learning/run_tenx_reproducible.sh {a} {b}',
    ]
    # Fail closed when preserved artifacts advertise a different seed set.
    # Otherwise a failed run can upload plausible-looking evidence with
    # misleading epoch provenance even if its executable steps are correct.
    artifact_seed = (
        f'Provisional training seed {train} and holdouts {a},{b};'
    )
    if artifact_seed not in text:
        raise ValueError('Artifact seed provenance does not match executing roles')
    if text.count('seed-audit-warning.txt') != 1:
        raise ValueError('Expected exactly one seed-audit artifact writer')
    for i, item in enumerate(required):
        if i in (2, 3):
            if not re.search(item, text):
                raise ValueError(f"Mismatch in model epoch assertion {i}")
        elif item not in text:
            raise ValueError(f"Mismatch in workflow seed reference {i}: {item}")
    return ids

if __name__ == "__main__":
    p = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else ".github/workflows/v22-tenx.yml")
    try:
        print("WORKFLOW_EPOCH_CONSISTENCY=PASS", check(p.read_text()))
    except ValueError as e:
        print("WORKFLOW_EPOCH_CONSISTENCY=FAIL", str(e), file=sys.stderr)
        sys.exit(2)
