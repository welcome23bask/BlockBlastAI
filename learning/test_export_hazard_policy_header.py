import copy
import unittest
from export_hazard_policy_header import render, NAMES, SCALES, SCENARIOS

class HazardPolicyExportTests(unittest.TestCase):
    def model(self):
        return dict(status="research_only_validation_diagnostic_pass",
                    all_generator_brier_not_worse_than_constant=True,
                    train_epochs=[100], validation_epochs=[101],
                    feature_names=NAMES[:], feature_scales=SCALES[:],
                    weights=[0.0]*10,
                    per_epoch_validation={s:{"101":dict(brier=.1,
                        constant_prevalence_brier=.2,passes_unrounded=True)}
                        for s in SCENARIOS})
    def test_export(self):
        self.assertIn("bl_hazard_weights[10]", render(self.model(),100,101))
    def test_reject_bad_models(self):
        for key, value in (("train_epochs",[99]),("validation_epochs",[100]),
                           ("status","fail"),("feature_scales",[]),
                           ("feature_names",NAMES[::-1]),("weights",[float("nan")]*10),
                           ("all_generator_brier_not_worse_than_constant",False)):
            model=self.model()
            model[key]=value
            with self.assertRaises(ValueError): render(model,100,101)
    def test_one_generator_fails(self):
        model=self.model()
        model["per_epoch_validation"]["heavy"]["101"]["passes_unrounded"]=False
        with self.assertRaises(ValueError): render(model,100,101)
    def test_one_generator_missing(self):
        model=self.model()
        del model["per_epoch_validation"]["shifted-heavy"]
        with self.assertRaises(ValueError): render(model,100,101)
    def test_disjoint_epochs(self):
        with self.assertRaises(ValueError): render(self.model(),100,100)

if __name__=="__main__":
    unittest.main()
