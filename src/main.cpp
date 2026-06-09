#include <simgrid/s4u.hpp>
#include <simgrid/plugins/energy.h>

XBT_LOG_NEW_DEFAULT_CATEGORY(vm_scheduling, "VM Scheduling Energy Simulation");

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::fprintf(stderr, "Usage: %s <platform.xml>\n", argv[0]);
        std::fprintf(stderr, "  e.g. ./build/simulation platform/datacenter.xml\n");
        return 1;
    }

    // Energy plugin must be initialised before the platform is loaded
    sg_host_energy_plugin_init();

    simgrid::s4u::Engine engine(&argc, argv);
    engine.load_platform(argv[1]);

    XBT_INFO("=== Platform loaded: %zu hosts ===", engine.get_host_count());
    XBT_INFO("%-10s  %5s  %11s  %s",
             "Host", "Cores", "Speed(GHz)", "wattage_per_state (idle:peak W)");
    XBT_INFO("%-10s  %5s  %11s  %s",
             "----------", "-----", "-----------", "------------------------------");

    for (auto* host : engine.get_all_hosts()) {
        const char* wattage = host->get_property("wattage_per_state");
        XBT_INFO("%-10s  %5d  %11.1f  %s",
                 host->get_cname(),
                 static_cast<int>(host->get_core_count()),
                 host->get_speed() / 1e9,
                 wattage ? wattage : "N/A");
    }

    return 0;
}
