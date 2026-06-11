# Experimental Findings & Design Decisions

This document records design decisions made during the experimental phase of the
project, along with the reasoning behind them. It is intended as source material for
Chapter Four (Results & Discussion) and as a reference for viva questions about why the
experimental setup looks the way it does.

---

## 2026-06-11 — Revising host power profiles so capacity and efficiency diverge

### 1. Finding so far

With all four scheduling policies implemented and run against the original host power
profiles:

- Type A (High Capacity): 8 cores @ 2.4 GHz, idle 120 W, peak 250 W
- Type B (Medium Capacity): 4 cores @ 2.0 GHz, idle 80 W, peak 170 W
- Type C (Low Capacity): 2 cores @ 1.6 GHz, idle 50 W, peak 110 W

the Energy-Aware Best Fit policy produced results that were nearly identical to First
Fit (identical for the light and moderate workloads, and within ~0.1% for the heavy
workload, with the same number of active hosts in all three cases).

The reason is structural, not a bug in either policy. Under the linear power model
P(u) = P_idle + (P_peak - P_idle) * u, the marginal power cost of placing a VM on a host
depends only on (P_peak - P_idle) / total_cores for that host's type, not on its current
load. For the original profiles this "watts per core" figure works out to:

- Type A: (250 - 120) / 8 = 16.25 W per core
- Type B: (170 - 80) / 4 = 22.5 W per core
- Type C: (110 - 50) / 2 = 30.0 W per core

Type A is therefore simultaneously:

- the **highest-capacity** host type (most cores and RAM per host),
- the **most energy-efficient per core** (lowest marginal watts per core), and
- **first in declaration order** in the platform XML (and therefore the first host
  considered by Round Robin and First Fit).

Because all three of these properties point the same way, every policy that tries to
"fill a host before moving to the next" — First Fit by iteration order, and
Energy-Aware Best Fit by lowest projected power increase — converges on the same
behaviour: fill the Type A hosts first, in the same order, before touching Type B or C.
The two policies end up making the same placement decisions for the same reason, via
different reasoning paths.

The practical consequence is that this scenario cannot distinguish energy-aware
placement from naive placement. Energy-Aware Best Fit has nothing to be "aware" of: the
energy-optimal choice and the position-optimal choice are the same host, every time.

### 2. Design decision

The host power profiles will be revised so that **capacity and energy-efficiency do not
align**. Concretely, the largest/highest-capacity host type will no longer also be the
most efficient per watt. This mirrors real data centre hardware: large enterprise-class
servers are often provisioned for peak throughput and carry a high idle power draw
(features like redundant PSUs, more memory channels, management controllers, etc.,
all consume power even when the server is lightly loaded), whereas mid-range servers
are frequently the "performance-per-watt sweet spot" that hardware vendors and data
centre operators specifically target for energy efficiency.

The exact revised wattage figures are still to be finalised, but the shape of the
change is: increase the idle-power cost (and/or reduce the peak-to-idle efficiency
ratio) of the high-capacity host type relative to the medium-capacity type, so that a
host with more cores is not automatically the cheapest place, per core, to run a VM.

### 3. Justification (viva defense)

This change is **tuning the experimental scenario to be discriminating, not tuning (or
rigging) the policies themselves**. The four scheduling policies — Round Robin, First
Fit, Best Fit, and Energy-Aware Best Fit — remain exactly as specified, with no special
casing for any host type. What changes is the *environment* the policies are tested
against.

The argument for why this is methodologically sound:

- If the most powerful host is also the most power-efficient *and* is the one every
  policy reaches first, there is no scheduling decision left to make — every reasonable
  policy lands on the same answer, and the experiment is measuring "does the policy fill
  the obvious host first?" rather than "does the policy make good energy trade-offs?"
- An experiment that cannot, even in principle, produce different outcomes for
  different policies is not testing the phenomenon (energy-aware scheduling) that the
  project is about. A non-discriminating scenario would understate the contribution of
  the Energy-Aware Best Fit policy not because it doesn't work, but because the test
  never required it to do anything different.
- Making capacity and efficiency diverge is not an artificial assumption invented to
  flatter the energy-aware policy — it is grounded in well-documented real-world server
  behaviour (idle power overhead scales with a server's provisioned capacity and
  redundancy features, not linearly with useful work done). The revised profiles will
  still use the same linear power model and the same SimGrid Energy Plugin mechanism;
  only the idle/peak wattage constants change.
- With divergent capacity and efficiency, First Fit (which only looks at position) and
  Best Fit (which only looks at remaining capacity) will sometimes place VMs on the
  high-capacity-but-less-efficient host type, while Energy-Aware Best Fit will route
  those VMs toward the more efficient type when it can still satisfy the request. The
  resulting differences in total energy, active host count, and utilization become
  attributable to the *scheduling criterion*, which is the variable under study.

### 4. Required follow-up

- The platform XML (`platform/datacenter.xml`) and the corresponding `wattage_per_state`
  properties need to be updated to the revised idle/peak figures.
- **All numeric results generated so far (the 12-row `results/simulation_results.csv`)
  will become invalid** once the power profiles change, since total energy and the
  Energy-Aware Best Fit placement decisions both depend directly on these figures. All
  12 runs (4 policies x 3 workloads) must be re-executed against the revised platform
  before any results are written up.
- Chapter Three's host specification table (which currently lists the original idle
  120/250, 80/170, 50/110 W figures for Types A/B/C) must be updated to match the final
  revised profiles, with a note explaining the rationale documented above so the
  specification and the implementation stay consistent.

---

## 2026-06-11 — PAUSED — open problem for next session

### Current state

The codebase has been reverted to the Stage 6 committed baseline, commit `6c5c993`
("Stage 6 all four policies build and run correctly"). The original power profiles are
in place:

- Type A: idle 120 W, peak 250 W
- Type B: idle 80 W, peak 170 W
- Type C: idle 50 W, peak 110 W

All four policies build and run. SLA violation rate is 0% for First Fit, Best Fit, and
Energy-Aware Best Fit (Round Robin retains its expected double-digit SLA violation
rate, by design, as the non-searching baseline).

Energy-Aware Best Fit currently produces results **identical** to First Fit (5/8/12
active hosts across the light/moderate/heavy workloads), because under the linear power
model the energy-optimal host coincides with the first-fit host (see the section above
for the full explanation).

### The open problem

Make Energy-Aware Best Fit a genuinely distinct and defensible policy that meaningfully
reflects the consolidation / idle-power principle from the literature (Beloglazov &
Buyya), **without resorting to result-driven parameter tuning** (i.e. without picking
power numbers or rules whose only justification is "this makes the chart look like we
want").

### What we learned today (so we don't repeat it)

- The original energy-aware logic selects by marginal core-power, `(peak - idle) /
  cores`, which ignores idle power entirely — yet idle power is the dominant source of
  energy waste in the thesis argument. A policy built only on this metric is not
  "energy-aware" in the sense the thesis means, even though it is internally consistent
  and produces 0% SLA violations.
- Simply changing the power profiles (so that capacity and per-core efficiency diverge)
  did **not** make Energy-Aware Best Fit diverge from First Fit, because the *marginal*
  ordering A < B < C held under the new numbers too — only the gap between A and B
  narrowed. The marginal-core-power metric is not sensitive enough to profile changes of
  this kind.
- Redefining the policy to minimize *total system power* (full idle + marginal cost when
  activating a new host, marginal-only when reusing an active host) **did** make it
  diverge from First Fit — but it ended up using *more* active hosts and *more* total
  energy than First Fit, not fewer. The reason: this formulation avoided the high-idle
  Type A hosts entirely (their idle draw dominated the comparison for every VM size),
  pushing all load onto the smaller-capacity Type B/C hosts, which then exhausted their
  capacity faster and forced more total host activations — a heterogeneous bin-packing
  effect, the same family of issue documented for Best Fit in the section above.
- The combined lesson: on heterogeneous hardware, "energy-aware placement" is a genuine
  multi-objective trade-off between per-host efficiency, idle-power amortization, and
  capacity/bin-packing effects. It is not reducible to a one-line comparison of a single
  derived quantity per host.

### Options to explore next session

1. **Report the honest trade-off.** Present Energy-Aware Best Fit's current
   (marginal-core-power) behaviour as one valid energy criterion among several,
   acknowledge that no single policy dominates First Fit, Best Fit, and Energy-Aware
   Best Fit on every metric simultaneously, and discuss *why* in Chapter Four. This is
   the most defensible option and may produce the strongest discussion section, since it
   demonstrates genuine understanding of the trade-offs rather than a policy engineered
   to "win".
2. **A principled multi-factor formulation.** Design an energy-aware criterion that
   explicitly balances (a) consolidation onto already-active hosts, (b) host idle-power
   efficiency, and (c) remaining capacity / bin-packing fit, with each factor's role
   grounded in a citation from the literature rather than chosen to fit a target
   outcome. This is more work and carries more risk of looking "tuned" if not framed
   carefully, but could produce a policy that is both genuinely distinct and
   defensible.

No code has been changed in this session beyond the revert back to `6c5c993`.
