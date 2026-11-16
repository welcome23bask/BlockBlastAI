#!/usr/bin/env python3
"""Extra research-only 32-bit RNG-start collision audit (no gameplay training).

The historical cross-role ledger excludes equal epoch IDs, but the legacy
simulator's additive seed function can alias raw RNG starting states across
different scenarios and epochs offset by 20/40. This conservative diagnostic
does NOT reserve seeds or modify historical results. Do not confuse raw seed
equality across different generators with identical play trajectories.
"""
from pathlib import Path
import argparse
import re

HERE = Path(__file__).parent
LEDGER = HERE / "tenx_used_epochs.txt"
TRAINER = HERE / "train_tenx.c"
GAMES = tuple(range(30)) + tuple(range(100, 108))


def raw_seed(epoch, scenario, game):
    return (
        163000000 + 20000000 * scenario
        + 1000000 * epoch + 13717 * game
    ) & 0xFFFFFFFF


def verify_source_formula():
    text = TRAINER.read_text(encoding="utf-8")
    required = (
        "163000000u", "20000000u*scenario",
        "epoch*1000000u", "game*13717u",
    )
    if not all(part in text for part in required):
        raise ValueError("Simulator seed formula changed: re-audit before use")


def reserved():
    ids = []
    for raw in LEDGER.read_text(encoding="utf-8").splitlines():
        line = raw.strip()
        if not line or line.startswith("#"):
            continue
        if not re.fullmatch(r"\d+", line):
            raise ValueError(f"Invalid epoch ledger row: {line}")
        ids.append(int(line))
    if len(set(ids)) != len(ids):
        raise ValueError("Cross-role ledger contains duplicate epoch entries")
    return ids


def seed_table(epochs):
    table = {}
    for epoch in epochs:
        for scenario in range(3):
            for game in GAMES:
                key = raw_seed(epoch, scenario, game)
                table.setdefault(key, []).append((epoch, scenario, game))
    return table


def check(train, holdout1, holdout2):
    verify_source_formula()
    used = set(reserved())
    epochs = (train, holdout1, holdout2)
    if (len(set(epochs)) != 3 or
        any(e < 17 or e > 2000 or e in used for e in epochs)):
        raise ValueError("Epochs must be distinct, unused and in 17..2000")
    past = seed_table(used)
    proposed = seed_table(epochs)
    for seed, roles in proposed.items():
        if seed in past:
            raise ValueError(
                f"Raw PRNG seed alias 0x{seed:08x}: "
                f"new {roles} versus historic {past[seed][:3]}"
            )
        if len(roles) > 1:
            raise ValueError(
                f"Raw PRNG seed alias 0x{seed:08x} within new run: {roles}"
            )
    return len(proposed)


def selftest():
    # Different training/holdout epoch IDs do NOT imply different raw
    # xorshift initial states when scenario offsets are additive.
    assert raw_seed(1955, 1, 0) == raw_seed(1975, 0, 0)
    assert raw_seed(1955, 2, 100) == raw_seed(1995, 0, 100)
    assert raw_seed(1600, 0, 0) != raw_seed(1611, 1, 0)
    verify_source_formula()
    print("RAW_SEED_ALIAS_SELFTEST=PASS")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("train", type=int, nargs="?")
    parser.add_argument("holdout1", type=int, nargs="?")
    parser.add_argument("holdout2", type=int, nargs="?")
    parser.add_argument("--selftest", action="store_true")
    args = parser.parse_args()
    if args.selftest:
        selftest()
        return
    if None in (args.train, args.holdout1, args.holdout2):
        parser.error("Specify train holdout1 holdout2, or --selftest")
    n = check(args.train, args.holdout1, args.holdout2)
    print(
        f"RAW_SEED_ALIAS_AUDIT=PASS epochs={args.train},{args.holdout1},"
        f"{args.holdout2} distinct_observed_rng_starts={n} "
        "conservative_history=all_reserved_epochs_and_generator_roles"
    )
    print(
        "Note: generator redraw changes still induce different gameplay "
        "even when legacy raw seed states match; this checks only starts."
    )


if __name__ == "__main__":
    main()
