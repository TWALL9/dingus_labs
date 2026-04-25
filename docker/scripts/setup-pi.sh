#!/bin/bash
# One-time setup script for Raspberry Pi
# Run this once to configure Docker and authentication for pulling images from GHCR

set -e

echo "===================================================="
echo "Dingus ROS Development - Raspberry Pi Setup"
echo "===================================================="
echo

# Check if running on ARM64
ARCH=$(uname -m)
if [[ "$ARCH" != "aarch64" ]]; then
    echo "⚠️  Warning: This script is optimized for ARM64 (aarch64)"
    echo "   Detected architecture: $ARCH"
    echo "   The image may not run correctly on this system"
    echo
fi

# Check if Docker is installed
if ! command -v docker &> /dev/null; then
    echo "❌ Docker is not installed. Installing Docker..."
    curl -fsSL https://get.docker.com -o get-docker.sh
    sudo sh get-docker.sh
    sudo usermod -aG docker "$USER"
    echo "✅ Docker installed. You may need to log out and back in for group changes to take effect."
else
    echo "✅ Docker is installed"
fi

echo

# Check if user can run docker without sudo
if ! docker ps &> /dev/null; then
    echo "⚠️  Warning: You don't have permission to run docker commands without sudo"
    echo "   Adding current user to docker group..."
    sudo usermod -aG docker "$USER"
    echo "   Please log out and back in for changes to take effect"
    echo
fi

# GitHub Container Registry (GHCR) authentication
echo "Configuring GitHub Container Registry (GHCR) authentication..."
echo "You'll need:"
echo "  1. Your GitHub username"
echo "  2. A Personal Access Token (PAT) with 'read:packages' permission"
echo "     Create one at: https://github.com/settings/tokens"
echo

read -p "GitHub username: " GITHUB_USER
read -sp "GitHub PAT (won't be displayed): " GITHUB_PAT
echo

echo "Logging in to GHCR..."
echo "$GITHUB_PAT" | docker login ghcr.io -u "$GITHUB_USER" --password-stdin

if [ $? -eq 0 ]; then
    echo "✅ Successfully authenticated with GHCR"
else
    echo "❌ Failed to authenticate with GHCR"
    exit 1
fi

echo

# Create deployment scripts directory
SCRIPT_DIR="$HOME/dingus_deploy"
mkdir -p "$SCRIPT_DIR"
echo "Created deployment directory: $SCRIPT_DIR"

# Create workspace directory for bind mounting
WS_DIR="$HOME/dingus_ws"
mkdir -p "$WS_DIR"
echo "Created workspace directory: $WS_DIR"

echo

# Display next steps
echo "===================================================="
echo "✅ Setup complete!"
echo "===================================================="
echo
echo "Next steps:"
echo "  1. Copy deployment scripts to your home directory:"
echo "     $ cp docker/scripts/*.sh ~/"
echo "     $ chmod +x ~/{deploy,stop,logs}.sh"
echo
echo "  2. Deploy the latest image:"
echo "     $ ~/deploy.sh"
echo
echo "  3. Inside the container, your workspace is mounted at /root/ros2_ws"
echo "     (maps to $WS_DIR on the Pi)"
echo
echo "Quick commands:"
echo "  Deploy latest:  ~/deploy.sh"
echo "  Stop container: ~/stop.sh"
echo "  View logs:      ~/logs.sh"
echo
