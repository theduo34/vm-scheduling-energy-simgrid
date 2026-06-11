#include "workload/workload.h"

#include <algorithm>
#include <random>
#include <stdexcept>

std::vector<VmRequest> generate_workload(const std::string& scenario)
{
    int count;
    if      (scenario == "light")    count = 20;
    else if (scenario == "moderate") count = 50;
    else if (scenario == "heavy")    count = 100;
    else
        throw std::invalid_argument(
            "Unknown scenario '" + scenario + "'. Use: light | moderate | heavy");

    std::mt19937 rng(WORKLOAD_SEED);

    // Distributions per CLAUDE.md spec
    std::uniform_int_distribution<int>     cpu_dist(1, 4);
    std::uniform_int_distribution<int>     ram_dist(1, 4);   // multiply by 256 MB
    std::uniform_real_distribution<double> life_dist(50.0, 300.0);
    // Arrivals spread uniformly across count*10 seconds
    // (light→200 s, moderate→500 s, heavy→1000 s)
    std::uniform_real_distribution<double> arr_dist(0.0, static_cast<double>(count) * 10.0);

    std::vector<VmRequest> requests;
    requests.reserve(count);

    for (int i = 0; i < count; ++i) {
        VmRequest req;
        req.id             = i + 1;  // temp; reassigned after sort
        req.cpu_cores      = cpu_dist(rng);
        req.ram_mb         = ram_dist(rng) * 256;
        req.lifetime_s     = life_dist(rng);
        req.arrival_time_s = arr_dist(rng);
        requests.push_back(req);
    }

    // Sort by arrival time; reassign IDs to reflect arrival order
    std::sort(requests.begin(), requests.end(),
              [](const VmRequest& a, const VmRequest& b) {
                  return a.arrival_time_s < b.arrival_time_s;
              });
    for (int i = 0; i < count; ++i)
        requests[i].id = i + 1;

    return requests;
}
