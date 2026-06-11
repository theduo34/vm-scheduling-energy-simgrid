#include "infrastructure/host_state.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>

double HostState::power_at(int cores_in_use) const
{
    const double u = total_cores > 0 ? static_cast<double>(cores_in_use) / total_cores : 0.0;
    return idle_watts + (peak_watts - idle_watts) * u;
}

void HostState::apply_usage_change(double now, int delta_cores, long delta_ram_mb)
{
    const int cores_in_use = total_cores - free_cores;
    busy_core_seconds += (now - last_update_time) * cores_in_use;
    last_update_time = now;

    free_cores  += delta_cores;
    free_ram_mb += delta_ram_mb;
}

namespace {

// Parses the "idle:peak" format of the wattage_per_state property
// (e.g. "120.0:250.0" -> idle_w=120.0, peak_w=250.0).
void parse_wattage(const char* prop, double& idle_w, double& peak_w)
{
    idle_w = 0.0;
    peak_w = 0.0;
    if (prop != nullptr)
        std::sscanf(prop, "%lf:%lf", &idle_w, &peak_w);
}

} // namespace

std::vector<HostState> build_host_states(simgrid::s4u::Engine& engine)
{
    std::vector<simgrid::s4u::Host*> all_hosts = engine.get_all_hosts();

    std::sort(all_hosts.begin(), all_hosts.end(),
              [](const simgrid::s4u::Host* a, const simgrid::s4u::Host* b) {
                  return a->get_name() < b->get_name();
              });

    std::vector<HostState> states;
    states.reserve(all_hosts.size());

    for (auto* host : all_hosts) {
        HostState hs;
        hs.host        = host;
        hs.total_cores = host->get_core_count();
        hs.free_cores  = hs.total_cores;

        const char* ram_prop = host->get_property("ram_mb");
        hs.total_ram_mb = ram_prop != nullptr ? std::strtol(ram_prop, nullptr, 10) : 0;
        hs.free_ram_mb  = hs.total_ram_mb;

        parse_wattage(host->get_property("wattage_per_state"), hs.idle_watts, hs.peak_watts);

        states.push_back(hs);
    }

    return states;
}
