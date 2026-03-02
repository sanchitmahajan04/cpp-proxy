#pragma once

#include <string>

void init_logger(const std::string &path = "proxy.log");
void log_request(const std::string &client_ip,
                 const std::string &host,
                 const std::string &status,
                 int bytes_sent,
                 long time_taken_ms);
