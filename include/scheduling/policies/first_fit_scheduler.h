#pragma once

#include "scheduling/scheduler.h"

// First Fit (greedy baseline).
//
// Iterates hosts in order from the start of the list. Places the VM on the
// FIRST host that has enough free cores+RAM. If no host fits, the request
// is rejected as an SLA violation.
class FirstFitScheduler : public SchedulerPolicy {
public:
    std::string name() const override { return "firstfit"; }

    int select_host(const VmRequest& req, const std::vector<HostState>& hosts) override;
};
