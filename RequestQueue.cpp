/**
 * @file RequestQueue.cpp
 * @brief Implements the RequestQueue wrapper methods.
 */
#include "RequestQueue.h"

#include <stdexcept>

/**
 * @brief Enqueues one request.
 */
void RequestQueue::enqueue(const Request& request) {
    q.push(request);
}

/**
 * @brief Dequeues one request from the front.
 */
Request RequestQueue::dequeue() {
    if (q.empty()) {
        throw std::runtime_error("RequestQueue::dequeue called on empty queue");
    }
    Request front = q.front();
    q.pop();
    return front;
}

/**
 * @brief Returns queue length.
 */
std::size_t RequestQueue::size() const {
    return q.size();
}

/**
 * @brief Returns whether the queue is empty.
 */
bool RequestQueue::empty() const {
    return q.empty();
}
