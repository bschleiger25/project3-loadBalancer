/**
 * @file WebServer.cpp
 * @brief Implements WebServer behavior.
 */
#include "WebServer.h"

/**
 * @brief Constructs an idle WebServer.
 */
WebServer::WebServer() : busy(false), remaining_time(0), current_request() {}

/**
 * @brief Advances active work by one cycle.
 */
bool WebServer::tick() {
    if (!busy) {
        return false;
    }

    --remaining_time;
    if (remaining_time <= 0) {
        busy = false;
        remaining_time = 0;
        return true;
    }
    return false;
}

/**
 * @brief Assigns a new request for processing.
 */
void WebServer::assignRequest(const Request& request) {
    current_request = request;
    remaining_time = request.time_required;
    busy = true;
}

/**
 * @brief Returns whether this server is busy.
 */
bool WebServer::isBusy() const {
    return busy;
}

/**
 * @brief Returns remaining cycles for the active request.
 */
int WebServer::getRemainingTime() const {
    return remaining_time;
}

/**
 * @brief Returns current request reference.
 */
const Request& WebServer::getCurrentRequest() const {
    return current_request;
}
