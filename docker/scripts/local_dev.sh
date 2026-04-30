#!/bin/bash
# Local development script: Build Dockerfile.dev and mount source code
# Run this from inside the dingus_labs repository
# Usage: ./local_dev.sh

set -e

print_help() {
    cat <<'EOF'
Local development script: Build Dockerfile.dev and start the container
This mounts the source code from the repo for live development.
Build artifacts stay in the container (ephemeral).

Usage:
  ./docker/scripts/local_dev.sh

Requirements:
  - Run from inside the dingus_labs repository
  - Docker installed

Workflow:
  1. Builds Dockerfile.dev (local image)
  2. Mounts repo/src → /dingus_ws/src in container
  3. Inside container, run: colcon build --symlink-install
  4. Edit code on host, rebuild as needed

EOF
}

if [[ "$1" == "-h" || "$1" == "--help" ]]; then
    print_help
    exit 0
fi

# Find repo root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
DOCKERFILE_BASE="$REPO_ROOT/docker/Dockerfile.base"
DOCKERFILE_DEV="$REPO_ROOT/docker/Dockerfile.dev"

if [[ ! -f "$DOCKERFILE_BASE" ]]; then
    echo "❌ Dockerfile.base not found at: $DOCKERFILE_BASE"
    echo "   Make sure you run this script from inside the dingus_labs repository"
    exit 1
fi

if [[ ! -f "$DOCKERFILE_DEV" ]]; then
    echo "❌ Dockerfile.dev not found at: $DOCKERFILE_DEV"
    echo "   Make sure you run this script from inside the dingus_labs repository"
    exit 1
fi

BASE_IMAGE="dingus:base"
IMAGE_NAME="dingus_dev:local"
IMAGE_TAG="$IMAGE_NAME"
CONTAINER_NAME="dingus_ws_dev"

echo "===================================================="
echo "Development Mode: Building Local Images"
echo "===================================================="
echo
echo "Dockerfile.base: $DOCKERFILE_BASE"
echo "Dockerfile.dev:  $DOCKERFILE_DEV"
echo "Base image:      $BASE_IMAGE"
echo "Dev image:       $IMAGE_TAG"
echo "Repository:      $REPO_ROOT"
echo "Container:       $CONTAINER_NAME"
echo

# Build base image first
echo "Building Dockerfile.base..."
docker build -f "$DOCKERFILE_BASE" -t "$BASE_IMAGE" "$REPO_ROOT/docker"
echo "✅ Base image built successfully"
echo

# Build local development image
echo "Building Dockerfile.dev..."
docker build -f "$DOCKERFILE_DEV" -t "$IMAGE_TAG" "$REPO_ROOT/docker"
echo "✅ Image built successfully"
echo

echo "===================================================="
echo "Deploying Development Container"
echo "===================================================="
echo
echo "Mounting repository source: $REPO_ROOT/src → /dingus_ws/src"
echo "Build artifacts will be ephemeral in the container"
echo "Run 'colcon build --symlink-install' inside the container"
echo

# Stop any running dev container
if docker ps -a --format '{{.Names}}' | grep -q "^${CONTAINER_NAME}$"; then
    echo "Stopping existing container..."
    docker stop "$CONTAINER_NAME" 2>/dev/null || true
    docker rm "$CONTAINER_NAME" 2>/dev/null || true
    echo "Existing container stopped and removed"
    echo
fi

# Start development container
echo "Starting development container..."
echo "Inside container:"
echo "  $ cd /dingus_ws"
echo "  $ colcon build --symlink-install"
echo
docker run -it \
    --privileged \
    --name "$CONTAINER_NAME" \
    --net=host \
    -v /dev:/dev \
    -v "$REPO_ROOT/src:/dingus_ws/src" \
    "$IMAGE_TAG"

# Note: This command blocks until container is exited
