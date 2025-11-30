#pragma once
#include "models.hpp"
#include <tuple>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cmath>       


inline std::tuple<
    std::vector<PeriodicTask>,
    std::vector<AperiodicJob>,
    std::optional<ServerCfg>
> parseInputFile(const std::string& path)
{
    std::vector<PeriodicTask> tasks;
    std::vector<AperiodicJob> aperiodic;
    std::optional<ServerCfg> serverCfg;

    std::ifstream in(path);
    if (!in) {
        throw std::runtime_error("Could not open input file: " + path);
    }

    
    auto toInt = [](double x) {
        return static_cast<int>(std::round(x));
    };

    std::string raw;
    int lineIdx = 0;
    while (std::getline(in, raw)) {
        ++lineIdx;

        // strip comments
        auto sharpPos = raw.find('#');
        if (sharpPos != std::string::npos) {
            raw = raw.substr(0, sharpPos);
        }

        // trim leading spaces
        std::string line;
        {
            std::stringstream ss(raw);
            ss >> std::ws;
            std::getline(ss, line);
        }
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string tag;
        ss >> tag;
        for (auto &c : tag) c = std::toupper(c);

        try {
            if (tag == "P") {
                // P ri ei pi di
                // P ri ei pi
                // P ei pi
                std::vector<double> nums;
                double v;
                while (ss >> v) nums.push_back(v);

                double r_d, e_d, p_d, d_d;
                if (nums.size() == 4) {
                    r_d = nums[0]; e_d = nums[1]; p_d = nums[2]; d_d = nums[3];
                } else if (nums.size() == 3) {
                    r_d = nums[0]; e_d = nums[1]; p_d = nums[2]; d_d = p_d;
                } else if (nums.size() == 2) {
                    e_d = nums[0]; p_d = nums[1]; r_d = 0.0; d_d = p_d;
                } else {
                    throw std::runtime_error(
                        "P line must be: 'P ri ei pi di' or 'P ri ei pi' or 'P ei pi'"
                    );
                }

                int r_i = toInt(r_d);
                int e_i = toInt(e_d);
                int p_i = toInt(p_d);
                int d_i = toInt(d_d);

                std::string name = "T" + std::to_string(tasks.size() + 1);
                tasks.push_back(PeriodicTask{name, r_i, e_i, p_i, d_i});
            }
            else if (tag == "A") {
                // A ri ei
                std::vector<double> nums;
                double v;
                while (ss >> v) nums.push_back(v);
                if (nums.size() != 2) {
                    throw std::runtime_error("A line must be: 'A ri ei'");
                }
                int r_i = toInt(nums[0]);
                int e_i = toInt(nums[1]);
                std::string name = "A" + std::to_string(aperiodic.size() + 1);
                aperiodic.push_back(AperiodicJob{name, r_i, e_i, e_i});
            }
            else if (tag == "D") {
                // D ei pi di -> server config
                std::vector<double> nums;
                double v;
                while (ss >> v) nums.push_back(v);
                if (nums.size() != 3) {
                    throw std::runtime_error("D line must be: 'D ei pi di'");
                }
                int Q = toInt(nums[0]);
                int T = toInt(nums[1]);
                int D = toInt(nums[2]);
                serverCfg = ServerCfg{Q, T, D};
            }
            else {
                throw std::runtime_error("Unknown tag '" + tag + "'");
            }
        }
        catch (const std::exception& e) {
            throw std::runtime_error(
                "Error at line " + std::to_string(lineIdx) +
                " ('" + line + "'): " + e.what()
            );
        }
    }

    return {tasks, aperiodic, serverCfg};
}