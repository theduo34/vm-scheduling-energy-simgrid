#include "scheduling/policies/best_fit_scheduler.h"

int BestFitScheduler::select_host(const VmRequest& req, const std::vector<HostState>& hosts)
{
    int  best_idx             = -1;
    int  best_remaining_cores = 0;
    long best_remaining_ram   = 0;

    for (size_t idx = 0; idx < hosts.size(); ++idx) {
        const HostState& h = hosts[idx];
        if (h.free_cores < req.cpu_cores || h.free_ram_mb < req.ram_mb)
            continue; // doesn't fit, skip

        const int  remaining_cores = h.free_cores  - req.cpu_cores;
        const long remaining_ram   = h.free_ram_mb - req.ram_mb;

        // Tightest fit so far: fewer leftover cores, or (on a core tie)
        // less leftover RAM. The first host wins any remaining tie.
        if (best_idx == -1 ||
            remaining_cores < best_remaining_cores ||
            (remaining_cores == best_remaining_cores && remaining_ram < best_remaining_ram)) {
            best_idx             = static_cast<int>(idx);
            best_remaining_cores = remaining_cores;
            best_remaining_ram   = remaining_ram;
        }
    }

    return best_idx; // -1 if no host fits -> SLA violation
}
