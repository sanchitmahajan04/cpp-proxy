# cpp-proxy

A C++ HTTP proxy implementation showcasing two concurrency models: **epoll-based** (async multiplexing, scalable) and **thread-per-connection** (simple, blocking).

## Features

- **Request filtering**: block access to specified hosts
- **Structured logging**: per-request logs (client IP, host, status, bytes, duration)
- **Two implementations**:
  - `EpollProxy`: Epoll‑based proxy that multiplexes client and server sockets on a single thread, offering much higher concurrency than the thread-per-connection variant
  - `ThreadProxy`: Thread-per-connection model; simpler but higher resource overhead
- **Error handling**: graceful socket cleanup, connection timeout handling
- **HTTP Host header parsing**: extract target host and validate/filter

## Project Structure

```
src/
├── common/
│   ├── Logger.h/cpp      - thread-safe request logging
│   └── Utils.h/cpp       - host extraction, filtering, HTTP responses
├── epoll/
│   ├── EpollProxy.h/cpp  - event-driven proxy (scalable)
├── thread/
│   ├── ThreadProxy.h/cpp - thread-per-connection (simple reference)
├── main_epoll.cpp        - epoll proxy entry point
└── main_thread.cpp       - thread proxy entry point
CMakeLists.txt
README.md
```

## Build

### Prerequisites
- C++17 compiler (g++)
- CMake 3.5+
- Linux (uses epoll, requires POSIX sockets)

### Compile

```bash
mkdir -p build && cd build
cmake ..
make
```

Outputs:
- `epoll_proxy_bin` - the main epoll-based proxy
- `thread_proxy_bin` - the thread-based reference implementation

## Run

### Epoll Proxy (Recommended for production-like testing)

```bash
./epoll_proxy_bin
```

Listens on `127.0.0.1:8080`. Logs to `proxy.log`.

### Thread Proxy (Simple reference)

```bash
./thread_proxy_bin
```

Same interface; for comparison/testing.

## Usage & Testing

### Quick Test with curl

Forward a request to `example.com` through the proxy:

```bash
curl -v -x http://127.0.0.1:8080 http://example.com/
```

Expected output:
- Proxy prints: `New client: 127.0.0.1` and `[debug] extracted host: 'example.com'`
- curl receives HTML from example.com
- `proxy.log` records: `2026-03-02 15:39:00 | 127.0.0.1 | example.com | OK | 12345 bytes | 150 ms`

### Test Multiple Concurrent Requests (epoll)

```bash
# Terminal 1: run proxy
./epoll_proxy_bin

# Terminal 2: send requests in parallel
for i in {1..5}; do
    curl -s -x http://127.0.0.1:8080 http://example.com/ > /dev/null &
done
wait
tail proxy.log
```

### Test Blocked Host

The proxy blocks: `facebook.com`, `instagram.com`, `youtube.com`

```bash
curl -v -x http://127.0.0.1:8080 http://facebook.com/
```

Expected: HTTP 403 Forbidden response, log entry with status `BLOCKED`.

### Inspect Logs

```bash
tail -f proxy.log
```

Format: `timestamp | client_ip | host | status | bytes_sent | duration_ms`

## Design Notes

### Epoll vs Thread

| Aspect | Epoll | Thread |
|--------|-------|--------|
| Scalability | High (thousands of connections) | Low (limited by threads) |
| CPU Overhead | Low (no context switch per connection) | Higher (thread scheduling) |
| Memory | Minimal | ~1-2 MB per thread |
| Latency | Predictable | Affected by thread pool |
| Code Complexity | Higher | Lower |
| Use Case | Production proxies, load balancers | Simple demos, dev testing |

**Recommendation**: Use `EpollProxy` for real workloads; use `ThreadProxy` as a reference or teaching tool.

### Key Implementation Details

**EpollProxy**:
- Uses epoll for readiness notifications on client and server sockets, allowing a single thread to manage hundreds or thousands of connections
- Proper socket-pair mapping with bidirectional logging
- Detects and cleans up socket errors via `EPOLLERR`/`EPOLLHUP`

**ThreadProxy**:
- Blocking I/O for simplicity
- One thread per client (spawned with `std::thread`, detached)
- Synchronous DNS and connect calls
- Minimal overhead for small request counts

## Future Improvements

- [ ] HTTP/1.1 keep-alive connection pooling
- [ ] HTTPS/CONNECT tunnel support
- [ ] Connection timeouts and graceful shutdown
- [ ] Per-connection rate limiting and metrics
- [ ] Configurable blocked hosts (JSON config file)
- [ ] Performance benchmarks (throughput, latency percentiles)
- [ ] Docker image for easy deployment
- [ ] Unit tests and stress tests

## License

MIT

## Author

Built as a learning project to demonstrate epoll-based async I/O vs thread-based concurrency in C++.
