#!/usr/bin/env python3
"""Train provisional RGB second-opinion classifier on v3.2 ZIP extraction.
Requires: pillow, numpy, scikit-learn. Labels are UNREVIEWED predictions.
Never score this as independent correctness.
Usage: python scripts/train_v33_color_verifier.py path/to/extracted/dataset
"""
from pathlib import Path
from PIL import Image
from sklearn.linear_model import LogisticRegression
from sklearn.metrics import confusion_matrix
import sys,hashlib,json,random,numpy as np
folder=Path(sys.argv[1])
records=[]
for file in sorted(folder.glob('capture-*.json')):
    j=json.loads(file.read_text())
    jpg=file.with_suffix('.jpg')
    if not jpg.exists() or j.get('source')!='nativeScanner_unverified':
        continue
    picture=np.asarray(Image.open(jpg).convert('RGB'))
    br=j['boardRectInCrop']
    x0,y0,w,h=[br[k] for k in ['x','y','width','height']]
    cells=[]
    for row in range(8):
        for col in range(8):
            x=round(x0+(col+0.5)*w/8)
            y=round(y0+(row+0.5)*h/8)
            rgb=np.median(picture[y-2:y+3,x-2:x+3].reshape(-1,3),axis=0)
            cells.append(rgb/255)
    board=np.asarray(j['board'],dtype=np.uint8)
    records.append((hashlib.sha256(board.tobytes()).hexdigest(),
                    np.asarray(cells),board.flatten(),file.name))
if len(records)<20:raise SystemExit('Need at least 20 screenshot+JSON pairs')
groups=sorted({a for a,_,_,_ in records})
random.Random(144).shuffle(groups)
train=set(groups[:int(.70*len(groups))]);test=set(groups[int(.85*len(groups)):])
X=np.concatenate([r[1] for r in records if r[0] in train])
y=np.concatenate([r[2] for r in records if r[0] in train])
T=np.concatenate([r[1] for r in records if r[0] in test])
ty=np.concatenate([r[2] for r in records if r[0] in test])
model=LogisticRegression(C=10,max_iter=1000).fit(X,y)
pred=model.predict(T)
print('Screenshots:',len(records))
print('Held out cells:',len(ty))
print('Teacher agreement:',float(np.mean(pred==ty)))
print('Confusion:',confusion_matrix(ty,pred).tolist())
print('RGB coefficients:',model.coef_[0].tolist())
print('Intercept:',model.intercept_.tolist())
print('WARNING: scores measure agreement with unreviewed native scanner labels only.')
