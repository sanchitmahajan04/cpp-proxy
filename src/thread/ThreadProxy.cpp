#include "ThreadProxy.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include "../common/Logger.h"
#include "../common/Utils.h"

#define BUFFER_SIZE 8192

ThreadProxy::ThreadProxy(int port) : port_(port) {}

ThreadProxy::~ThreadProxy() {}

void ThreadProxy::handle_client(int client_socket, std::string client_ip) {
    auto start_time = std::chrono::high_resolution_clock::now();
    int total_bytes = 0;
    std::string status = "ALLOWED";
    char buffer[BUFFER_SIZE];

    memset(buffer, 0, BUFFER_SIZE);
    int bytes = read(client_socket, buffer, BUFFER_SIZE);

    if (bytes <= 0) {
        close(client_socket);
        return;
    }

    std::string request(buffer, bytes);

    std::cout << "\n=== Incoming Request ===\n" << request << std::endl;

    std::string host = extract_host(request);

    if (host.empty()) {
        std::cerr << "No host found\n";
        close(client_socket);
        return;
    }

    // filtering
    if (is_blocked(host)) {
        std::cout << "Blocked access to: " << host << std::endl;
        send_forbidden(client_socket);
        status = "BLOCKED";
        log_request(client_ip, host, status, 0, 0);
        close(client_socket);
        return;
    }

    // resolve host
    struct addrinfo hints{}, *res;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(host.c_str(), "80", &hints, &res) != 0) {
        perror("DNS resolution failed");
        close(client_socket);
        return;
    }

    int server_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    if (connect(server_socket, res->ai_addr, res->ai_addrlen) < 0) {
        perror("connect failed");
        freeaddrinfo(res);
        close(client_socket);
        return;
    }

    send(server_socket, request.c_str(), request.size(), 0);

    // relay response
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int recv_bytes = recv(server_socket, buffer, BUFFER_SIZE, 0);

        if (recv_bytes <= 0) break;

        send(client_socket, buffer, recv_bytes, 0);
        total_bytes += recv_bytes;
    }

    std::cout << "Response relayed to client: " << host << std::endl;
    auto end_time = std::chrono::high_resolution_clock::now();
    long duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                        end_time - start_time).count();

    log_request(client_ip, host, status, total_bytes, duration);

    freeaddrinfo(res);
    close(server_socket);
    close(client_socket);
}

void ThreadProxy::run() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port_);

    bind(server_fd, (sockaddr*)&address, sizeof(address));
    listen(server_fd, 10);

    std::cout << "Proxy running on port " << port_ << std::endl;

    while (true) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        int client_socket = accept(server_fd, (sockaddr*)&client_addr, &client_len);

        if (client_socket < 0) {
            perror("accept failed");
            continue;
        }

        std::cout << "New client connected\n";
        std::string client_ip = inet_ntoa(client_addr.sin_addr);

        // create new thread for each client
        std::thread t(handle_client, client_socket, client_ip);
        t.detach();
    }
}
