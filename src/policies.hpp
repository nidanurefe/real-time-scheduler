#pragma once
#include "models.hpp"
#include <memory>
#include <string>
#include <unordered_map>

// Priority Policy Base
class PriorityPolicy {
public:
    virtual ~PriorityPolicy() = default;
    virtual double key(const PeriodicJob& job, int now) const = 0;
    virtual std::string name() const = 0;
};

// RMS
class RMSPolicy : public PriorityPolicy {
public:
    double key(const PeriodicJob& job, int) const override {
        return job.task->period;
    }
    std::string name() const override { return "RMS"; }
};

// DMS
class DMSPolicy : public PriorityPolicy {
public:
    double key(const PeriodicJob& job, int) const override {
        return job.task->deadline;
    }
    std::string name() const override { return "DMS"; }
};

// EDF
class EDFPolicy : public PriorityPolicy {
public:
    double key(const PeriodicJob& job, int) const override {
        return job.absDeadline;
    }
    std::string name() const override { return "EDF"; }
};

// LLF
class LLFPolicy : public PriorityPolicy {
public:
    double key(const PeriodicJob& job, int now) const override {
        int laxity = job.absDeadline - now - job.remaining;
        return (laxity < -1'000'000 ? -1'000'000 : laxity);
    }
    std::string name() const override { return "LLF"; }
};

// Policy factory
inline std::unique_ptr<PriorityPolicy> makePolicy(const std::string& algName) {
    std::string name = algName;
    for (auto& c : name) c = std::toupper(c);

    if (name == "RMS") return std::make_unique<RMSPolicy>();
    if (name == "DMS") return std::make_unique<DMSPolicy>();
    if (name == "EDF") return std::make_unique<EDFPolicy>();
    if (name == "LLF") return std::make_unique<LLFPolicy>();

    throw std::runtime_error("Unknown policy: " + algName);
}