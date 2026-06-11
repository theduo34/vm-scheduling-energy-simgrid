#pragma once

#include "scheduling/scheduler.h"

// Best Fit (consolidation-aware).
//
// Considers every host that has enough free cores+RAM for the request and
// selects the one that would be left with the LEAST remaining capacity
// afterwards (the "tightest fit"), so gaps too small for future VMs are
// minimised and consolidation is maximised.
//
// "Remaining capacity" is measured primarily in free CPU cores after
// placement, since cores are the dimension the platform spec and the power
// model are built around. Free RAM after placement is used as a
// tie-breaker, and host index (declaration order) as a final deterministic
// tie-breaker.
//
// If no host fits, the request is rejected as an SLA violation.
//
// NOTE (viva discussion point): on THIS heterogeneous platform (Type A/B/C
// hosts have 8/4/2 cores), "fewest leftover cores" systematically favours
// smaller host types -- a 1-core VM leaves 1 leftover core on a Type C host
// vs. 7 on a Type A host, so Type C "wins" even though Type A has plenty of
// spare capacity. The result is that Best Fit packs each host it uses more
// tightly (higher avg. utilisation) than First Fit, but spreads across MORE
// distinct hosts overall, since small hosts reach a "perfect fit" sooner.
// This is a well-known property of Best Fit bin-packing on heterogeneous
// bin sizes, not a bug -- and it's part of the motivation for the
// Energy-Aware Best Fit policy, which selects by projected POWER increase
// rather than raw leftover capacity.
class BestFitScheduler : public SchedulerPolicy {
public:
    std::string name() const override { return "bestfit"; }

    int select_host(const VmRequest& req, const std::vector<HostState>& hosts) override;
};
