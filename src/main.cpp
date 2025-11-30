#include <iostream>
#include <string>

#include "models.hpp"
#include "parser.hpp"
#include "factory.hpp"

int main() {
    try {
        std::string path;
        std::cout << "Input file path: ";
        std::getline(std::cin, path);

        auto [tasks, aperiodic, serverCfg] = parseInputFile(path);

        if (tasks.empty()) {
            std::cerr << "No periodic tasks found in input file.\n";
            return 1;
        }

        int hp = hyperperiod(tasks);
        std::cout << "Hyperperiod = " << hp << "\n";

        std::string simStr;
        std::cout << "Simulation time (empty = hyperperiod): ";
        std::getline(std::cin, simStr);
        int simTime = simStr.empty() ? hp : std::stoi(simStr);

        std::cout << "Algorithms:\n"
                  << "  EDF, RMS, DMS, LLF\n"
                  << "  BACKGROUND\n"
                  << "  POLLING, DEFERRABLE, SPORADIC\n";

        std::string alg;
        std::cout << "Algorithm: ";
        std::getline(std::cin, alg);

        ServerRuleConfig rules = loadServerRuleConfig("settings.json");
        auto scheduler = buildScheduler(alg, tasks, aperiodic, serverCfg, simTime);

        scheduler->run();
        std::cout << "\n" << scheduler->summaryText() << "\n";

    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << "\n";
        return 1;
    }
    return 0;
}