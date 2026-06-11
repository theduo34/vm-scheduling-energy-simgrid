#include "scheduling/policies/first_fit_scheduler.h"

int FirstFitScheduler::select_host(const VmRequest& req, const std::vector<HostState>& hosts)
{
    for (size_t idx = 0; idx < hosts.size(); ++idx) {
        const HostState& h = hosts[idx];
        if (h.free_cores >= req.cpu_cores && h.free_ram_mb >= req.ram_mb)
            return static_cast<int>(idx);
    }

    return -1; // no host fits -> SLA violation
}
