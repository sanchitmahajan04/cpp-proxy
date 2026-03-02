# Test Examples

Quick test scripts to validate proxy functionality.

## Prerequisites

```bash
cd /path/to/cpp-proxy
mkdir -p build && cd build
cmake ..
make
```

Then in one terminal, run the proxy:
```bash
./epoll_proxy_bin
# or
./thread_proxy_bin
```

## Run Tests

From another terminal, run any of:

```bash
# Single request test
bash examples/test_single_request.sh

# Blocked host test (expects 403)
bash examples/test_blocked_host.sh

# Concurrent requests test (5 in parallel)
bash examples/test_concurrent.sh

# Raw request without Host header
bash examples/test_no_host_header.sh
```

## Check Logs

After tests, inspect the proxy log:

```bash
cat proxy.log
# or watch in real-time
tail -f proxy.log
```

Expected format:
```
2026-03-02 15:39:00 | 127.0.0.1 | example.com | OK | 12345 bytes | 150 ms
2026-03-02 15:39:01 | 127.0.0.1 | facebook.com | BLOCKED | 0 bytes | 0 ms
```

## What Each Test Does

| Test | Purpose | Expected Result |
|------|---------|-----------------|
| `test_single_request.sh` | Basic functionality | curl downloads HTML from example.com through proxy |
| `test_blocked_host.sh` | Filtering works | HTTP 403 Forbidden returned |
| `test_concurrent.sh` | Epoll scalability | 5 requests handled simultaneously, all logged |
| `test_no_host_header.sh` | Error handling | Connection closed or error (no valid Host header) |
