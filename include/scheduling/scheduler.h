#pragma once

#include <memory>
#include <string>
#include <vector>

#include "infrastructure/host_state.h"
#include "workload/workload.h"

// Common interface implemented by every VM placement policy
// (Round Robin, First Fit, Best Fit, Energy-Aware Best Fit).
//
// A policy only DECIDES which host (if any) should receive an incoming VM
// request; it must not mutate `hosts`. The simulation driver performs the
// actual placement bookkeeping once a decision has been made.
class SchedulerPolicy {
public:
    virtual ~SchedulerPolicy() = default;

    // Short, lowercase name used in logs and the results CSV
    // (e.g. "roundrobin", "firstfit", "bestfit", "energyaware").
    virtual std::string name() const = 0;

    // Returns the index into `hosts` of the host that should receive `req`,
    // or -1 if the policy could not find a suitable host. The caller then
    // records an SLA violation and moves on without retrying.
    virtual int select_host(const VmRequest& req, const std::vector<HostState>& hosts) = 0;
};

// Constructs a policy instance from its CLI name.
// Recognised names: "roundrobin" (more added in later stages).
// Throws std::invalid_argument for an unrecognised name.
std::unique_ptr<SchedulerPolicy> create_scheduler(const std::string& policy_name);
