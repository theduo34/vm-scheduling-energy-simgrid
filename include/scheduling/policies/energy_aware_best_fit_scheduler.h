#pragma once

#include "scheduling/scheduler.h"

// Energy-Aware Best Fit (PRIMARY policy).
//
// For every candidate host that can fit the request, the PROJECTED POWER
// INCREASE is computed from the linear power model:
//
//     P(u) = P_idle + (P_peak - P_idle) * u     where u = cores_in_use / total_cores
//
//     delta_P = P(u_after_placement) - P(u_before_placement)
//             = power with the VM added  -  current power
//
// Selection proceeds in two passes:
//
//   Pass 1 - hosts already ACTIVATED (ever_used == true, i.e. they have
//   received >= 1 VM during this run, even if momentarily empty right now)
//   are tried first. The host with the SMALLEST delta_P wins; ties (which
//   the linear model produces for same-type hosts, since delta_P depends
//   only on host type and cpu_cores requested) are broken Best-Fit style:
//   fewest leftover cores after placement, then fewest leftover RAM, then
//   host index. This keeps consolidating work onto the cheapest hosts that
//   are already running.
//
//   Pass 2 - if NO already-active host fits, the host with the SMALLEST
//   delta_P among the NOT-YET-ACTIVATED hosts that fit is activated (ties
//   broken by host index). A new host is only brought online when strictly
//   necessary.
//
// If nothing fits at all (neither pass finds a host), the request is
// rejected as an SLA violation.
class EnergyAwareBestFitScheduler : public SchedulerPolicy {
public:
    std::string name() const override { return "energyaware"; }

    int select_host(const VmRequest& req, const std::vector<HostState>& hosts) override;
};
