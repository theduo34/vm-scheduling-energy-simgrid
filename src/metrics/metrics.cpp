#include "metrics/metrics.h"

#include <fstream>
#include <iomanip>
#include <sys/stat.h>

namespace {

bool file_exists(const std::string& path)
{
    struct stat buffer{};
    return stat(path.c_str(), &buffer) == 0;
}

} // namespace

void append_metrics_csv(const SimulationMetrics& m, const std::string& csv_path)
{
    const bool needs_header = !file_exists(csv_path);

    std::ofstream out(csv_path, std::ios::app);
    out << std::fixed << std::setprecision(4);

    if (needs_header)
        out << "policy,workload,energy_joules,active_hosts,sla_violation_rate,avg_utilization\n";

    out << m.policy << ','
        << m.workload << ','
        << m.energy_joules << ','
        << m.active_hosts << ','
        << m.sla_violation_rate << ','
        << m.avg_utilization << '\n';
}
