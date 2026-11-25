#!/usr/bin/env bash
# Fixture-only compile/link and inference smoke. Does NOT train or certify policy.
set -euo pipefail
cd "$(dirname "$0")/.."
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
cat > "$tmp/hazard_policy_model.generated.h" <<'EOF'
static const double bl_hazard_weights[10] = {-2,4,0,0,0,0,0,0,0,0};
static const double bl_hazard_scales[9] = {64,36,32,32,11,64,20,10,32};
EOF
cat > "$tmp/probe.c" <<'EOF'
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
double BLResearchHazardProbability(uint64_t board,int streak);
int BLResearchHazardPenalty(uint64_t board,int streak);
int main(void) {
    const double low=BLResearchHazardProbability(0,0);
    const double high=BLResearchHazardProbability(UINT64_MAX,0);
    assert(fabs(low-1.0/(1.0+exp(2.0)))<1e-12);
    assert(fabs(high-1.0/(1.0+exp(-2.0)))<1e-12);
    assert(BLResearchHazardPenalty(UINT64_MAX,0)>BLResearchHazardPenalty(0,0));
    assert(low==BLResearchHazardProbability(0,0));
    puts("HAZARD_BRIDGE_INFERENCE_SMOKE=PASS fixture only");
}
EOF
cc -O2 -std=c11 -Isrc -DBLFindTwoStrategies=BLV17FindTwoStrategies -c src/BLDualStrategy.c -o "$tmp/v17.o"
cc -O2 -std=c11 -Isrc -c src/BlockBlastSolver.c -o "$tmp/solver.o"
cc -O2 -std=c11 -Isrc -I"$tmp" -c learning/hazard_policy_bridge.c -o "$tmp/bridge.o"
cc -O2 -std=c11 -Isrc -DBL_RESEARCH_LEARNED_HAZARD=1 -c learning/BLTrainableStrategy.c -o "$tmp/trained.o"
cc -O2 -std=c11 -Isrc "$tmp/probe.c" "$tmp/v17.o" "$tmp/solver.o" "$tmp/bridge.o" -lm -o "$tmp/probe"
"$tmp/probe"
cc -O2 -std=c11 -Isrc learning/tenx_rules_selftest.c "$tmp/v17.o" "$tmp/trained.o" "$tmp/solver.o" "$tmp/bridge.o" -lm -o "$tmp/rules"
"$tmp/rules"
echo "HAZARD_POLICY_FIXTURE_LINK_AND_RULES=PASS (no training or official scores)"
