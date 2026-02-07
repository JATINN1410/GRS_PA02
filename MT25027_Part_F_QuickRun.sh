#!/bin/bash
# MT25027
# MT25027_Part_F_QuickRun.sh
# Automated single-run script for server and client

if [ "$#" -ne 4 ]; then
    echo "Usage: sudo $0 <A1|A2|A3> <msg_size> <threads> <duration>"
    echo "Example: sudo $0 A1 4096 1 5"
    exit 1
fi

IMPL=$1
SIZE=$2
THREADS=$3
DURATION=$4

SERVER_BIN="./MT25027_Part_${IMPL}_Server"
CLIENT_BIN="./MT25027_Part_${IMPL}_Client"

# Check if binaries exist
if [ ! -f "$SERVER_BIN" ] || [ ! -f "$CLIENT_BIN" ]; then
    echo "Binaries not found. Running make..."
    make >/dev/null 2>&1
fi

# Network namespace configuration
NS_SERVER="ns_server"
NS_CLIENT="ns_client"
VETH_SERVER="veth_server"
VETH_CLIENT="veth_client"
SERVER_IP="10.0.0.1"
CLIENT_IP="10.0.0.2"

setup_namespaces() {
    # Only setup if they don't exist
    if ! ip netns list | grep -q "$NS_SERVER"; then
        echo "Setting up network namespaces..."
        ip netns add $NS_SERVER
        ip netns add $NS_CLIENT
        ip link add $VETH_SERVER type veth peer name $VETH_CLIENT
        ip link set $VETH_SERVER netns $NS_SERVER
        ip link set $VETH_CLIENT netns $NS_CLIENT
        ip netns exec $NS_SERVER ip addr add ${SERVER_IP}/24 dev $VETH_SERVER
        ip netns exec $NS_CLIENT ip addr add ${CLIENT_IP}/24 dev $VETH_CLIENT
        ip netns exec $NS_SERVER ip link set $VETH_SERVER up
        ip netns exec $NS_CLIENT ip link set $VETH_CLIENT up
        ip netns exec $NS_SERVER ip link set lo up
        ip netns exec $NS_CLIENT ip link set lo up
    fi
}

cleanup_processes() {
    { ip netns exec $NS_SERVER pkill -9 -f "_Server" || true; } >/dev/null 2>&1
    { ip netns exec $NS_CLIENT pkill -9 -f "_Client" || true; } >/dev/null 2>&1
}

setup_namespaces
cleanup_processes

echo "Starting Server ($IMPL) in namespace $NS_SERVER..."
ip netns exec $NS_SERVER $SERVER_BIN $SIZE $THREADS $DURATION >/dev/null 2>&1 &
SERVER_PID=$!
disown $SERVER_PID 2>/dev/null

sleep 2

echo "Running Client ($IMPL) in namespace $NS_CLIENT..."
echo "------------------------------------------------"
ip netns exec $NS_CLIENT $CLIENT_BIN $SERVER_IP $SIZE $THREADS $DURATION
echo "------------------------------------------------"

sleep 1
echo "Cleaning up..."
{ kill -9 $SERVER_PID || true; } >/dev/null 2>&1
wait $SERVER_PID >/dev/null 2>&1 || true

echo "Done."
