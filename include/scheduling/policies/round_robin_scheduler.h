#pragma once

#include "scheduling/scheduler.h"

// Round Robin (baseline, no energy awareness).
//
// Maintains a counter that advances on every call, regardless of outcome.
// Each request is checked against exactly ONE host (the next one in
// rotation); if that host does not have enough free cores+RAM, the request
// is rejected as an SLA violation -- other hosts are never consulted.
class RoundRobinScheduler : public SchedulerPolicy {
public:
    std::string name() const override { return "roundrobin"; }

    int select_host(const VmRequest& req, const std::vector<HostState>& hosts) override;

private:
    size_t next_index_ = 0;
};
