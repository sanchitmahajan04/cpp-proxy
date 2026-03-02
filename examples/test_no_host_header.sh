#!/bin/bash
# Test request with missing Host header

set -e

PROXY_HOST="127.0.0.1"
PROXY_PORT="8080"

echo "[test] Sending request without Host header (raw HTTP)"
echo ""

# Send raw HTTP without Host header
nc -z $PROXY_HOST $PROXY_PORT 2>/dev/null && {
    echo "[test] Proxy is listening. Sending raw request..."
    {
        echo -e "GET / HTTP/1.1\r"
        echo -e "Connection: close\r"
        echo -e "\r"
        sleep 1
    } | nc $PROXY_HOST $PROXY_PORT | head -5
} || {
    echo "[SKIP] Proxy not listening (use 'curl' test if nc unavailable)"
}

echo ""
echo "Proxy should close connection or return error (no valid Host)."
