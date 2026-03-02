#include "Utils.h"
#include <vector>
#include <string>
#include <unistd.h>
#include <sys/socket.h>

static std::vector<std::string> g_blocked_hosts = {
    "facebook.com",
    "instagram.com",
    "youtube.com"
};

std::string extract_host(const std::string &request) {
    std::string host_key = "Host: ";
    size_t pos = request.find(host_key);
    if (pos == std::string::npos) return "";
    size_t end = request.find("\r\n", pos);
    return request.substr(pos + host_key.length(), end - (pos + host_key.length()));
}

bool is_blocked(const std::string &host) {
    for (const auto &b : g_blocked_hosts) {
        if (host.find(b) != std::string::npos) return true;
    }
    return false;
}

void send_forbidden(int client_socket) {
    std::string response =
        "HTTP/1.1 403 Forbidden\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 18\r\n\r\n"
        "Access Blocked!\n";
    send(client_socket, response.c_str(), response.size(), 0);
}
