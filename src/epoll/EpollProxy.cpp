#include "EpollProxy.h"
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <errno.h>
#include <chrono>
#include <unordered_map>
#include "../common/Logger.h"
#include "../common/Utils.h"

EpollProxy::EpollProxy(int port) : port_(port) {}

EpollProxy::~EpollProxy() {}

void EpollProxy::run() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port_);

    bind(server_fd, (sockaddr*)&address, sizeof(address));
    listen(server_fd, 100);

    std::cout << "Proxy running on port " << port_ << std::endl;

    int epoll_fd = epoll_create1(0);

    epoll_event ev{}, events[1024];
    ev.events = EPOLLIN;
    ev.data.fd = server_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev);

    std::unordered_map<int,int> client_to_server;
    std::unordered_map<int,int> server_to_client;
    std::unordered_map<int,std::string> client_ip_map;
    std::unordered_map<int,std::chrono::high_resolution_clock::time_point> start_time_map;
    std::unordered_map<int,int> bytes_map;
    std::unordered_map<int,std::string> host_map;

    while (true) {
        int n = epoll_wait(epoll_fd, events, 1024, -1);

        for (int i = 0; i < n; i++) {
            int fd = events[i].data.fd;

            if (fd == server_fd) {
                sockaddr_in client_addr{};
                socklen_t len = sizeof(client_addr);
                int client_fd = accept(server_fd, (sockaddr*)&client_addr, &len);
                std::string ip = inet_ntoa(client_addr.sin_addr);
                std::cout << "New client: " << ip << std::endl;

                client_ip_map[client_fd] = ip;
                start_time_map[client_fd] = std::chrono::high_resolution_clock::now();
                bytes_map[client_fd] = 0;

                epoll_event client_ev{};
                client_ev.events = EPOLLIN;
                client_ev.data.fd = client_fd;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &client_ev);
            } else {
                char buffer[8192];
                memset(buffer, 0, sizeof(buffer));
                int bytes = recv(fd, buffer, sizeof(buffer), 0);

                if (bytes <= 0) {
                    // closing and logging
                    if (client_to_server.count(fd)) {
                        int srv = client_to_server[fd];
                        int client_fd = fd;
                        int total_bytes = 0;
                        if (bytes_map.count(client_fd)) total_bytes = bytes_map[client_fd];
                        long duration_ms = 0;
                        if (start_time_map.count(client_fd)) {
                            auto end = std::chrono::high_resolution_clock::now();
                            duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start_time_map[client_fd]).count();
                        }
                        std::string host = host_map.count(client_fd) ? host_map[client_fd] : std::string("-");
                        std::string client_ip = client_ip_map.count(client_fd) ? client_ip_map[client_fd] : std::string("-");
                        log_request(client_ip, host, "OK", total_bytes, duration_ms);

                        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, srv, nullptr);
                        close(srv);
                        server_to_client.erase(srv);
                        client_to_server.erase(client_fd);

                        bytes_map.erase(client_fd);
                        start_time_map.erase(client_fd);
                        host_map.erase(client_fd);
                        client_ip_map.erase(client_fd);
                    }

                    if (server_to_client.count(fd)) {
                        int client_fd = server_to_client[fd];
                        int total_bytes = 0;
                        if (bytes_map.count(client_fd)) total_bytes = bytes_map[client_fd];
                        long duration_ms = 0;
                        if (start_time_map.count(client_fd)) {
                            auto end = std::chrono::high_resolution_clock::now();
                            duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start_time_map[client_fd]).count();
                        }
                        std::string host = host_map.count(client_fd) ? host_map[client_fd] : std::string("-");
                        std::string client_ip = client_ip_map.count(client_fd) ? client_ip_map[client_fd] : std::string("-");
                        log_request(client_ip, host, "OK", total_bytes, duration_ms);

                        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, nullptr);
                        close(client_fd);
                        client_to_server.erase(client_fd);
                        server_to_client.erase(fd);

                        bytes_map.erase(client_fd);
                        start_time_map.erase(client_fd);
                        host_map.erase(client_fd);
                        client_ip_map.erase(client_fd);
                    }

                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
                    close(fd);
                    continue;
                }

                std::string data(buffer, bytes);

                // if fd is a server socket
                if (server_to_client.find(fd) != server_to_client.end()) {
                    int client_fd = server_to_client[fd];
                    send(client_fd, buffer, bytes, 0);
                    bytes_map[client_fd] += bytes;
                } else {
                    // client socket: first packet contains request
                    std::cout << "\n=== Incoming Request ===\n" << data << std::endl;
                    std::string host = extract_host(data);
                    host_map[fd] = host;

                    if (is_blocked(host)) {
                        send_forbidden(fd);
                        log_request(client_ip_map.count(fd) ? client_ip_map[fd] : std::string("-"), host, "BLOCKED", 0, 0);
                        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
                        close(fd);
                        continue;
                    }

                    addrinfo hints{}; addrinfo *res = nullptr;
                    hints.ai_family = AF_INET; hints.ai_socktype = SOCK_STREAM;
                    if (getaddrinfo(host.c_str(), "80", &hints, &res) != 0) {
                        std::cerr << "  [debug] getaddrinfo failed for '" << host << "'\n";
                        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
                        close(fd);
                        continue;
                    }

                    int server_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

                    if (connect(server_socket, res->ai_addr, res->ai_addrlen) < 0) {
                        std::cerr << "  [debug] connect to server failed: " << strerror(errno) << "\n";
                        freeaddrinfo(res);
                        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
                        close(fd);
                        close(server_socket);
                        continue;
                    }

                    // send initial request data synchronously
                    send(server_socket, data.c_str(), data.size(), 0);

                    client_to_server[fd] = server_socket;
                    server_to_client[server_socket] = fd;

                    epoll_event server_ev{};
                    server_ev.events = EPOLLIN;
                    server_ev.data.fd = server_socket;
                    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_socket, &server_ev);

                    freeaddrinfo(res);
                }
            }
        }
    }
}
