#!/bin/bash
# Stop the running Dingus container

CONTAINER_NAME="dingus_ws"

echo "Stopping container: $CONTAINER_NAME"

if docker ps --format '{{.Names}}' | grep -q "^${CONTAINER_NAME}$"; then
    docker stop "$CONTAINER_NAME"
    echo "✅ Container stopped"
else
    echo "ℹ️  Container '$CONTAINER_NAME' is not running"
fi
