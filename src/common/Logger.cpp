#include "Logger.h"
#include <fstream>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <sstream>

static std::mutex g_log_mutex;
static std::ofstream g_log_file;

std::string current_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm buf;
    localtime_r(&in_time_t, &buf);
    std::ostringstream ss;
    ss << std::put_time(&buf, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

void init_logger(const std::string &path) {
    std::lock_guard<std::mutex> lock(g_log_mutex);
    if (!g_log_file.is_open()) g_log_file.open(path, std::ios::app);
}

void log_request(const std::string &client_ip,
                 const std::string &host,
                 const std::string &status,
                 int bytes_sent,
                 long time_taken_ms) {
    std::lock_guard<std::mutex> lock(g_log_mutex);
    if (!g_log_file.is_open()) g_log_file.open("proxy.log", std::ios::app);
    g_log_file << current_timestamp()
               << " | " << client_ip
               << " | " << host
               << " | " << status
               << " | " << bytes_sent << " bytes"
               << " | " << time_taken_ms << " ms"
               << std::endl;
}
