/**
 * @file Request.h
 * @brief Defines the Request struct used by the load balancer simulation.
 */
#ifndef REQUEST_H
#define REQUEST_H

#include <string>

/**
 * @brief Represents one incoming job handled by the load balancer.
 */
struct Request {
    /** Source IP address of the request. */
    std::string ip_in;
    /** Destination IP address for the response. */
    std::string ip_out;
    /** Number of clock cycles required to finish the request. */
    int time_required;
    /** Job type: 'P' for processing, 'S' for streaming. */
    char job_type;

    /**
     * @brief Creates a default request.
     */
    Request();
    /**
     * @brief Creates a request with all fields populated.
     * @param in Source IP.
     * @param out Destination IP.
     * @param t Processing time in cycles.
     * @param type Request type ('P' or 'S').
     */
    Request(const std::string& in, const std::string& out, int t, char type);
};

#endif
