# Research-only synthetic generator priors — exact source audit

Date: November 16, 2026 UTC. Based solely on `learning/learn_combo_guarded.c:new_tray` and `learning/train_tenx.c:tray_scenario` on the research branch. **NOT** observations of real Block Blast piece distributions, not a score claim and not a learned policy. No production files or checkpoints touched.

## Mechanism extracted from current simulator

Normal generator (`scenario==0`): each of the 28 simulated shapes 0..27 is drawn at approximately 1/28 per piece.

Heavy (`scenario==1`): draw an initial normal piece; with probability approximately 2/3 override it with a uniform draw from 13 `big` shapes `{9,10,11,12,13,14,21,22,23,24,25,26,27}`, otherwise keep the initial shape.

Shifted-heavy (`scenario==2`): first draw a heavy tray exactly as above; then for each of its three pieces, with probability approximately 3/4 **replace again** with one of 11 `hard` shapes `{4,8,10,11,21,22,23,24,25,26,27}` uniformly, otherwise keep the prior heavy draw. No actual future tray is exposed to the solver. Strictly, `random32()%k` introduces extremely small modulo bias; probabilities shown below are ideal-uniform theoretical approximations, not guaranteed i.i.d. PRNG outputs.

| Shape category (IDs) | Count | Per-shape normal | Per-shape heavy | Per-shape shifted-heavy |
| --- | ---: | ---: | ---: | ---: |
| Hard + big (10,11,21..27) | 9 | 3.5714% | 6.3187% | **8.3979%** |
| Hard, not big (4,8) | 2 | 3.5714% | 1.1905% | **7.1158%** |
| Big, not hard (9,12,13,14) | 4 | 3.5714% | 6.3187% | 1.5797% |
| Neither (remaining 13) | 13 | 3.5714% | 1.1905% | 0.2976% |

Formulas, for a given shape `i`: `P_normal(i)=1/28`; `P_heavy(i)=1/(3*28) + (2/3)*[i in big]/13`; `P_shifted(i)=P_heavy(i)/4 + (3/4)*[i in hard]/11`. These distributions sum to 1 per generator, counting all category members.

## Consequence for next bounded research hypothesis

A global fixed 28-shape placeability bonus, fixed future tray samples, or an uncalibrated uniform hard-shape proxy cannot faithfully represent this simulator's **three very different** future piece distributions, much less the real game's unknown RNG. This supports researching **conditional robust continuation value**: exact legal current 3-piece search + a small learned *future survival value* trained across the three known synthetic distributions using only isolated training seeds and their measurable continuation outcomes; select on worst-generator lower tail and current combo safety, never on the average score alone. At runtime the model may use only visible board/three pieces/combo, **not** which hidden future piece will be drawn. The next experiment must be an honestly new bounded training and simulation run with fresh audited cross-role epochs and all unchanged strict two-holdout 10×/p10/combo/legality/censoring gates. This analytic audit alone is not a gameplay improvement and must not be presented as one.

The score in this simulator remains a synthetic estimate, not the official game's points. Keep v1.7 baseline, saved checkpoint, main, and live v3.4 iPhone code untouched. Avoid overlapping ongoing research Actions jobs.

## Executed independent research-only smoke validation

The source-driven `learning/audit_shape_symmetry.py` and `learning/audit_synthetic_future_risk.py` were **actually executed**, not just proposed, on the research branch in [Actions smoke run 43809143081](https://github.com/welcome23bask/BlockBlastAI/actions/runs/43809143081), conclusion **success**. Logs show `SYNTHETIC_PRIOR_RISK_SELFTEST=PASS`, `SYMMETRY_AUGMENTATION_AUDIT=NOT_CLOSED`, and `SYNTHETIC_PRIORS_AND_SYMMETRY_SMOKE=PASS`, plus separately compiled v1.7/candidate legality selftests. Across the seven non-identity D4 transforms, the catalog omits between 2 and 13 of 28 rotated/reflected patterns; simple 8-way data augmentation would be invalid here. The same smoke confirms raw RNG alias **accept/reject** preflight tests. This workflow did **not** train a candidate, run gameplay holdouts or promote anything, and does not imply a 10× survival improvement.
