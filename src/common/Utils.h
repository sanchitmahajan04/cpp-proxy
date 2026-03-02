#pragma once
#include <string>

std::string extract_host(const std::string &request);
bool is_blocked(const std::string &host);
void send_forbidden(int client_socket);
