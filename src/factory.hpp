#pragma once
#include "models.hpp"
#include "policies.hpp"
#include "sched_base.hpp"
#include "sched_servers.hpp"
#include "server_rules.hpp"
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
    for (auto &c : name) c = std::toupper(c);

    // Read server rules from settings.json
    ServerRuleConfig rules = loadServerRuleConfig("settings.json");

    // Pure periodic 
    if (name == "RMS" || name == "DMS" || name == "EDF" || name == "LLF") {
        auto policy = makePolicy(name);
        return std::unique_ptr<PeriodicScheduler>(
            new PeriodicScheduler(tasks, simTime, std::move(policy))
        );
    }

    // Background 
    if (name == "BACKGROUND") {
        auto policy = makePolicy("RMS"); 
        return std::unique_ptr<PeriodicScheduler>(
            new BackgroundScheduler(tasks, aperiodic, simTime, std::move(policy))
        );
    }

    // Server based 
    if (name == "POLLING") {
        if (!serverCfg) {
            throw std::runtime_error("Polling Server requires a D line in input.");
        }
        auto policy = makePolicy("RMS");
        return std::unique_ptr<PeriodicScheduler>(
            new PollingServerScheduler(
                tasks, aperiodic, *serverCfg, simTime,
                std::move(policy), rules.polling)
        );
    }

    if (name == "DEFERRABLE") {
        if (!serverCfg) {
            throw std::runtime_error("Deferrable Server requires a D line in input.");
        }
        auto policy = makePolicy("RMS");
        return std::unique_ptr<PeriodicScheduler>(
            new DeferrableServerScheduler(
                tasks, aperiodic, *serverCfg, simTime,
                std::move(policy), rules.deferrable)
        );
    }

    if (name == "SPORADIC") {
        if (!serverCfg) {
            throw std::runtime_error("Sporadic Server requires a D line in input.");
        }
        auto policy = makePolicy("RMS");
        return std::unique_ptr<PeriodicScheduler>(
            new SporadicServerScheduler(
                tasks, aperiodic, *serverCfg, simTime,
                std::move(policy), rules.sporadic)
        );
    }

    throw std::runtime_error("Unknown algorithm: " + algName);
}