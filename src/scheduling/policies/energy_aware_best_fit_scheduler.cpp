#include "scheduling/policies/energy_aware_best_fit_scheduler.h"

#include <cmath>

namespace {

// Tolerance for comparing projected power deltas. The linear power model
// makes delta_P depend only on host type and cpu_cores requested, so equal
// values are normally bit-identical -- the epsilon just guards against any
// floating-point rounding noise.
constexpr double kPowerEpsilon = 1e-9;

bool fits(const HostState& h, const VmRequest& req)
{
    return h.free_cores >= req.cpu_cores && h.free_ram_mb >= req.ram_mb;
}

// Projected power INCREASE (W) of placing `req` on host `h`:
//   delta_P = power with the VM added - current power
// using the linear model P(u) = idle + (peak - idle) * (cores_in_use / total_cores).
double projected_power_increase(const HostState& h, const VmRequest& req)
{
    const int cores_in_use = h.total_cores - h.free_cores;
    return h.power_at(cores_in_use + req.cpu_cores) - h.power_at(cores_in_use);
}

} // namespace

int EnergyAwareBestFitScheduler::select_host(const VmRequest& req, const std::vector<HostState>& hosts)
{
    // --- Pass 1: prefer hosts already activated (ever_used) ---
    int    best_idx             = -1;
    double best_delta           = 0.0;
    int    best_remaining_cores = 0;
    long   best_remaining_ram   = 0;

    for (size_t idx = 0; idx < hosts.size(); ++idx) {
        const HostState& h = hosts[idx];
        if (!h.ever_used || !fits(h, req))
            continue;

        const double delta           = projected_power_increase(h, req);
        const int    remaining_cores = h.free_cores  - req.cpu_cores;
        const long   remaining_ram   = h.free_ram_mb - req.ram_mb;

        const bool is_new_best =
            best_idx == -1 ||
            delta < best_delta - kPowerEpsilon ||
            (std::abs(delta - best_delta) <= kPowerEpsilon &&
             (remaining_cores < best_remaining_cores ||
              (remaining_cores == best_remaining_cores && remaining_ram < best_remaining_ram)));

        if (is_new_best) {
            best_idx             = static_cast<int>(idx);
            best_delta           = delta;
            best_remaining_cores = remaining_cores;
            best_remaining_ram   = remaining_ram;
        }
    }

    if (best_idx != -1)
        return best_idx;

    // --- Pass 2: no already-active host fits -- activate the cheapest
    // not-yet-used host that does ---
    double best_new_delta = 0.0;

    for (size_t idx = 0; idx < hosts.size(); ++idx) {
        const HostState& h = hosts[idx];
        if (h.ever_used || !fits(h, req))
            continue;

        const double delta = projected_power_increase(h, req);
        if (best_idx == -1 || delta < best_new_delta) {
            best_idx       = static_cast<int>(idx);
            best_new_delta = delta;
        }
    }

    return best_idx; // -1 if nothing fits at all -> SLA violation
}
