#!/bin/bash

set -e

print_help() {
    cat <<'EOF'
Deploy script: Pull a specific GHCR image tag and start the container
Usage:
  ./deploy.sh [IMAGE_PATH] [TAG]
  ./deploy.sh [TAG]
Examples:
  ./deploy.sh ghcr.io/twall9/dingus_labs latest
  ./deploy.sh ghcr.io/twall9/dingus_labs docker-registry
  ./deploy.sh docker-registry
  ./deploy.sh
EOF
}

if [[ "$1" == "-h" || "$1" == "--help" ]]; then
    print_help
    exit 0
fi

DEFAULT_REGISTRY_PATH="ghcr.io/twall9/dingus_labs"
ARG1="${1:-}"
ARG2="${2:-}"

if [[ -n "$ARG1" && -z "$ARG2" && "$ARG1" != */* ]]; then
    REGISTRY_PATH="$DEFAULT_REGISTRY_PATH"
    TAG="$ARG1"
else
    REGISTRY_PATH="${ARG1:-$DEFAULT_REGISTRY_PATH}"
    TAG="${ARG2:-latest}"
fi

IMAGE_TAG="$REGISTRY_PATH:$TAG"
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

# Pull image
echo "Pulling GHCR image: $IMAGE_TAG"
if docker pull "$IMAGE_TAG"; then
    echo "Image pulled successfully"
else
    echo "Failed to pull image: $IMAGE_TAG"
    echo "Check your GHCR authentication and image name."
    exit 1
fi

echo

# Stop any running container with same name
if docker ps -a --format '{{.Names}}' | grep -q "^${CONTAINER_NAME}$"; then
    echo "Stopping existing container..."
    docker stop "$CONTAINER_NAME" 2>/dev/null || true
    docker rm "$CONTAINER_NAME" 2>/dev/null || true
    echo "Existing container stopped and removed"
fi

echo

# Start new container
echo "Starting new container..."
docker run -it \
    --privileged \
    --name "$CONTAINER_NAME" \
    --net=host \
    -v /dev:/dev \
    -v "$WS_HOST_DIR:/root/ros2_ws" \
    "$IMAGE_TAG"

# Note: This command blocks until container is exited
