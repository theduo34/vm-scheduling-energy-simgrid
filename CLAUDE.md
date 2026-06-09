# Project: Energy-Efficient VM Scheduling in Cloud Data Centers using SimGrid

## Overview
This is a final-year BTech Computer Science project. It is a SIMULATION STUDY that
implements and compares four Virtual Machine (VM) scheduling policies in a simulated
cloud data center, measuring their energy efficiency and service quality. There is NO
real infrastructure. Everything runs in SimGrid 3.35 using the S4U API and the Energy Plugin.

## Goal
Implement four scheduling policies, run each against three workload intensities (12 runs
total), and output metrics to CSV for later Python analysis. Determine which policy gives
the best energy efficiency without unacceptable service degradation.

## Environment (already set up - do not change)
- OS: Ubuntu 22.04 on WSL2
- SimGrid: version 3.35, installed at /usr/local (headers at /usr/local/include/simgrid)
- Compiler: g++ 11.4, C++17 standard
- Build: CMake (FindSimGrid.cmake is in ./cmake/)
- The project already compiles. src/main.cpp currently prints a SimGrid test message.

## Project Structure
- src/        -> C++ source files
- include/    -> C++ header files
- platform/   -> SimGrid XML platform files (host + network definitions)
- workloads/  -> workload configuration if needed
- results/    -> CSV output files (one row per run)
- analysis/   -> Python scripts (pandas/matplotlib) for charts - LATER, not now
- cmake/      -> contains FindSimGrid.cmake

## Data Center Specification (MUST MATCH EXACTLY)
20 physical hosts in THREE types:

Type A - High Capacity: 6 hosts
  - 8 cores @ 2.4 GHz (speed: 2.4Gf per core)
  - 8 GB RAM
  - Idle power: 120 W, Peak power: 250 W

Type B - Medium Capacity: 8 hosts
  - 4 cores @ 2.0 GHz (speed: 2.0Gf per core)
  - 4 GB RAM
  - Idle power: 80 W, Peak power: 170 W

Type C - Low Capacity: 6 hosts
  - 2 cores @ 1.6 GHz (speed: 1.6Gf per core)
  - 2 GB RAM
  - Idle power: 50 W, Peak power: 110 W

## Power Model (linear)
Power at utilization u (0 to 1):  P(u) = P_idle + (P_peak - P_idle) * u
This is the standard SimGrid Energy Plugin model. Use the host XML "wattage_per_state"
property with idle and peak values. Enable the Energy Plugin via
sg_host_energy_plugin_init() before loading the platform.

## Network
Simple flat topology: all 20 hosts connected via a backbone.
Bandwidth 1 Gbps, latency 10ms. Network is NOT under study - keep it simple and uniform.

## The Four Scheduling Policies
All policies share the same structure: receive a VM request, read host states, select a
host using the policy's criterion, place the VM if a host fits, else record an SLA violation.
Only the SELECTION CRITERION differs.

1. Round Robin (baseline, no energy awareness)
   - Maintain a counter. Assign each VM to the next host in rotation.
   - Only check if that one host has enough free cores+RAM. If not, record SLA violation
     and move on (do NOT search other hosts).

2. First Fit (greedy baseline)
   - Iterate hosts from the start. Place VM on the FIRST host with enough free cores+RAM.
   - If no host fits, record SLA violation.

3. Best Fit (consolidation-aware)
   - Iterate ALL active hosts. Select the one with the LEAST remaining capacity that can
     still fit the VM (tightest fit).
   - If no host fits, record SLA violation.

4. Energy-Aware Best Fit (PRIMARY policy)
   - For each active host that can fit the VM, compute the PROJECTED POWER INCREASE using
     the linear power model (power with VM added minus current power).
   - Select the host with the SMALLEST projected power increase.
   - If no host fits, record SLA violation.

## VM Request Properties (randomized)
- CPU: 1 to 4 cores
- RAM: 256 MB to 1 GB
- Lifetime: 50 to 300 seconds
- Arrival: uniform random inter-arrival times across the run
- Use a FIXED RANDOM SEED so every policy faces the identical workload sequence.

## Workload Scenarios (3)
- Light:    20 VM requests
- Moderate: 50 VM requests
- Heavy:    100 VM requests
Only the request COUNT changes; CPU/RAM/lifetime ranges stay identical.

## Metrics (4) - written to results CSV
1. Total Energy Consumed (Joules) - from SimGrid Energy Plugin, summed over all hosts
2. Number of Active Hosts - count of hosts powered on at any point
3. SLA Violation Rate (%) - (rejected requests / total requests) * 100
4. Average Host Utilization (%) - mean CPU utilization across active hosts

## CSV Output Format
One CSV at results/simulation_results.csv with header:
policy,workload,energy_joules,active_hosts,sla_violation_rate,avg_utilization
Append one row per run. Twelve rows total after all runs.

## Run Structure
12 runs = 4 policies x 3 workloads. The program should let the policy and workload be
selected (e.g. via command-line arguments) so each run is reproducible and scriptable.

## Coding Conventions
- C++17, clear readable code with comments explaining the scheduling logic.
- Keep each policy in its own file under src/ with a shared interface/header.
- Use SimGrid S4U API (simgrid::s4u::). Use XBT_INFO for logging.
- Prioritize clarity and correctness over cleverness - this is academic code that must be
  explainable at a viva defense.

## Build Approach - IMPORTANT
Build INCREMENTALLY and verify each stage compiles and runs before moving on:
1. First: platform XML (20 hosts, 3 types, power profiles) + load it and print host info
2. Then: workload generator (fixed seed, 3 scenarios)
3. Then: ONE policy (Round Robin), compile, run, verify CSV output
4. Then: First Fit, then Best Fit, then Energy-Aware Best Fit - one at a time
5. Each policy must compile and produce sensible CSV before starting the next.
Do NOT generate all four policies at once. Wait for confirmation between stages.
