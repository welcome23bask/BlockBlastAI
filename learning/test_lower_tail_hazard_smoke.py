#!/usr/bin/env python3
"""Research-only smoke/regression test. Synthetic rows are NOT gameplay evidence."""
import csv
import json
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent
FIELDS = ["epoch","scenario","game","tray","finalist","split","dead_within_3","censored",
          "occupied","square_fits","hbar_fits","vbar_fits","hard_zero",
          "largest_free","pockets","combo_streak","future_clear"]
SCENARIOS = ("normal", "heavy", "shifted-heavy")

def dataset(path, duplicate=False, censored=False, omit=None):
    rows = []
    for split, epochs in (("train",(401,402)), ("validation",(403,404))):
        for epoch in epochs:
            for si, scenario in enumerate(SCENARIOS):
                if (epoch, scenario) == omit:
                    continue
                for game in range(12):
                    for finalist in range(2):
                        dead = int((game + si + epoch + finalist) % 4 == 0)
                        rows.append(dict(epoch=epoch,scenario=scenario,game=game,tray=0,
                            finalist=finalist,split=split,dead_within_3=dead,
                            censored=int(censored and epoch == 403 and game == 0),
                            occupied=10+game*3,square_fits=max(0,8-game//2),
                            hbar_fits=max(0,7-game//2),vbar_fits=max(0,6-game//2),
                            hard_zero=int(game>8),largest_free=54-game*3,
                            pockets=game//4,combo_streak=game%4,future_clear=game%5))
    if duplicate:
        rows.append(dict(rows[0]))
    with path.open("w",newline="",encoding="utf-8") as f:
        writer=csv.DictWriter(f,fieldnames=FIELDS)
        writer.writeheader()
        writer.writerows(rows)

def execute(dataset_path,output_path):
    return subprocess.run([sys.executable,str(ROOT/"train_lower_tail_hazard.py"),
                           str(dataset_path),"--output",str(output_path)],
                          capture_output=True,text=True,timeout=30)

def main():
    with tempfile.TemporaryDirectory(prefix="tenx-hazard-selftest-") as temp:
        d=Path(temp)/"rows.csv"
        o=Path(temp)/"model.json"
        dataset(d)
        result=execute(d,o)
        assert result.returncode == 0, result.stderr
        model=json.loads(o.read_text(encoding="utf-8"))
        assert model["train_epochs"] == [401,402]
        assert model["validation_epochs"] == [403,404]
        assert set(model["per_epoch_validation"]) == set(SCENARIOS)
        for scenario in SCENARIOS:
            assert set(model["per_epoch_validation"][scenario]) == {"403","404"}
            assert all(isinstance(v["passes_unrounded"],bool)
                       for v in model["per_epoch_validation"][scenario].values())
        for kwargs in ({"duplicate":True},{"censored":True},
                       {"omit":(403,"shifted-heavy")}):
            dataset(d,**kwargs)
            o.unlink(missing_ok=True)
            failed=execute(d,o)
            assert failed.returncode != 0, "Malformed dataset was accepted: "+str(kwargs)
        print("PASS: bounded trainer smoke, split/epoch coverage, duplicate and censor rejection")
        print("Synthetic fixture only; no gameplay training or 10x evidence")

if __name__ == "__main__":
    main()
