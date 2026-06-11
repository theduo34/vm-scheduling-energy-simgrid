#include "simulation/simulation.h"

#include <simgrid/plugins/energy.h>

XBT_LOG_NEW_DEFAULT_CATEGORY(vm_sim, "VM placement and lifecycle events");

namespace {

// Runs on a freshly-spawned actor: occupies `req.cpu_cores` cores on the
// host at hosts[host_idx] for `req.lifetime_s` seconds (driving the Energy
// Plugin's load model), then releases the host's resources.
//
// One core's worth of computation is modelled as `host_speed * lifetime_s`
// flops, so a single execution finishes in exactly `lifetime_s` seconds when
// it has a full core to itself; spawning `cpu_cores` such executions in
// parallel makes the host's CPU load (and therefore its power draw) reflect
// the fraction of cores the VM occupies.
void run_vm_lifecycle(std::vector<HostState>& hosts, int host_idx, const VmRequest& req)
{
    HostState& h = hosts[host_idx];

    const double per_core_speed = h.host->get_speed(); // flop/s for ONE core
    const double flops = per_core_speed * req.lifetime_s;

    std::vector<simgrid::s4u::ExecPtr> execs;
    execs.reserve(req.cpu_cores);
    for (int c = 0; c < req.cpu_cores; ++c)
        execs.push_back(h.host->exec_async(flops));

    for (auto& e : execs)
        e->wait();

    h.apply_usage_change(simgrid::s4u::Engine::get_clock(), req.cpu_cores, req.ram_mb);
}

} // namespace

SimulationMetrics run_simulation(simgrid::s4u::Engine& engine,
                                  std::vector<HostState>& hosts,
                                  SchedulerPolicy& scheduler,
                                  const std::vector<VmRequest>& requests,
                                  const std::string& workload_name)
{
    int sla_violations = 0;

    simgrid::s4u::Actor::create("dispatcher", hosts.front().host, [&]() {
        for (const auto& req : requests) {
            const double now = simgrid::s4u::Engine::get_clock();
            if (req.arrival_time_s > now)
                simgrid::s4u::this_actor::sleep_for(req.arrival_time_s - now);

            const int idx = scheduler.select_host(req, hosts);
            if (idx < 0) {
                ++sla_violations;
                XBT_INFO("[t=%7.2f] VM %3d REJECTED  (SLA violation): %d core(s), %4d MB",
                         simgrid::s4u::Engine::get_clock(), req.id, req.cpu_cores, req.ram_mb);
                continue;
            }

            HostState& h = hosts[idx];
            h.apply_usage_change(simgrid::s4u::Engine::get_clock(), -req.cpu_cores, -req.ram_mb);
            h.ever_used = true;

            XBT_INFO("[t=%7.2f] VM %3d -> %-8s (%d core(s), %4d MB, %.1fs lifetime)",
                     simgrid::s4u::Engine::get_clock(), req.id, h.host->get_cname(),
                     req.cpu_cores, req.ram_mb, req.lifetime_s);

            simgrid::s4u::Actor::create("vm" + std::to_string(req.id), h.host,
                                         [&hosts, idx, req]() { run_vm_lifecycle(hosts, idx, req); });
        }
    });

    engine.run();

    SimulationMetrics m;
    m.policy   = scheduler.name();
    m.workload = workload_name;

    // Metric 1: total energy (J), summed over ALL hosts (active or idle).
    double total_energy = 0.0;
    for (const auto& h : hosts)
        total_energy += sg_host_get_consumed_energy(h.host);
    m.energy_joules = total_energy;

    const double total_time = simgrid::s4u::Engine::get_clock();

    // Metrics 2 & 4: hosts that ever received a VM, and their mean
    // time-averaged core utilization over the whole run.
    int active_count = 0;
    double util_sum = 0.0;
    for (const auto& h : hosts) {
        if (!h.ever_used)
            continue;
        ++active_count;
        if (total_time > 0.0)
            util_sum += h.busy_core_seconds / (h.total_cores * total_time);
    }
    m.active_hosts = active_count;
    m.avg_utilization = active_count > 0 ? (util_sum / active_count) * 100.0 : 0.0;

    // Metric 3: SLA violation rate (%).
    m.sla_violation_rate = requests.empty()
        ? 0.0
        : (static_cast<double>(sla_violations) / requests.size()) * 100.0;

    return m;
}
