#include <simgrid/s4u.hpp>
#include <simgrid/plugins/energy.h>

#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "infrastructure/host_state.h"
#include "metrics/metrics.h"
#include "scheduling/scheduler.h"
#include "simulation/simulation.h"
#include "workload/workload.h"

XBT_LOG_NEW_DEFAULT_CATEGORY(vm_scheduling, "VM Scheduling Energy Simulation");

int main(int argc, char* argv[])
{
    if (argc < 4) {
        std::fprintf(stderr, "Usage: %s <platform.xml> <policy> <scenario>\n", argv[0]);
        std::fprintf(stderr, "  policy   : roundrobin\n");
        std::fprintf(stderr, "  scenario : light | moderate | heavy\n");
        std::fprintf(stderr, "  e.g.  ./build/simulation platform/datacenter.xml roundrobin heavy\n");
        return 1;
    }

    // Energy plugin must be initialised before the platform is loaded
    sg_host_energy_plugin_init();

    simgrid::s4u::Engine engine(&argc, argv);
    engine.load_platform(argv[1]);

    const std::string policy_name = argv[2];
    const std::string scenario    = argv[3];

    std::unique_ptr<SchedulerPolicy> scheduler;
    std::vector<VmRequest> requests;
    try {
        scheduler = create_scheduler(policy_name);
        requests  = generate_workload(scenario);
    } catch (const std::invalid_argument& e) {
        std::fprintf(stderr, "Error: %s\n", e.what());
        return 1;
    }

    std::vector<HostState> hosts = build_host_states(engine);

    XBT_INFO("=== %zu hosts | policy=%s | workload=%s (%zu requests, seed=%u) ===",
             hosts.size(), scheduler->name().c_str(), scenario.c_str(),
             requests.size(), WORKLOAD_SEED);

    SimulationMetrics metrics = run_simulation(engine, hosts, *scheduler, requests, scenario);

    XBT_INFO("=== Results: %s / %s ===", metrics.policy.c_str(), metrics.workload.c_str());
    XBT_INFO("Total energy        : %.2f J", metrics.energy_joules);
    XBT_INFO("Active hosts        : %d / %zu", metrics.active_hosts, hosts.size());
    XBT_INFO("SLA violation rate  : %.2f %%", metrics.sla_violation_rate);
    XBT_INFO("Avg host utilization: %.2f %%", metrics.avg_utilization);

    append_metrics_csv(metrics, "results/simulation_results.csv");
    XBT_INFO("Appended results row to results/simulation_results.csv");

    return 0;
}
