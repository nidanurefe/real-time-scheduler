#pragma once
#include <string>
#include <fstream>
#include <nlohmann/json.hpp>

struct PollingConfig {
    bool budget_if_aperiodic_ready = true;
};

struct DeferrableConfig {
    bool reset_budget_each_period = true;
};

struct SporadicConfig {
    int    replenish_amount       = 1;
    double replenish_delay_factor = 1.0;
};

struct ServerRuleConfig {
    PollingConfig    polling;
    DeferrableConfig deferrable;
    SporadicConfig   sporadic;
};

inline ServerRuleConfig loadServerRuleConfig(const std::string& path)
{
    ServerRuleConfig cfg; 

    std::ifstream f(path);
    if (!f) {
        return cfg;
    }

    nlohmann::json j;
    f >> j;

    if (!j.contains("servers")) {
        return cfg;
    }

    auto js = j["servers"];

    if (js.contains("POLLING")) {
        auto jp = js["POLLING"];
        if (jp.contains("budget_if_aperiodic_ready")) {
            cfg.polling.budget_if_aperiodic_ready =
                jp["budget_if_aperiodic_ready"].get<bool>();
        }
    }

    if (js.contains("DEFERRABLE")) {
        auto jd = js["DEFERRABLE"];
        if (jd.contains("reset_budget_each_period")) {
            cfg.deferrable.reset_budget_each_period =
                jd["reset_budget_each_period"].get<bool>();
        }
    }

    if (js.contains("SPORADIC")) {
        auto jspr = js["SPORADIC"];
        if (jspr.contains("replenish_amount")) {
            cfg.sporadic.replenish_amount =
                jspr["replenish_amount"].get<int>();
        }
        if (jspr.contains("replenish_delay_factor")) {
            cfg.sporadic.replenish_delay_factor =
                jspr["replenish_delay_factor"].get<double>();
        }
    }

    return cfg;
}