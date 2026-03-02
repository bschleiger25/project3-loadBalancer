/**
 * @file LoadBalancer.cpp
 * @brief Implements load balancer simulation logic and config parsing.
 */
#include "LoadBalancer.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>

namespace {
/** ANSI reset code. */
const std::string RESET = "\033[0m";
/** ANSI green code used for completion events. */
const std::string GREEN = "\033[32m";
/** ANSI red code used for blocked events. */
const std::string RED = "\033[31m";
/** ANSI yellow code used for scaling events. */
const std::string YELLOW = "\033[33m";
/** ANSI blue code used for new and assignment events. */
const std::string BLUE = "\033[34m";

/**
 * @brief Removes leading/trailing whitespace from a string.
 * @param s Input text.
 * @return Trimmed string.
 */
std::string trim(const std::string& s) {
    std::size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) {
        ++start;
    }
    std::size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) {
        --end;
    }
    return s.substr(start, end - start);
}
}  // namespace

LoadBalancer::LoadBalancer(const LBConfig& cfg)
    : time(0),
      last_adjust_time(0),
      blocked_ranges(cfg.blockedRanges),
      config(cfg),
      rng(std::random_device{}()),
      probDist(0.0, 1.0),
      octetDist(0, 255),
      timeDist(cfg.minRequestTime, std::max(cfg.minRequestTime, cfg.maxRequestTime)),
      jobDist(0, 1),
      generated_requests(0),
      blocked_requests(0),
      completed_requests(0),
      assigned_requests(0),
      added_servers(0),
      removed_servers(0),
      initial_queue_size(0),
      peak_queue_size(0),
      log_stream("loadbalancer.log", std::ios::out) {
    if (log_stream.is_open()) {
        log_stream << "=== Load Balancer Log Start ===\n";
    }
    // Start with at least one server even if config input is invalid.
    int initialServers = std::max(1, config.servers);
    for (int i = 0; i < initialServers; ++i) {
        servers.push_back(new WebServer());
    }
}

LoadBalancer::~LoadBalancer() {
    for (WebServer* server : servers) {
        delete server;
    }
    servers.clear();
    if (log_stream.is_open()) {
        log_stream << "=== Load Balancer Log End ===\n";
        log_stream.close();
    }
}

/**
 * @brief Prints a colored event line with current simulation time.
 */
void LoadBalancer::logLine(const std::string& color, const std::string& message) {
    std::cout << color << "[t=" << time << "] " << message << RESET << '\n';
    if (log_stream.is_open()) {
        log_stream << "[t=" << time << "] " << message << '\n';
    }
}

/**
 * @brief Generates a random IPv4 address.
 */
std::string LoadBalancer::randomIp() {
    std::ostringstream ip;
    ip << octetDist(rng) << '.' << octetDist(rng) << '.' << octetDist(rng) << '.' << octetDist(rng);
    return ip.str();
}

/**
 * @brief Generates a randomized request.
 */
Request LoadBalancer::randomRequest() {
    char type = (jobDist(rng) == 0) ? 'P' : 'S';
    return Request(randomIp(), randomIp(), timeDist(rng), type);
}

/**
 * @brief Returns true if source IP starts with any blocked prefix.
 */
bool LoadBalancer::isBlocked(const std::string& ip) const {
    for (const std::string& prefix : blocked_ranges) {
        if (ip.rfind(prefix, 0) == 0) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Generates the initial full queue for the simulation.
 */
void LoadBalancer::generateInitialQueue(int count) {
    for (int i = 0; i < count; ++i) {
        Request req = randomRequest();
        ++generated_requests;
        if (isBlocked(req.ip_in)) {
            ++blocked_requests;
            logLine(RED, "BLOCKED initial request from " + req.ip_in);
            continue;
        }
        queue.enqueue(req);
        logLine(BLUE, "NEW initial request " + req.ip_in + " -> " + req.ip_out + " [" + req.job_type + "]");
    }
    initial_queue_size = static_cast<int>(queue.size());
    if (initial_queue_size > peak_queue_size) {
        peak_queue_size = initial_queue_size;
    }
}

/**
 * @brief Adds probabilistic bursts of new requests each cycle.
 */
void LoadBalancer::maybeGenerateNewRequests() {
    double roll = probDist(rng);
    if (roll > config.newRequestProb) {
        return;
    }

    int burst = 1 + (octetDist(rng) % 3);
    for (int i = 0; i < burst; ++i) {
        Request req = randomRequest();
        ++generated_requests;
        if (isBlocked(req.ip_in)) {
            ++blocked_requests;
            logLine(RED, "BLOCKED request from " + req.ip_in);
            continue;
        }
        queue.enqueue(req);
        logLine(BLUE, "NEW request " + req.ip_in + " -> " + req.ip_out + " [" + req.job_type + "]");
    }
}

/**
 * @brief Assigns queued work to all currently idle servers.
 */
void LoadBalancer::dispatchRequestsToServers() {
    for (std::size_t i = 0; i < servers.size() && !queue.empty(); ++i) {
        WebServer* server = servers[i];
        if (server->isBusy()) {
            continue;
        }
        Request req = queue.dequeue();
        server->assignRequest(req);
        ++assigned_requests;
        std::ostringstream msg;
        msg << "ASSIGN server#" << i << " <= " << req.ip_in << " -> " << req.ip_out << " (" << req.time_required
            << " cycles, type " << req.job_type << ")";
        logLine(BLUE, msg.str());
    }
}

/**
 * @brief Adds one server and records/logs the event.
 */
bool LoadBalancer::addServer() {
    servers.push_back(new WebServer());
    ++added_servers;
    logLine(YELLOW, "SCALE UP: added server (total=" + std::to_string(servers.size()) + ")");
    return true;
}

/**
 * @brief Removes one idle server and records/logs the event.
 */
bool LoadBalancer::removeServer() {
    if (servers.size() <= 1) {
        return false;
    }

    for (auto it = servers.rbegin(); it != servers.rend(); ++it) {
        if (!(*it)->isBusy()) {
            WebServer* victim = *it;
            auto baseIt = std::next(it).base();
            servers.erase(baseIt);
            delete victim;
            ++removed_servers;
            logLine(YELLOW, "SCALE DOWN: removed idle server (total=" + std::to_string(servers.size()) + ")");
            return true;
        }
    }
    return false;
}

/**
 * @brief Applies autoscaling rules using queue thresholds and cooldown.
 */
void LoadBalancer::adjustServers() {
    if (time - last_adjust_time < config.adjustDelay) {
        return;
    }

    std::size_t serverCount = servers.size();
    std::size_t qSize = queue.size();
    std::size_t minThreshold = static_cast<std::size_t>(config.minQueuePerServer) * serverCount;
    std::size_t maxThreshold = static_cast<std::size_t>(config.maxQueuePerServer) * serverCount;

    if (qSize > maxThreshold) {
        if (addServer()) {
            last_adjust_time = time;
        }
        return;
    }

    if (qSize < minThreshold) {
        if (removeServer()) {
            last_adjust_time = time;
        }
    }
}

/**
 * @brief Advances full simulation state by one cycle.
 */
void LoadBalancer::tick() {
    ++time;
    for (std::size_t i = 0; i < servers.size(); ++i) {
        WebServer* server = servers[i];
        if (!server->isBusy()) {
            continue;
        }
        Request done = server->getCurrentRequest();
        if (server->tick()) {
            ++completed_requests;
            std::ostringstream msg;
            msg << "COMPLETE server#" << i << " " << done.ip_in << " -> " << done.ip_out << " [" << done.job_type
                << "]";
            logLine(GREEN, msg.str());
        }
    }

    maybeGenerateNewRequests();
    dispatchRequestsToServers();
    adjustServers();
    if (static_cast<int>(queue.size()) > peak_queue_size) {
        peak_queue_size = static_cast<int>(queue.size());
    }
}

/**
 * @brief Gets current simulation cycle count.
 */
int LoadBalancer::getTime() const {
    return time;
}

/**
 * @brief Gets current pending queue size.
 */
std::size_t LoadBalancer::getQueueSize() const {
    return queue.size();
}

/**
 * @brief Gets current number of active servers.
 */
std::size_t LoadBalancer::getServerCount() const {
    return servers.size();
}

/**
 * @brief Gets count of completed requests.
 */
int LoadBalancer::getCompletedRequests() const {
    return completed_requests;
}

/**
 * @brief Gets count of blocked requests.
 */
int LoadBalancer::getBlockedRequests() const {
    return blocked_requests;
}

/**
 * @brief Gets count of generated requests.
 */
int LoadBalancer::getGeneratedRequests() const {
    return generated_requests;
}

/**
 * @brief Gets count of assigned requests.
 */
int LoadBalancer::getAssignedRequests() const {
    return assigned_requests;
}

/**
 * @brief Gets number of scale-up events.
 */
int LoadBalancer::getAddedServers() const {
    return added_servers;
}

/**
 * @brief Gets number of scale-down events.
 */
int LoadBalancer::getRemovedServers() const {
    return removed_servers;
}

/**
 * @brief Gets initial queue size after startup seeding.
 */
int LoadBalancer::getInitialQueueSize() const {
    return initial_queue_size;
}

/**
 * @brief Gets peak queue size seen during simulation.
 */
int LoadBalancer::getPeakQueueSize() const {
    return peak_queue_size;
}

/**
 * @brief Counts servers currently processing requests.
 */
int LoadBalancer::getBusyServerCount() const {
    int busy = 0;
    for (const WebServer* server : servers) {
        if (server->isBusy()) {
            ++busy;
        }
    }
    return busy;
}

/**
 * @brief Counts currently idle servers.
 */
int LoadBalancer::getIdleServerCount() const {
    return static_cast<int>(servers.size()) - getBusyServerCount();
}

/**
 * @brief Appends an unformatted text line to the log file.
 */
void LoadBalancer::appendToLog(const std::string& message) {
    if (log_stream.is_open()) {
        log_stream << message << '\n';
    }
}

/**
 * @brief Parses key=value config values into LBConfig.
 */
bool loadConfigFile(const std::string& path, LBConfig& cfg) {
    std::ifstream in(path);
    if (!in.is_open()) {
        return false;
    }

    std::string line;
    while (std::getline(in, line)) {
        std::string trimmed = trim(line);
        if (trimmed.empty() || trimmed[0] == '#') {
            continue;
        }

        std::size_t eq = trimmed.find('=');
        if (eq == std::string::npos) {
            continue;
        }

        std::string key = trim(trimmed.substr(0, eq));
        std::string value = trim(trimmed.substr(eq + 1));

        if (key == "servers") cfg.servers = std::stoi(value);
        else if (key == "cycles") cfg.cycles = std::stoi(value);
        else if (key == "minQueuePerServer") cfg.minQueuePerServer = std::stoi(value);
        else if (key == "maxQueuePerServer") cfg.maxQueuePerServer = std::stoi(value);
        else if (key == "adjustDelay") cfg.adjustDelay = std::stoi(value);
        else if (key == "newRequestProb") cfg.newRequestProb = std::stod(value);
        else if (key == "minRequestTime") cfg.minRequestTime = std::stoi(value);
        else if (key == "maxRequestTime") cfg.maxRequestTime = std::stoi(value);
        else if (key == "blockedRanges") {
            cfg.blockedRanges.clear();
            std::stringstream ss(value);
            std::string token;
            while (std::getline(ss, token, ',')) {
                std::string prefix = trim(token);
                if (!prefix.empty()) {
                    cfg.blockedRanges.push_back(prefix);
                }
            }
        }
    }

    if (cfg.servers < 1) cfg.servers = 1;
    if (cfg.cycles < 1) cfg.cycles = 1;
    if (cfg.minQueuePerServer < 0) cfg.minQueuePerServer = 0;
    if (cfg.maxQueuePerServer < cfg.minQueuePerServer) cfg.maxQueuePerServer = cfg.minQueuePerServer;
    if (cfg.adjustDelay < 1) cfg.adjustDelay = 1;
    if (cfg.newRequestProb < 0.0) cfg.newRequestProb = 0.0;
    if (cfg.newRequestProb > 1.0) cfg.newRequestProb = 1.0;
    if (cfg.minRequestTime < 1) cfg.minRequestTime = 1;
    if (cfg.maxRequestTime < cfg.minRequestTime) cfg.maxRequestTime = cfg.minRequestTime;
    return true;
}
