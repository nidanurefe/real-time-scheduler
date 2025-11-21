#pragma once
#include <string>
#include <vector>
#include <numeric>  
#include <optional>


struct PeriodicTask {
    std::string name;
    int arrival;    // r_i
    int execTime;   // e_i
    int period;     // p_i
    int deadline;   // d_i (relative)
};

struct PeriodicJob {
    const PeriodicTask* task;
    int releaseTime;
    int remaining;
    int absDeadline;
    std::string id;

    PeriodicJob(const PeriodicTask* t, int r)
        : task(t),
          releaseTime(r),
          remaining(t->execTime),
          absDeadline(r + t->deadline),
          id(t->name + "@" + std::to_string(r)) {}
};

struct AperiodicJob {
    std::string name;
    int releaseTime;
    int execTime;
    int remaining;
};

// Server config: Q, T, D
struct ServerCfg {
    int Q;
    int T;
    int D;
};

// Helper functions

inline int lcm(int a, int b) {
    return a / std::gcd(a, b) * b;
}

inline int hyperperiod(const std::vector<PeriodicTask>& tasks) {
    int h = 1;
    for (const auto& t : tasks) {
        h = lcm(h, t.period);
    }
    return h;
}