# dingus_docker
Dockerfile and other misc files needed to set up the dingus workspace on other PC's

# Startup
Note that not all packages may be in the Dockerfile. You may need to add more via rosdep on the host development machine, not the one you indend to run the Dockerfile on

```bash
cd /path/to/workspace
sudo rosdep init
rosdep update
rosdep install --from-paths src -y --ignore-src
```

# Installing USB rules via udev
```bash
sudo cp 80-st-board.rules /etc/udev/rules.d/
sudo udevadm control --reload-rules && sudo udevadm trigger
```

# Running Docker
to build the image
```bash
docker build . -t dingus_img
```

to run the image
```bash
docker run -it --privileged -v /dev:/dev --net=host --name dingus_ws dingus_img
```

to run it again
```bash
docker start dingus_ws
docker attach dingus_ws
```
