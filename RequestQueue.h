/**
 * @file RequestQueue.h
 * @brief Declares a queue wrapper for Request objects.
 */
#ifndef REQUEST_QUEUE_H
#define REQUEST_QUEUE_H

#include <cstddef>
#include <queue>

#include "Request.h"

/**
 * @brief Thin wrapper around std::queue<Request> for simulation queue operations.
 */
class RequestQueue {
private:
    /** Internal FIFO queue of requests. */
    std::queue<Request> q;

public:
    /**
     * @brief Adds a request to the back of the queue.
     * @param request Request to enqueue.
     */
    void enqueue(const Request& request);
    /**
     * @brief Removes and returns the front request.
     * @return Front request.
     * @throws std::runtime_error if the queue is empty.
     */
    Request dequeue();
    /**
     * @brief Returns the number of queued requests.
     * @return Queue size.
     */
    std::size_t size() const;
    /**
     * @brief Checks whether the queue is empty.
     * @return True if empty, false otherwise.
     */
    bool empty() const;
};

#endif
