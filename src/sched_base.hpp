#pragma once
#include "models.hpp"
#include "policies.hpp"
#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <iostream>

class PeriodicScheduler {
protected:
    std::vector<PeriodicTask> tasks_;
    int simTime_;
    std::unique_ptr<PriorityPolicy> policy_;

    std::vector<PeriodicJob> ready_;
    std::vector<PeriodicJob> finished_;
    std::vector<PeriodicJob> missed_;
    std::vector<std::string> timeline_; 

public:
    PeriodicScheduler(const std::vector<PeriodicTask>& tasks,
                      int simTime,
                      std::unique_ptr<PriorityPolicy> policy)
        : tasks_(tasks),
          simTime_(simTime),
          policy_(std::move(policy)),
          timeline_(simTime, "IDLE")
    {}

    virtual ~PeriodicScheduler() = default;

    virtual void releaseJobs(int t) {
        for (auto& task : tasks_) {
            if (t < task.arrival) continue;
            if ((t - task.arrival) % task.period == 0) {
                ready_.emplace_back(&task, t);
            }
        }
    }

    virtual void checkDeadlines(int t) {
        for (auto it = ready_.begin(); it != ready_.end();) {
            if (t > it->absDeadline && it->remaining > 0) {
                missed_.push_back(*it);
                it = ready_.erase(it);
            } else {
                ++it;
            }
        }
    }

    virtual PeriodicJob* chooseJob(int t) {
        if (ready_.empty()) return nullptr;
        PeriodicJob* best = &ready_.front();
        for (auto& j : ready_) {
            if (policy_->key(j, t) < policy_->key(*best, t)) {
                best = &j;
            }
        }
        return best;
    }

    // Bir zaman adımı
    virtual void step(int t) {
        releaseJobs(t);
        checkDeadlines(t);
        auto* job = chooseJob(t);
        if (!job) {
            timeline_[t] = "IDLE";
            return;
        }
        job->remaining--;
        timeline_[t] = job->task->name;
        if (job->remaining == 0) {
            finished_.push_back(*job);
            ready_.erase(std::remove_if(ready_.begin(), ready_.end(),
                [&](const PeriodicJob& j){ return j.id == job->id; }), ready_.end());
        }
    }

    virtual void run() {
        for (int t = 0; t < simTime_; ++t) {
            step(t);
        }
    }

    virtual std::string summaryText() const {
        std::string out;
        out += "=== Periodic Scheduler (" + policy_->name() + ") ===\n";
        out += "Timeline (time: task):\n";
        for (int t = 0; t < simTime_; ++t) {
            out += std::to_string(t) + " : " + timeline_[t] + "\n";
        }
        out += "\nFinished jobs: " + std::to_string(finished_.size()) + "\n";
        out += "Missed deadlines: " + std::to_string(missed_.size()) + "\n";
        if (!missed_.empty()) {
            out += "Missed jobs:\n";
            for (const auto& j : missed_) {
                out += "  " + j.id + " (deadline " +
                       std::to_string(j.absDeadline) + ")\n";
            }
        }
        out += "\nGantt-like:\n";
        for (int t = 0; t < simTime_; ++t) {
            const auto& label = timeline_[t];
            if (label == "IDLE") out += "_";
            else out += (label.size() > 1 ? label[1] : label[0]);
        }
        out += "\n";
        return out;
    }
};


// ---------------- Background Scheduler ----------------

class BackgroundScheduler : public PeriodicScheduler {
    std::vector<AperiodicJob> aperiodicAll_;
    std::vector<AperiodicJob> aperiodicReady_;

public:
    BackgroundScheduler(const std::vector<PeriodicTask>& tasks,
                        const std::vector<AperiodicJob>& aperiodic,
                        int simTime,
                        std::unique_ptr<PriorityPolicy> policy)
        : PeriodicScheduler(tasks, simTime, std::move(policy)),
          aperiodicAll_(aperiodic)
    {}

    void releaseAperiodic(int t) {
        for (auto it = aperiodicAll_.begin(); it != aperiodicAll_.end();) {
            if (it->releaseTime == t) {
                aperiodicReady_.push_back(*it);
                it = aperiodicAll_.erase(it);
            } else {
                ++it;
            }
        }
    }

    void step(int t) override {
        // periodic
        releaseJobs(t);
        releaseAperiodic(t);
        checkDeadlines(t);

        // önce periodic
        auto* job = chooseJob(t);
        if (job) {
            job->remaining--;
            timeline_[t] = job->task->name;
            if (job->remaining == 0) {
                finished_.push_back(*job);
                ready_.erase(std::remove_if(ready_.begin(), ready_.end(),
                    [&](const PeriodicJob& j){ return j.id == job->id; }), ready_.end());
            }
            return;
        }

        // boşsa aperiodic
        if (!aperiodicReady_.empty()) {
            auto& aj = aperiodicReady_.front();
            aj.remaining--;
            timeline_[t] = aj.name;
            if (aj.remaining == 0) {
                aperiodicReady_.erase(aperiodicReady_.begin());
            }
        } else {
            timeline_[t] = "IDLE";
        }
    }

    std::string summaryText() const override {
        std::string base = PeriodicScheduler::summaryText();
        base += "Remaining aperiodic jobs: " +
                std::to_string(aperiodicReady_.size()) + "\n";
        return base;
    }
};