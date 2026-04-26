# Development Docker image for Dingus ROS2 packages
# Hardware-only (no Gazebo simulation)

FROM ros:jazzy

# Install system dependencies and development tools
RUN apt-get update && apt-get install -y \
    vim \
    protobuf-compiler \
    libboost-all-dev \
    udev \
    usbutils \
    python3-pip \
    tmux \
    v4l-utils \
    && rm -rf /var/lib/apt/lists/*

# Install ROS 2 packages (hardware, sensors, control, navigation)
RUN apt-get update && apt-get install -y \
    ros-jazzy-xacro \
    ros-jazzy-image-transport-plugins \
    ros-jazzy-rplidar-ros \
    ros-jazzy-v4l2-camera \
    ros-jazzy-ros2-control \
    ros-jazzy-ros2-controllers \
    ros-jazzy-rosbridge-suite \
    ros-jazzy-controller-manager \
    ros-jazzy-joy-linux \
    ros-jazzy-joy-teleop \
    ros-jazzy-teleop-twist-joy \
    ros-jazzy-slam-toolbox \
    ros-jazzy-navigation2 \
    ros-jazzy-nav2-bringup \
    ros-jazzy-realsense2-camera \
    ros-jazzy-librealsense2 \
    ros-jazzy-twist-mux \
    python3-rosdep \
    && rm -rf /var/lib/apt/lists/*

# Copy udev rules for hardware recognition
COPY 80-st-board.rules /etc/udev/rules.d/80-st-board.rules

# Clone source repositories
WORKDIR /dingus_ws/src
RUN git clone https://github.com/TWALL9/dingus_labs.git --recurse-submodules
RUN git clone https://github.com/TWALL9/async_serial.git
RUN git clone https://github.com/ros-teleop/twist_mux.git

# Build ROS workspace
WORKDIR /dingus_ws
RUN . /opt/ros/jazzy/setup.sh && rosdep install --from-paths src --ignore-src -y
RUN . /opt/ros/jazzy/setup.sh && colcon build --symlink-install

# Source setup script on container start
RUN echo "source /opt/ros/jazzy/setup.bash" >> /root/.bashrc && \
    echo "source /dingus_ws/install/local_setup.bash" >> /root/.bashrc
