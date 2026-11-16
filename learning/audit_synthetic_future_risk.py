#!/usr/bin/env python3
"""Audit source-defined offline generator probabilities and board accessibility.

Diagnostic only: NOT a gameplay policy, NOT a genuine future-tray predictor,
NOT official Block Blast score/death probability. Reads the source shape
catalog; no access to hidden future RNG or training/holdout game seeds.
"""
import argparse
from pathlib import Path
import re

SOURCE = Path(__file__).with_name("learn_combo_guarded.c")
BIG = frozenset((9, 10, 11, 12, 13, 14, 21, 22, 23, 24, 25, 26, 27))
HARD = frozenset((4, 8, 10, 11, 21, 22, 23, 24, 25, 26, 27))


def catalog():
    text = SOURCE.read_text(encoding="utf-8")
    match = re.search(
        r"static\s+const\s+char\s*\*\s*patterns\s*\[\s*N_SHAPES\s*\]\s*=\s*\{(.*?)\};",
        text, re.S,
    )
    if match is None:
        raise ValueError("Could not extract synthetic shape catalog")
    raw = re.findall(r'"([^"]+)"', match.group(1))
    n = re.search(r"#define\s+N_SHAPES\s+(\d+)", text)
    if n is None or len(raw) != int(n.group(1)):
        raise ValueError("Unexpected generator catalog size")
    masks = []
    for pattern in raw:
        rows = pattern.split("/")
        coords = [(x, y) for y, row in enumerate(rows)
                  for x, ch in enumerate(row) if ch == "#"]
        if not coords:
            raise ValueError("Empty shape")
        width = max(x for x, _ in coords) + 1
        height = max(y for _, y in coords) + 1
        masks.append((sum(1 << (y * 8 + x) for x, y in coords), width, height))
    return masks


def shape_priors(shape_count):
    if shape_count != 28:
        raise ValueError("Prior formula requires the audited 28-shape simulator")
    priors = {"normal": [], "heavy": [], "shifted-heavy": []}
    for i in range(shape_count):
        normal = 1.0 / 28
        heavy = 1.0 / 84 + (2.0 / (3 * len(BIG)) if i in BIG else 0.0)
        shifted = heavy / 4 + (3.0 / (4 * len(HARD)) if i in HARD else 0.0)
        priors["normal"].append(normal)
        priors["heavy"].append(heavy)
        priors["shifted-heavy"].append(shifted)
    for name, mass in priors.items():
        if abs(sum(mass) - 1.0) > 1e-12:
            raise AssertionError(f"{name} prior mass does not sum to 1")
    return priors


def count_placements(board, piece):
    mask, w, h = piece
    return sum(not (board & (mask << (8 * row + col)))
               for row in range(9 - h) for col in range(9 - w))


def audit(board, shapes):
    if board < 0 or board > (1 << 64) - 1:
        raise ValueError("Board must be a 64-bit nonnegative occupancy mask")
    fits = [count_placements(board, shape) for shape in shapes]
    report = {}
    for name, mass in shape_priors(len(shapes)).items():
        zero_mass = sum(weight for weight, n in zip(mass, fits) if n == 0)
        one_mass = sum(weight for weight, n in zip(mass, fits) if n == 1)
        # This is ONLY the chance one of three independently drawn nominal
        # future pieces has no *immediate* placement on the unchanged board.
        # Other pieces can clear lines first; it is not a forced-death rate.
        report[name] = {
            "zero_fit_piece_probability": zero_mass,
            "single_fit_piece_probability": one_mass,
            "any_of_three_zero_fit_proxy": 1 - (1 - zero_mass) ** 3,
        }
    return report


def selftest(shapes):
    empty = audit(0, shapes)
    full = audit((1 << 64) - 1, shapes)
    for name in empty:
        assert abs(empty[name]["zero_fit_piece_probability"]) < 1e-12
        assert abs(full[name]["zero_fit_piece_probability"] - 1) < 1e-12
        assert abs(full[name]["any_of_three_zero_fit_proxy"] - 1) < 1e-12
    a = audit(0x000000000103070f, shapes)
    b = audit(0x00000000f103070f, shapes)
    for name in a:
        assert b[name]["zero_fit_piece_probability"] >= (
            a[name]["zero_fit_piece_probability"] - 1e-12
        )
    print("SYNTHETIC_PRIOR_RISK_SELFTEST=PASS")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--board", help="64-bit board hex occupancy, e.g. 0x0")
    args = parser.parse_args()
    shapes = catalog()
    selftest(shapes)
    if args.board:
        result = audit(int(args.board, 0), shapes)
        for name, values in result.items():
            print(f"{name}: " + " ".join(
                f"{key}={value:.5f}" for key, value in values.items()
            ))


if __name__ == "__main__":
    main()
