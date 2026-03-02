#!/bin/bash
# Test concurrent requests to measure epoll scalability

set -e

PROXY_HOST="127.0.0.1"
PROXY_PORT="8080"
NUM_REQUESTS=5
TARGET_HOST="example.com"

echo "[test] Sending $NUM_REQUESTS concurrent requests through proxy"
echo "This tests proxy scalability and simultaneous connection handling."
echo ""

# Send requests in parallel
for i in $(seq 1 $NUM_REQUESTS); do
    echo "[request $i] Sending..."
    curl -s -x http://$PROXY_HOST:$PROXY_PORT http://$TARGET_HOST/ > /tmp/response_$i.txt &
done

# Wait for all background jobs
echo "Waiting for all requests to complete..."
wait

echo ""
echo "[test] All $NUM_REQUESTS requests completed."
echo ""
echo "Log summary (last $NUM_REQUESTS lines):"
tail -$NUM_REQUESTS proxy.log
