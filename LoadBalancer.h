/**
 * @file LoadBalancer.h
 * @brief Declares load balancer configuration and simulation classes.
 */
#ifndef LOAD_BALANCER_H
#define LOAD_BALANCER_H

#include <fstream>
#include <random>
#include <string>
#include <vector>

#include "RequestQueue.h"
#include "WebServer.h"

/**
 * @brief Configuration values loaded from config.txt.
 */
struct LBConfig {
    /** Initial number of servers. */
    int servers = 10;
    /** Number of simulation cycles to run. */
    int cycles = 10000;
    /** Lower queue threshold per server for scaling down. */
    int minQueuePerServer = 50;
    /** Upper queue threshold per server for scaling up. */
    int maxQueuePerServer = 80;
    /** Cooldown in cycles after each scaling action. */
    int adjustDelay = 50;
    /** Per-cycle probability of generating new incoming requests. */
    double newRequestProb = 0.3;
    /** Minimum request processing time in cycles. */
    int minRequestTime = 2;
    /** Maximum request processing time in cycles. */
    int maxRequestTime = 12;
    /** Blocked source IP prefixes (firewall rules). */
    std::vector<std::string> blockedRanges = {"192.168.", "10.0."};
};

/**
 * @brief Simulates a load balancer managing server pool, queue, and events.
 */
class LoadBalancer {
private:
    /** Dynamically managed server pool. */
    std::vector<WebServer*> servers;
    /** Pending request queue. */
    RequestQueue queue;
    /** Current simulation time in cycles. */
    int time;
    /** Last cycle when a scale action happened. */
    int last_adjust_time;
    /** Blocked IP prefixes used for filtering requests. */
    std::vector<std::string> blocked_ranges;
    /** Runtime configuration values. */
    LBConfig config;

    std::mt19937 rng;
    std::uniform_real_distribution<double> probDist;
    std::uniform_int_distribution<int> octetDist;
    std::uniform_int_distribution<int> timeDist;
    std::uniform_int_distribution<int> jobDist;

    /** Total number of generated requests. */
    int generated_requests;
    /** Total number of blocked requests. */
    int blocked_requests;
    /** Total number of completed requests. */
    int completed_requests;
    /** Total number of assigned requests. */
    int assigned_requests;
    /** Number of server add events. */
    int added_servers;
    /** Number of server remove events. */
    int removed_servers;
    /** Queue size after initial generation completes. */
    int initial_queue_size;
    /** Highest queue size observed during the run. */
    int peak_queue_size;
    /** Output stream for persistent log file. */
    std::ofstream log_stream;

    /**
     * @brief Generates a random IPv4 string.
     * @return Random IP address.
     */
    std::string randomIp();
    /**
     * @brief Creates one random request.
     * @return Randomly generated request.
     */
    Request randomRequest();
    /**
     * @brief Writes a colored log line to stdout.
     * @param color ANSI color escape code.
     * @param message Message body.
     */
    void logLine(const std::string& color, const std::string& message);
    /**
     * @brief Adds one server to the pool.
     * @return True when a server was added.
     */
    bool addServer();
    /**
     * @brief Removes one idle server from the pool.
     * @return True when a server was removed.
     */
    bool removeServer();

public:
    /**
     * @brief Constructs a load balancer using configuration values.
     * @param cfg Loaded configuration.
     */
    explicit LoadBalancer(const LBConfig& cfg);
    /**
     * @brief Frees dynamically allocated servers.
     */
    ~LoadBalancer();

    /**
     * @brief Executes one simulation cycle.
     */
    void tick();
    /**
     * @brief Fills the queue with initial random requests.
     * @param count Number of requests to generate.
     */
    void generateInitialQueue(int count);
    /**
     * @brief Randomly generates new requests during simulation.
     */
    void maybeGenerateNewRequests();
    /**
     * @brief Assigns queued requests to available servers.
     */
    void dispatchRequestsToServers();
    /**
     * @brief Scales server count based on queue thresholds and cooldown.
     */
    void adjustServers();
    /**
     * @brief Checks whether an IP is blocked by prefix list.
     * @param ip Source IP address.
     * @return True if blocked.
     */
    bool isBlocked(const std::string& ip) const;

    /** @brief Current simulation time. */
    int getTime() const;
    /** @brief Current queue size. */
    std::size_t getQueueSize() const;
    /** @brief Current number of active servers. */
    std::size_t getServerCount() const;
    /** @brief Number of completed requests. */
    int getCompletedRequests() const;
    /** @brief Number of blocked requests. */
    int getBlockedRequests() const;
    /** @brief Number of generated requests. */
    int getGeneratedRequests() const;
    /** @brief Number of assigned requests. */
    int getAssignedRequests() const;
    /** @brief Number of scale-up events. */
    int getAddedServers() const;
    /** @brief Number of scale-down events. */
    int getRemovedServers() const;
    /** @brief Queue size at simulation start (after initial generation). */
    int getInitialQueueSize() const;
    /** @brief Highest queue length observed. */
    int getPeakQueueSize() const;
    /** @brief Number of currently busy servers. */
    int getBusyServerCount() const;
    /** @brief Number of currently idle servers. */
    int getIdleServerCount() const;
    /**
     * @brief Appends a plain line to the persistent log file.
     * @param message Line to append.
     */
    void appendToLog(const std::string& message);
};

/**
 * @brief Loads load balancer settings from key=value config file.
 * @param path File path to config text file.
 * @param cfg Output config object.
 * @return True if file opened successfully, false otherwise.
 */
bool loadConfigFile(const std::string& path, LBConfig& cfg);

#endif
