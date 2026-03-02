#!/bin/bash
# Test a single request through the proxy

set -e

PROXY_HOST="127.0.0.1"
PROXY_PORT="8080"
TARGET_HOST="example.com"

echo "[test] Sending single request to $TARGET_HOST through proxy at $PROXY_HOST:$PROXY_PORT"
curl -v -x http://$PROXY_HOST:$PROXY_PORT http://$TARGET_HOST/ 2>&1 | head -20

echo ""
echo "[test] Request complete. Check proxy.log for log entry."
