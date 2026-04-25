#!/bin/bash
# Deploy script: Pull latest image from GHCR and start container
# Usage: ./deploy.sh [REGISTRY_PATH]
# Example: ./deploy.sh ghcr.io/username/dingus

set -e

# Configuration
REGISTRY_PATH="${1:-ghcr.io/twall9/dingus}"
IMAGE_TAG="$REGISTRY_PATH:latest"
CONTAINER_NAME="dingus_ws"
WS_HOST_DIR="${HOME}/dingus_ws"

echo "===================================================="
echo "Deploying Dingus Development Container"
echo "===================================================="
echo
echo "Image:      $IMAGE_TAG"
echo "Container:  $CONTAINER_NAME"
echo "Workspace:  $WS_HOST_DIR → /root/ros2_ws (in container)"
echo

# Ensure workspace directory exists
mkdir -p "$WS_HOST_DIR"

# Pull latest image
echo "📥 Pulling latest image from GHCR..."
if docker pull "$IMAGE_TAG"; then
    echo "✅ Image pulled successfully"
else
    echo "❌ Failed to pull image. Check your GHCR authentication and image name."
    exit 1
fi

echo

# Stop any running container with same name
if docker ps -a --format '{{.Names}}' | grep -q "^${CONTAINER_NAME}$"; then
    echo "🛑 Stopping existing container..."
    docker stop "$CONTAINER_NAME" 2>/dev/null || true
    docker rm "$CONTAINER_NAME" 2>/dev/null || true
    echo "✅ Existing container stopped and removed"
fi

echo

# Start new container
echo "🚀 Starting new container..."
docker run -it \
    --privileged \
    --name "$CONTAINER_NAME" \
    --net=host \
    -v /dev:/dev \
    -v "$WS_HOST_DIR:/root/ros2_ws" \
    "$IMAGE_TAG"

# Note: This command blocks until container is exited
