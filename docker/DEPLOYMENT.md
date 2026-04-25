# Deploying Dingus ROS to Raspberry Pi

This guide explains how to deploy the Dingus ROS development environment to a Raspberry Pi 4/5 using Docker and GHCR (GitHub Container Registry).

## Prerequisites

### On Your Local Machine
- GitHub repository with this code pushed
- Docker installed (for local testing)
- GitHub account

### On Raspberry Pi
- Raspberry Pi 4 or 5 (ARM64 architecture)
- Raspberry Pi OS (32-bit or 64-bit, though 64-bit recommended)
- Internet connection
- SSH access enabled
- ~10GB free disk space (for image + workspace)

## Architecture

```
┌─────────────────────┐
│   Your Computer     │
│  (Push to GitHub)   │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────────────────────┐
│  GitHub Actions                     │
│  (Builds ARM64 image on push)       │
└──────────┬────────────────────────────┘
           │
           ▼
┌─────────────────────────────────────┐
│  GHCR (ghcr.io)                     │
│  dingus:latest                      │
└──────────┬────────────────────────────┘
           │
           ▼
┌─────────────────────────────────────┐
│  Raspberry Pi                       │
│  $ ./deploy.sh                      │
│  (Pulls latest, starts container)   │
└─────────────────────────────────────┘
```

## Setup Steps

### Step 1: Prepare Your GitHub Repository

Ensure your code is pushed to GitHub with:
- `docker/Dockerfile` (updated for ARM64, Gazebo removed)
- `docker/scripts/` (deployment scripts)
- `.github/workflows/docker-build.yml` (CI/CD workflow)

The workflow automatically triggers on push to `main` branch.

### Step 2: Verify GitHub Actions Build

1. Push your changes to `main` branch:
   ```bash
   git add .
   git commit -m "Add Raspberry Pi deployment infrastructure"
   git push origin main
   ```

2. Watch the build in GitHub:
   - Go to your repository → **Actions** tab
   - Click the latest workflow run
   - Wait for "Build and push Docker image" to complete (5-15 minutes)
   - Verify it shows ✅ passed

3. Verify the image was pushed to GHCR:
   ```bash
   # View packages on GitHub
   # Settings → Packages and registries → Container registry
   # Or pull locally to test:
   docker login ghcr.io -u YOUR_USERNAME
   docker pull ghcr.io/YOUR_USERNAME/dingus_labs:latest
   ```

### Step 3: One-Time Setup on Raspberry Pi

SSH into your Pi and run the setup script:

```bash
ssh pi@<your-pi-ip>

# Clone this repo (or download just the setup script)
git clone https://github.com/TWALL9/dingus_labs.git
cd dingus_labs

# Run setup (this installs Docker and configures GHCR auth)
bash docker/scripts/setup-pi.sh
```

The script will:
- ✅ Check/install Docker
- ✅ Verify ARM64 architecture
- ✅ Prompt for GitHub credentials (for GHCR login)
- ✅ Create workspace directories
- ✅ Display next steps

**Important:** You'll need a GitHub Personal Access Token (PAT):
1. Go to https://github.com/settings/tokens
2. Click "Generate new token" → "Generate new token (classic)"
3. Give it a name (e.g., "Pi GHCR Access")
4. Select scope: `read:packages`
5. Click "Generate token"
6. Copy and paste it into the setup script when prompted

### Step 4: Deploy and Start

Still on your Pi, copy and run the deploy script:

```bash
# Copy scripts to home directory
cp docker/scripts/deploy.sh ~/
cp docker/scripts/stop.sh ~/
cp docker/scripts/logs.sh ~/
chmod +x ~/{deploy,stop,logs}.sh

# Deploy the latest image (pulls from GHCR and starts container)
~/deploy.sh
```

The script will:
1. Pull the latest image from GHCR: `ghcr.io/YOUR_USERNAME/dingus_labs:latest`
2. Stop any existing container
3. Start a new container with:
   - Privileged access (for hardware)
   - `/dev` passthrough (for USB devices)
   - Host networking (for ROS discovery)
   - Workspace bind mount: `~/dingus_ws` → `/root/ros2_ws` (in container)

### Step 5: Verify It Works

Inside the container:

```bash
# Inside the container shell
$ ros2 node list
$ ros2 pkg list | grep dingus
$ ros2 launch dingus_core rplidar.launch.py  # (if you have hardware)
```

Your workspace at `~/dingus_ws` on the Pi is now synced with `/root/ros2_ws` in the container.

## Daily Workflow

### Get Latest Updates

Whenever you push changes to GitHub's `main` branch:

```bash
# On Pi (or any machine)
~/deploy.sh
```

This automatically:
- Pulls the latest image (built by GitHub Actions)
- Stops the old container
- Starts a new one with your latest code

### Useful Commands

```bash
# Stop container gracefully
~/stop.sh

# View container logs
~/logs.sh

# Attach to running container from another terminal
ssh pi@<ip>
docker attach dingus_ws

# Execute command in container without interactive shell
docker exec dingus_ws ros2 node list

# Copy files between Pi and container
docker cp dingus_ws:/root/ros2_ws/data ~/backup_data
docker cp ~/local_file dingus_ws:/root/ros2_ws/

# Check image size
docker images ghcr.io/YOUR_USERNAME/dingus_labs
```

## Troubleshooting

### Image Pull Fails ("Unauthorized")
- Verify GHCR login: `docker login ghcr.io`
- Check PAT has `read:packages` permission
- Confirm you're using your GitHub username, not email

### Container Won't Start
- Check container logs: `~/logs.sh`
- Verify Docker is running: `docker ps`
- Ensure privileged mode is enabled (should be in deploy.sh)

### Hardware Not Accessible (USB devices)
- Verify `/dev` is mounted: `ls /dev/ttyUSB*` in container
- Check udev rules: `cat /etc/udev/rules.d/80-st-board.rules`
- Reload udev: `sudo udevadm control --reload && sudo udevadm trigger`

### Image Build Failed in GitHub Actions
- Check GitHub Actions logs: Repository → Actions tab
- Common issues:
  - Submodule access (ensure submodules are public or use deploy key)
  - Build timeout (increase if needed in workflow file)
  - Out of disk space on GitHub runner

### Workspace Persistence Issues
- Workspace is mounted at `~/dingus_ws` on Pi
- Changes in container are persisted on host
- If container restarts, workspace is preserved

## Advanced: Custom Image Name

If using a different GitHub org or repository name:

```bash
# Update the REGISTRY_PATH in deploy.sh
~/deploy.sh ghcr.io/YOUR_ORG/YOUR_IMAGE_NAME
```

Or edit the hardcoded value at the top of `deploy.sh`.

## Size Optimization

Current image size: ~4-5GB (ARM64)
- Includes: ROS 2 Jazzy, sensors, navigation, control stacks
- Excludes: Gazebo (reduces by ~1.5GB), development headers (optional)

To further optimize:
- Use multi-stage build in Dockerfile (advanced)
- Mount source code instead of baking into image (trade-off: slower startup)

## Next Steps

1. ✅ Push code to GitHub
2. ✅ Verify GitHub Actions build completes
3. ✅ Run `setup-pi.sh` on Pi (one time)
4. ✅ Run `deploy.sh` to start development
5. ✅ Start developing! Changes to workspace persist across container restarts

## See Also

- [dingus_labs README](../README.md)
- [Dockerfile](./Dockerfile)
- [GitHub Container Registry Docs](https://docs.github.com/en/packages/working-with-a-github-packages-registry/working-with-the-container-registry)
