#!/usr/bin/env python3
"""Research-only trace-to-CSV bridge for completed three-tray continuations.

Input JSONL MUST be produced by the simulator, not synthesized by this tool.
This bridge does not run the simulator or establish trace provenance.
"""
import argparse
import csv
import json
from pathlib import Path

FEATURES = ('occupied','square_fits','hbar_fits','vbar_fits','hard_zero',
            'largest_free','pockets','combo_streak','future_clear')
FIELDS = ('epoch','scenario','game','tray','finalist','split','dead_within_3',
          'censored') + FEATURES

def convert(records):
    seen = set()
    output = []
    for lineno, record in enumerate(records, 1):
        r = json.loads(record)
        if set(r) != {'epoch','scenario','game','tray','finalist','split',
                      'legal','features','continuation'}:
            raise ValueError(f'line {lineno}: unexpected/missing trace fields')
        for field in ('epoch','game','tray','finalist'):
            if type(r[field]) is not int or r[field] < 0:
                raise ValueError(f'line {lineno}: invalid {field}')
        if not 17 <= r['epoch'] <= 2000:
            raise ValueError(f'line {lineno}: epoch outside 17..2000')
        if r['scenario'] not in ('normal','heavy','shifted-heavy'):
            raise ValueError(f'line {lineno}: invalid generator')
        if r['split'] not in ('train','validation'):
            raise ValueError(f'line {lineno}: invalid split')
        for field in FEATURES:
            value = r['features'][field]
            if type(value) is not int or value < 0:
                raise ValueError(f'line {lineno}: invalid afterstate feature {field}')
        features = r['features']
        if (features['occupied'] > 64 or features['largest_free'] > 64 or
            features['occupied'] + features['largest_free'] > 64 or
            features['hard_zero'] not in (0,1)):
            raise ValueError(f'line {lineno}: impossible 8x8 afterstate')
        key = tuple(r[k] for k in ('epoch','scenario','game','tray','finalist'))
        if key in seen:
            raise ValueError(f'line {lineno}: duplicate finalist')
        seen.add(key)
        if r['legal'] is not True:
            raise ValueError(f'line {lineno}: illegal or unverified afterstate')
        if set(r['features']) != set(FEATURES):
            raise ValueError(f'line {lineno}: incomplete or hidden feature')
        cont = r['continuation']
        if not isinstance(cont, list) or not 1 <= len(cont) <= 3:
            raise ValueError(f'line {lineno}: missing or excessive continuation')
        if any(type(c) is not dict or set(c) != {'completed'} or
               type(c['completed']) is not bool for c in cont):
            raise ValueError(f'line {lineno}: malformed continuation')
        failures = [i for i,c in enumerate(cont) if not c['completed']]
        if failures:
            if failures != [len(cont)-1]:
                raise ValueError(f'line {lineno}: continuation after death')
            dead = 1
        elif len(cont) == 3:
            dead = 0
        else:
            raise ValueError(f'line {lineno}: right-censored continuation')
        output.append({**{k:r[k] for k in ('epoch','scenario','game','tray','finalist','split')},
                       'dead_within_3':dead,'censored':0,**r['features']})
    if not output:
        raise ValueError('empty simulator trace')
    return output

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('trace', type=Path)
    parser.add_argument('--output', required=True, type=Path)
    args = parser.parse_args()
    with args.trace.open(encoding='utf-8') as f:
        rows = convert(f)
    with args.output.open('w',newline='',encoding='utf-8') as f:
        writer=csv.DictWriter(f,fieldnames=FIELDS)
        writer.writeheader()
        writer.writerows(rows)
    print(f'EXPORTED_VERIFIED_CONTINUATION_ROWS={len(rows)}')

if __name__ == '__main__':
    main()
