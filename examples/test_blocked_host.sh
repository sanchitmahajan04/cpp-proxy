#!/bin/bash
# Test that blocked hosts return 403

set -e

PROXY_HOST="127.0.0.1"
PROXY_PORT="8080"
BLOCKED_HOST="facebook.com"

echo "[test] Testing access to blocked host: $BLOCKED_HOST"
echo "Expected: HTTP 403 Forbidden"
echo ""

response=$(curl -s -w "%{http_code}" -x http://$PROXY_HOST:$PROXY_PORT http://$BLOCKED_HOST/ -o /tmp/response.txt 2>&1)

if [ "$response" -eq 403 ]; then
    echo "[PASS] Got expected 403 Forbidden"
    echo "Response body:"
    cat /tmp/response.txt
else
    echo "[FAIL] Expected 403, got $response"
    exit 1
fi

echo ""
echo "Check proxy.log for BLOCKED status."
