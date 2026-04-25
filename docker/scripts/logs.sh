#!/bin/bash
# View logs from the Dingus container

CONTAINER_NAME="dingus_ws"

echo "Fetching logs for container: $CONTAINER_NAME"
echo

docker logs -f "$CONTAINER_NAME"
