#pragma once
#include "sched_base.hpp"
#include <utility>
#include <algorithm> 

class BaseServerScheduler : public PeriodicScheduler {
protected:
    PeriodicTask serverTask_;
    int Q_, T_, D_;  

    std::vector<AperiodicJob> aperiodicAll_;
    std::vector<AperiodicJob> aperiodicReady_;

    int serverBudget_ = 0;
    int serverPeriodStart_ = 0;

public:
    BaseServerScheduler(const std::vector<PeriodicTask>& tasks,
                        const std::vector<AperiodicJob>& aperiodic,
                        const ServerCfg& cfg,
                        int simTime,
                        std::unique_ptr<PriorityPolicy> policy)
        : PeriodicScheduler({}, simTime, std::move(policy)),
          serverTask_({"S", 0, cfg.Q, cfg.T, cfg.D}),
          Q_(cfg.Q), T_(cfg.T), D_(cfg.D),
          aperiodicAll_(aperiodic)
    {
        tasks_ = tasks;
        tasks_.push_back(serverTask_);

        timeline_.assign(simTime_, "IDLE");
    }

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

    // Each server applies its own replenishment & consumption rule
    virtual void updateServerBudget(int t) = 0;

    virtual void consumeBudget(int /*t*/) {
        if (serverBudget_ > 0) --serverBudget_;
    }

    void step(int t) override {
        releaseJobs(t);
        releaseAperiodic(t);
        checkDeadlines(t);
        updateServerBudget(t);

        auto* job = chooseJob(t);

        // If selected job is the server itself
        if (job && job->task == &serverTask_) {
            if (serverBudget_ > 0 && !aperiodicReady_.empty()) {
                auto& aj = aperiodicReady_.front();
                aj.remaining--;
                consumeBudget(t);
                timeline_[t] = aj.name;
                if (aj.remaining == 0) {
                    aperiodicReady_.erase(aperiodicReady_.begin());
                }
                return;
            } else {
                // Remove server from ready list if no budget or aperiodic task available
                ready_.erase(std::remove_if(ready_.begin(), ready_.end(),
                    [&](const PeriodicJob& j){ return j.task == &serverTask_; }),
                    ready_.end());
                job = chooseJob(t);
            }
        }

        // If no jobs available -> idle
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
};


// Polling Server

class PollingServerScheduler : public BaseServerScheduler {
public:
    using BaseServerScheduler::BaseServerScheduler;

    void updateServerBudget(int t) override {
        if (t % T_ == 0) {                     
            serverPeriodStart_ = t;
            if (!aperiodicReady_.empty())
                serverBudget_ = Q_;
            else
                serverBudget_ = 0;            // Budget = 0 if no aperiodic task available
        }
    }
};


// Deferrable Server

class DeferrableServerScheduler : public BaseServerScheduler {
public:
    using BaseServerScheduler::BaseServerScheduler;

    void updateServerBudget(int t) override {
        if (t % T_ == 0) {                   
            serverPeriodStart_ = t;
            serverBudget_ = Q_;                // 100% budget at each period
        }
    }
};


// Sporadic Server

class SporadicServerScheduler : public BaseServerScheduler {
    // (time, amount)
    std::vector<std::pair<int,int>> replenishments_;

public:
    SporadicServerScheduler(const std::vector<PeriodicTask>& tasks,
                            const std::vector<AperiodicJob>& aperiodic,
                            const ServerCfg& cfg,
                            int simTime,
                            std::unique_ptr<PriorityPolicy> policy)
        : BaseServerScheduler(tasks, aperiodic, cfg, simTime, std::move(policy))
    {
        serverBudget_ = Q_;
    }

    void updateServerBudget(int t) override {
        for (auto it = replenishments_.begin(); it != replenishments_.end();) {
            if (it->first <= t) {
                serverBudget_ = std::min(Q_, serverBudget_ + it->second);  
                it = replenishments_.erase(it);
            } else {
                ++it;
            }
        }
    }

    void consumeBudget(int t) override {
        if (serverBudget_ <= 0) return;
        --serverBudget_;
        replenishments_.push_back({t + T_, 1});
    }
};