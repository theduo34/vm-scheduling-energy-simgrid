#pragma once

#include <string>

// One row of the results CSV (results/simulation_results.csv), matching the
// header: policy,workload,energy_joules,active_hosts,sla_violation_rate,avg_utilization
struct SimulationMetrics {
    std::string policy;
    std::string workload;
    double energy_joules      = 0.0;
    int    active_hosts       = 0;
    double sla_violation_rate = 0.0; // percent
    double avg_utilization    = 0.0; // percent
};

// Appends `m` as one row to `csv_path`, writing the header first if the
// file does not already exist.
void append_metrics_csv(const SimulationMetrics& m, const std::string& csv_path);
