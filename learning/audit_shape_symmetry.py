#!/usr/bin/env python3
"""Research-only audit: 8x8 synthetic piece catalog symmetry closure.

Do NOT blindly rotate/flip replay samples for a learned value model: any
symmetry changing a generated shape into a shape absent from the actual
generator catalog changes the learning distribution. Reads the exact catalog
from learn_combo_guarded.c rather than maintaining a second hand-copied list.
This is a cheap static audit, not gameplay training or an official-game claim.
"""
from pathlib import Path
import re

C_SOURCE = Path(__file__).with_name("learn_combo_guarded.c")


def load_patterns():
    source = C_SOURCE.read_text(encoding="utf-8")
    match = re.search(
        r"static\s+const\s+char\s*\*\s*patterns\s*\[\s*N_SHAPES\s*\]\s*=\s*\{(.*?)\};",
        source,
        re.S,
    )
    if not match:
        raise AssertionError("Cannot locate exact synthetic pattern catalog")
    patterns = re.findall(r'"([^"]+)"', match.group(1))
    shape_count = re.search(r"#define\s+N_SHAPES\s+(\d+)", source)
    if not shape_count or len(patterns) != int(shape_count.group(1)):
        raise AssertionError("Synthetic shape catalog count changed")
    return patterns


def normalized(coords):
    x0 = min(x for x, _ in coords)
    y0 = min(y for _, y in coords)
    return frozenset((x - x0, y - y0) for x, y in coords)


def parse_shape(pattern):
    return normalized(
        {(x, y) for y, row in enumerate(pattern.split("/"))
         for x, symbol in enumerate(row) if symbol == "#"}
    )


def transform(coords, quarter_turns, mirror):
    out = coords
    for _ in range(quarter_turns):
        out = normalized({(-y, x) for x, y in out})
    if mirror:
        out = normalized({(-x, y) for x, y in out})
    return out


def main():
    patterns = load_patterns()
    catalog = [parse_shape(p) for p in patterns]
    if len(set(catalog)) != len(catalog):
        raise AssertionError("Duplicate synthetic shape in the catalog")
    totals = {}
    for turns in range(4):
        for mirror in (False, True):
            missing = [
                i for i, shape in enumerate(catalog)
                if transform(shape, turns, mirror) not in set(catalog)
            ]
            totals[(turns, mirror)] = len(missing)
            print(f"rotation={90 * turns:3} mirror={int(mirror)} "
                  f"missing={len(missing)}/{len(catalog)} "
                  f"missing_shape_indices={missing}")
    assert totals[(0, False)] == 0
    unsafe = {k: n for k, n in totals.items() if k != (0, False) and n > 0}
    if unsafe:
        print("SYMMETRY_AUGMENTATION_AUDIT=NOT_CLOSED; "
              "do not apply unsupervised 8-way D4 augmentation")
    else:
        print("SYMMETRY_AUGMENTATION_AUDIT=CATALOG_CLOSED; "
              "still verify per-generator probabilities before augmentation")


if __name__ == "__main__":
    main()
