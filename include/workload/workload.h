#pragma once
#include <string>
#include <vector>

// Fixed seed used for all workload generation — every policy faces the same sequence.
static constexpr unsigned int WORKLOAD_SEED = 42;

struct VmRequest {
    int    id;             // 1-based, assigned in arrival-time order
    int    cpu_cores;      // 1–4
    int    ram_mb;         // 256 | 512 | 768 | 1024
    double lifetime_s;     // 50.0–300.0
    double arrival_time_s; // seconds from simulation start; vector is sorted ascending
};

// Returns a deterministic list of VM requests for the given scenario.
// scenario: "light" (20 VMs) | "moderate" (50 VMs) | "heavy" (100 VMs)
// Throws std::invalid_argument for an unrecognised scenario string.
std::vector<VmRequest> generate_workload(const std::string& scenario);
