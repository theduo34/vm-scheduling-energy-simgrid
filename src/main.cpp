#include <simgrid/s4u.hpp>

XBT_LOG_NEW_DEFAULT_CATEGORY(vm_scheduling, "VM Scheduling Energy Simulation");

int main(int argc, char* argv[])
{
    simgrid::s4u::Engine engine(&argc, argv);

    XBT_INFO("SimGrid environment is working.");
    XBT_INFO("Project: Energy-Efficient VM Scheduling in Cloud Data Centers");
    XBT_INFO("SimGrid version check successful. Ready to build the simulation.");

    return 0;
}
