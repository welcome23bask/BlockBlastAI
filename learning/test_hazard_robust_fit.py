"""Synthetic fixtures only: test train-only robust Brier fitting."""
import math
from train_lower_tail_hazard import fit, metrics

def build():
    rows = []
    for s in ("normal","heavy","shifted-heavy"):
        for game in range(20):
            y = int(game >= 10)
            x = [1.,float(y)]+[0.]*8
            rows.append((600,s,"train",y,x,(600,s,game)))
            rows.append((601,s,"validation",y,x,(601,s,game)))
    return rows

rows = build()
w = fit(rows)
assert all(math.isfinite(z) for z in w)
assert w == fit(rows)
changed = [(e,s,split,1-y if split=="validation" else y,x,k)
           for e,s,split,y,x,k in rows]
assert w == fit(changed), "validation labels influenced training"
for s,m in metrics(rows,w,"validation").items():
    assert m["brier"] < m["constant_prevalence_brier"], (s,m)
print("ROBUST_HAZARD_FIT_TEST=PASS (fixture only, no gameplay claim)")
