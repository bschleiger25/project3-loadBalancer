/**
 * @file main.cpp
 * @brief Entry point for load balancer simulation.
 */
#include <iostream>
#include <string>

#include "LoadBalancer.h"

namespace {
/**
 * @brief Parses an integer from input, otherwise returns fallback.
 * @param input Raw input string.
 * @param fallback Value returned on parse failure.
 * @return Parsed integer or fallback.
 */
int parseOrDefault(const std::string& input, int fallback) {
    if (input.empty()) {
        return fallback;
    }
    try {
        return std::stoi(input);
    } catch (...) {
        return fallback;
    }
}
}  // namespace

int main() {
    // Load defaults from config, then allow user overrides.
    LBConfig cfg;
    if (loadConfigFile("config.txt", cfg)) {
        std::cout << "Loaded config.txt\n";
    } else {
        std::cout << "config.txt not found, using built-in defaults\n";
    }

    std::string line;
    std::cout << "Number of servers [" << cfg.servers << "]: ";
    std::getline(std::cin, line);
    cfg.servers = parseOrDefault(line, cfg.servers);
    if (cfg.servers < 1) cfg.servers = 1;

    std::cout << "Number of cycles [" << cfg.cycles << "]: ";
    std::getline(std::cin, line);
    cfg.cycles = parseOrDefault(line, cfg.cycles);
    if (cfg.cycles < 1) cfg.cycles = 1;

    LoadBalancer lb(cfg);
    lb.generateInitialQueue(cfg.servers * 100);

    auto printAndLog = [&lb](const std::string& text) {
        std::cout << text << '\n';
        lb.appendToLog(text);
    };

    std::cout << "\n=== Run Configuration ===\n";
    lb.appendToLog("");
    lb.appendToLog("=== Run Configuration ===");
    printAndLog("Servers configured: " + std::to_string(cfg.servers));
    printAndLog("Cycles configured: " + std::to_string(cfg.cycles));
    printAndLog("Task time range: [" + std::to_string(cfg.minRequestTime) + ", " + std::to_string(cfg.maxRequestTime) + "]");
    printAndLog("Starting queue size: " + std::to_string(lb.getInitialQueueSize()));

    // Run the simulation for configured cycles.
    for (int i = 0; i < cfg.cycles; ++i) {
        lb.tick();
    }

    // Print end-of-run summary required by assignment.
    std::cout << "\n=== Simulation Summary ===\n";
    lb.appendToLog("");
    lb.appendToLog("=== Simulation Summary ===");
    printAndLog("Cycles run: " + std::to_string(cfg.cycles));
    printAndLog("Final time: " + std::to_string(lb.getTime()));
    printAndLog("Servers active: " + std::to_string(lb.getServerCount()));
    printAndLog("Servers busy: " + std::to_string(lb.getBusyServerCount()));
    printAndLog("Servers idle: " + std::to_string(lb.getIdleServerCount()));
    printAndLog("Starting queue size: " + std::to_string(lb.getInitialQueueSize()));
    printAndLog("Queue remaining: " + std::to_string(lb.getQueueSize()));
    printAndLog("Peak queue size: " + std::to_string(lb.getPeakQueueSize()));
    printAndLog("Task time range: [" + std::to_string(cfg.minRequestTime) + ", " + std::to_string(cfg.maxRequestTime) + "]");
    printAndLog("Requests generated: " + std::to_string(lb.getGeneratedRequests()));
    printAndLog("Requests blocked/rejected: " + std::to_string(lb.getBlockedRequests()));
    printAndLog("Requests assigned: " + std::to_string(lb.getAssignedRequests()));
    printAndLog("Requests completed: " + std::to_string(lb.getCompletedRequests()));
    printAndLog("Servers added: " + std::to_string(lb.getAddedServers()));
    printAndLog("Servers removed: " + std::to_string(lb.getRemovedServers()));

    return 0;
}
