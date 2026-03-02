/**
 * @file Request.cpp
 * @brief Implements Request constructors.
 */
#include "Request.h"

/**
 * @brief Constructs a default request with placeholder values.
 */
Request::Request() : ip_in("0.0.0.0"), ip_out("0.0.0.0"), time_required(1), job_type('S') {}

/**
 * @brief Constructs a fully specified request.
 */
Request::Request(const std::string& in, const std::string& out, int t, char type)
    : ip_in(in), ip_out(out), time_required(t), job_type(type) {}
