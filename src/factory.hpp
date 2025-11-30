#pragma once
#include "models.hpp"
#include "policies.hpp"
#include "sched_base.hpp"
#include "sched_servers.hpp"
#include <memory>
#include <cctype>

inline std::unique_ptr<PeriodicScheduler> buildScheduler(
    const std::string& algName,
    const std::vector<PeriodicTask>& tasks,
    const std::vector<AperiodicJob>& aperiodic,
    const std::optional<ServerCfg>& serverCfg,
    int simTime)
{
    std::string name = algName;
    for (auto& c : name) c = std::toupper(c);

    // Pure periodic algorithms
    if (name == "RMS" || name == "DMS" || name == "EDF" || name == "LLF") {
        auto policy = makePolicy(name);
        return std::make_unique<PeriodicScheduler>(tasks, simTime, std::move(policy));
    }

    // Background
    if (name == "BACKGROUND") {
        auto policy = makePolicy("RMS"); 
        return std::make_unique<BackgroundScheduler>(tasks, aperiodic, simTime, std::move(policy));
    }

    // Server-based 
    if (name == "POLLING") {
        if (!serverCfg) throw std::runtime_error("Polling Server requires a D line in input.");
        auto policy = makePolicy("RMS");
        return std::make_unique<PollingServerScheduler>(
            tasks, aperiodic, *serverCfg, simTime, std::move(policy));
    }

    if (name == "DEFERRABLE") {
        if (!serverCfg) throw std::runtime_error("Deferrable Server requires a D line in input.");
        auto policy = makePolicy("RMS");
        return std::make_unique<DeferrableServerScheduler>(
            tasks, aperiodic, *serverCfg, simTime, std::move(policy));
    }

    if (name == "SPORADIC") {
        if (!serverCfg) throw std::runtime_error("Sporadic Server requires a D line in input.");
        auto policy = makePolicy("RMS");
        return std::make_unique<SporadicServerScheduler>(
            tasks, aperiodic, *serverCfg, simTime, std::move(policy));
    }

    throw std::runtime_error("Unknown algorithm: " + algName);
}