#!/usr/bin/env python3
"""Fail closed when bounded 10x workflow has inconsistent epoch references.

Research-only static check. Run BEFORE editing workflow to trigger Actions.
It intentionally does not reserve seeds or certify historical independence.
"""
import pathlib
import re
import sys

def check(text: str) -> tuple[int, int, int, int, int]:
    pre = re.search(r'TENX_HAZARD_TRAIN_EPOCH=(\d+) TENX_HAZARD_VALIDATION_EPOCH=(\d+) TENX_TRAIN_SEED_EPOCH=(\d+) bash learning/run_tenx_reproducible.sh (\d+) (\d+)', text)
    if not pre:
        raise ValueError("Missing exact five-role preflight")
    ids = tuple(map(int, pre.groups()))
    if len(set(ids)) != 5 or any(not 17 <= n <= 2000 for n in ids):
        raise ValueError("Invalid five-role epochs")
    hazard, validation, train, a, b = ids
    required = [
        rf'audit_raw_seed_aliases.py {hazard} {validation} {train} {a} {b}',
        rf'hazard_trace_sim" {hazard} {validation} ',
        rf"model\['train_epochs'\]==\[{hazard}\]",
        rf"model\['validation_epochs'\]==\[{validation}\]",
        rf'TENX_HAZARD_TRAIN_EPOCH={hazard} TENX_HAZARD_VALIDATION_EPOCH={validation} TENX_TRAIN_SEED_EPOCH={train} TENX_TRAIN_GAMES=5 bash learning/run_tenx_reproducible.sh {a} {b}',
    ]
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
