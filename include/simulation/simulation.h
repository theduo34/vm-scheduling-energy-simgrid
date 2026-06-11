#pragma once

#include <simgrid/s4u.hpp>
#include <string>
#include <vector>

#include "infrastructure/host_state.h"
#include "metrics/metrics.h"
#include "scheduling/scheduler.h"
#include "workload/workload.h"

// Drives the full event-driven simulation for one (policy, workload) run.
//
// Spawns a dispatcher actor that releases each request from `requests` at
// its arrival time, asks `scheduler` to pick a host, and -- on success --
// spawns a short-lived "VM actor" that occupies the chosen host's cores for
// the request's lifetime (driving the Energy Plugin's load model) before
// releasing its resources. Requests the policy could not place are counted
// as SLA violations.
//
// Must be called after engine.load_platform() and before any call to
// engine.run(); this function calls engine.run() itself and returns the
// collected metrics once the simulation has finished.
SimulationMetrics run_simulation(simgrid::s4u::Engine& engine,
                                  std::vector<HostState>& hosts,
                                  SchedulerPolicy& scheduler,
                                  const std::vector<VmRequest>& requests,
                                  const std::string& workload_name);
