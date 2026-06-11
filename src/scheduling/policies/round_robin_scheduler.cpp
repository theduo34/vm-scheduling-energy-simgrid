#include "scheduling/policies/round_robin_scheduler.h"

int RoundRobinScheduler::select_host(const VmRequest& req, const std::vector<HostState>& hosts)
{
    const size_t idx = next_index_;
    next_index_ = (next_index_ + 1) % hosts.size();

    const HostState& h = hosts[idx];
    if (h.free_cores >= req.cpu_cores && h.free_ram_mb >= req.ram_mb)
        return static_cast<int>(idx);

    return -1; // single host didn't fit -> SLA violation, do not search further
}
