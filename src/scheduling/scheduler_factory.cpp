#include "scheduling/scheduler.h"

#include <stdexcept>

#include "scheduling/policies/first_fit_scheduler.h"
#include "scheduling/policies/round_robin_scheduler.h"

std::unique_ptr<SchedulerPolicy> create_scheduler(const std::string& policy_name)
{
    if (policy_name == "roundrobin")
        return std::make_unique<RoundRobinScheduler>();
    if (policy_name == "firstfit")
        return std::make_unique<FirstFitScheduler>();

    throw std::invalid_argument(
        "Unknown policy '" + policy_name + "'. Use: roundrobin | firstfit");
}
