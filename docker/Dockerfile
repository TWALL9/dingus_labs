FROM ros:jazzy

RUN apt-get update && apt-get install -y \
    tmux \
    ros-jazzy-xacro \
    ros-jazzy-image-transport-plugins \
    ros-jazzy-rplidar-ros \
    ros-jazzy-v4l2-camera \
    v4l-utils \
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
    vim \
    protobuf-compiler \
    libboost-all-dev \
    udev \
    usbutils \
    python3-pip \
    python3-rosdep

# RUN python3 -m pip install flask

COPY 80-st-board.rules /etc/udev/rules.d/80-st-board.rules

WORKDIR /otomo2_ws/src
RUN git clone https://github.com/TWALL9/otomo_msgs.git --recurse-submodules
RUN git clone https://github.com/TWALL9/otomo_control.git
RUN git clone https://github.com/TWALL9/otomo_core.git
RUN git clone https://github.com/TWALL9/otomo_plugins.git
RUN git clone https://github.com/TWALL9/async_serial.git
RUN git clone https://github.com/ros-teleop/twist_mux.git

WORKDIR /otomo2_ws
