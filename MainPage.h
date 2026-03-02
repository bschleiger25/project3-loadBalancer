/**
 * @file MainPage.h
 * @brief Doxygen main page content for Project 3 documentation.
 */

/**
 * @mainpage Project 3 Load Balancer Documentation
 *
 * @section overview Overview
 * This project simulates a load balancer for incoming web requests.
 * The simulator models request flow, queueing, server processing, and
 * autoscaling behavior over clock cycles.
 *
 * @section components Core Components
 * - Request: stores source IP, destination IP, processing time, and job type.
 * - RequestQueue: FIFO queue wrapper around std::queue<Request>.
 * - WebServer: processes one request at a time and tracks remaining cycles.
 * - LoadBalancer: manages queue, servers, scaling, request generation, and logs.
 *
 * @section behaviors Required Behaviors Implemented
 * - Random request generation during runtime.
 * - Dynamic server scaling using queue thresholds and cooldown delay.
 * - IP prefix blocking to simulate firewall or DoS protection.
 * - Color-coded event logging for key simulation actions.
 * - End-of-run summary statistics.
 * - Configurable parameters loaded from config.txt.
 *
 * @section run Running The Program
 * 1. Build the project.
 * 2. Run the executable.
 * 3. Enter server and cycle counts (or press Enter for config defaults).
 * 4. Review logs and final summary output.
 */
