# dingus_labs
Monorepo for all the silly little robots I make

## Quick Start: Deploy to Raspberry Pi

To deploy the development environment to a Raspberry Pi 4/5:

1. **Push code to GitHub** (triggers automatic ARM64 Docker build)
   ```bash
   git push origin main
   ```

2. **One-time setup on Pi** (install Docker, configure GHCR auth)
   ```bash
   bash docker/scripts/setup-pi.sh
   ```

3. **Deploy latest image**
   ```bash
   ~/deploy.sh
   ```

See [docker/DEPLOYMENT.md](docker/DEPLOYMENT.md) for complete instructions, troubleshooting, and architecture details.

### Daily Workflow
- Code changes → Push to GitHub → Automatic build → `./deploy.sh` on Pi → Done!
- Workspace persists at `~/dingus_ws` on the Pi (bind-mounted in container)
