/**
 * @file WebServer.h
 * @brief Declares the WebServer class used to process queued requests.
 */
#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include "Request.h"

/**
 * @brief Simulates one server that processes at most one request at a time.
 */
class WebServer {
private:
    /** True when this server has an active request. */
    bool busy;
    /** Remaining clock cycles for the active request. */
    int remaining_time;
    /** Request currently assigned to this server. */
    Request current_request;

public:
    /**
     * @brief Creates an idle server.
     */
    WebServer();

    /**
     * @brief Advances processing by one clock cycle.
     * @return True if a request completed on this tick.
     */
    bool tick();
    /**
     * @brief Assigns a request to this server.
     * @param request Request to process.
     */
    void assignRequest(const Request& request);
    /**
     * @brief Returns whether the server is currently processing a request.
     * @return True if busy.
     */
    bool isBusy() const;
    /**
     * @brief Gets remaining cycles for the active request.
     * @return Remaining cycles.
     */
    int getRemainingTime() const;
    /**
     * @brief Gets the currently assigned request.
     * @return Constant reference to active request.
     */
    const Request& getCurrentRequest() const;
};

#endif
