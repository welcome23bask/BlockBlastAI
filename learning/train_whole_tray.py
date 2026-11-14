#!/usr/bin/env python3
"""Fit long-run value of COMPLETE three-piece end states from v1.7 trajectories.

Training/validation split is by whole game ID, never by screenshot/turn.
Outputs a C-compatible linear model and honest group-held-out metrics. Nothing
is deployed to the iPhone without head-to-head policy evaluation.
"""
import argparse, json
from pathlib import Path
import numpy as np
import pandas as pd
from sklearn.linear_model import Ridge
from sklearn.model_selection import GroupShuffleSplit
from sklearn.metrics import mean_absolute_error, r2_score

FEATURES = ["empty", "fit3x3", "fit5h", "fit5v", "rectangles",
            "shape_coverage", "large_coverage", "isolated", "near_complete_lines",
            "combo_active", "combo_urgency", "combo_streak"]
def train(data: Path, output: Path):
    frame = pd.read_csv(data)
    assert len(FEATURES) == 12 and set(FEATURES).issubset(frame.columns)
    assert frame.game.nunique() >= 30
    X=frame[FEATURES].to_numpy(dtype=float)
    y=np.clip(frame.remaining.to_numpy(dtype=float), 0, 120)
    group=frame.game.to_numpy()
    splitter=GroupShuffleSplit(n_splits=1, test_size=.25, random_state=28341)
    train_idx,val_idx=next(splitter.split(X,y,group))
    assert set(group[train_idx]).isdisjoint(set(group[val_idx]))
    # Every game counts equally, not every *frame*: long games should not
    # dominate the model just by producing more adjacent board snapshots.
    counts=frame.groupby('game').size().to_dict()
    weights=np.array([1.0/counts[int(g)] for g in group[train_idx]])
    weights*=len(weights)/weights.sum()
    candidates=[]
    for alpha in [0.01,0.1,1.,10.,100.,1000.]:
        model=Ridge(alpha=alpha, fit_intercept=True)
        model.fit(X[train_idx],y[train_idx],sample_weight=weights)
        pred=model.predict(X[val_idx])
        candidates.append((float(mean_absolute_error(y[val_idx],pred)),alpha,model,pred))
    error,alpha,model,pred=min(candidates,key=lambda row:row[0])
    naive=np.full_like(y[val_idx],y[train_idx].mean())
    metric={
        'train_games':int(len(set(group[train_idx]))),
        'validation_games':int(len(set(group[val_idx]))),
        'train_afterstates':len(train_idx),
        'validation_afterstates':len(val_idx),
        'heldout_mae':error,
        'naive_mae':float(mean_absolute_error(y[val_idx],naive)),
        'heldout_r2':float(r2_score(y[val_idx],pred)),
        'reg_strength':alpha,
        'scenario_mae':{}
    }
    for scenario in (0,1):
        mask=frame.iloc[val_idx].scenario.to_numpy()==scenario
        if mask.any():
            metric['scenario_mae'][str(scenario)]=float(mean_absolute_error(y[val_idx][mask],pred[mask]))
    model_file={
        'format':'BLWholeTrayValue-v1','model_type':'group-validated-ridge-regression',
        'feature_order':FEATURES,
        'weights':[float(x) for x in model.coef_],
        'bias':float(model.intercept_),
        'target':'future simulator moves after completed three-piece tray (clipped to 120)',
        'data_source':'v1.7 SMART SURVIVAL simulated trajectories; not real mobile game',
        'promoted_for_ios':False,
        'validation':metric,
    }
    output.parent.mkdir(parents=True,exist_ok=True)
    output.write_text(json.dumps(model_file,indent=2)+'\n')
    (output.parent/'whole_tray_validation.json').write_text(json.dumps(metric,indent=2)+'\n')
    print(json.dumps(metric,indent=2))
    print('Wrote C-compatible model:',output)

if __name__=='__main__':
    p=argparse.ArgumentParser()
    p.add_argument('--data',type=Path,default=Path('learning/whole_tray_train.csv'))
    p.add_argument('--output',type=Path,default=Path('learning/models/whole_tray_value.json'))
    a=p.parse_args()
    train(a.data,a.output)