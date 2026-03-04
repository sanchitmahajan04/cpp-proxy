# cpp-proxy

A lightweight HTTP proxy written in C++. I built two versions: one using **epoll** (fast, handles tons of connections) and one using **threads** (simple, easy to understand). Both support request filtering and detailed logging.

## Features

- Block access to certain hosts (e.g., Facebook, YouTube)
- Logs each request with timing and byte counts
- **Two versions**:
  - **Epoll**: Single-threaded, scales to thousands of connections
  - **Thread-based**: One thread per connection, simple reference implementation
- Extracts target host from HTTP requests and routes/filters accordingly

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

## Setup & Build

You'll need C++17, CMake, and Linux (uses epoll).

```bash
mkdir -p build && cd build
cmake ..
make
```

This builds `epoll_proxy_bin` and `thread_proxy_bin`.

## Running the Proxy

Start the epoll version:
```bash
./epoll_proxy_bin
```

It listens on `127.0.0.1:8080` and logs to `proxy.log`.

## Testing

### Simple Request
Forward a request through the proxy:
```bash
curl -v -x http://127.0.0.1:8080 http://example.com/
```

You'll see the proxy print the incoming request and log the result.

### Blocked Hosts
The proxy blocks `facebook.com`, `instagram.com`, `youtube.com`. Try:
```bash
curl -x http://127.0.0.1:8080 http://facebook.com/
```

You get a 403 response and the log shows `BLOCKED`.

### Multiple Concurrent Requests
```bash
for i in {1..5}; do
    curl -s -x http://127.0.0.1:8080 http://example.com/ &
done
wait
tail proxy.log
```

The epoll version handles these easily on a single thread.

### View Logs
```bash
tail -f proxy.log
```

Each line: `timestamp | client_ip | host | status | bytes | ms`

## Epoll vs Threads

| | Epoll | Thread |
|---|---|---|
| Scalability | Thousands of connections | Limited by thread count |
| CPU | Efficient | Higher overhead |
| Memory | Low | ~1-2 MB per thread |
| Code | More complex | Simple |
| Best for | Production | Learning/small loads |

The epoll version is what we'd use in real deployments. The thread version is good for understanding how proxies work.

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
