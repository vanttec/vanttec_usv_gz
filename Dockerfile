FROM ros:humble
WORKDIR /root
COPY ./image_files .
RUN apt update
RUN apt upgrade -y
RUN apt-get install curl lsb-release gnupg -y
RUN curl https://packages.osrfoundation.org/gazebo.gpg --output /usr/share/keyrings/pkgs-osrf-archive-keyring.gpg
RUN echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/pkgs-osrf-archive-keyring.gpg] https://packages.osrfoundation.org/gazebo/ubuntu-stable $(lsb_release -cs) main" | sudo tee /etc/apt/sources.list.d/gazebo-stable.list > /dev/null
RUN apt-get update
RUN apt install gz-harmonic ros-humble-ros-gz-bridge \
ros-humble-ros-gz-sim ros-humble-ros-gz-image -y

WORKDIR /root/gz_ws

RUN apt install ros-humble-boost-* libcgal-dev libgz-msgs9-dev \
libgz-sim7-dev libgz-rendering9-dev libfftw3-dev ros-humble-rtcm-msgs \
ros-humble-nmea-msgs ros-humble-ament-cmake -y

# cgal setup
RUN mkdir cgal
ENV CGAL_DATA_DIR=/root/cgal

#Ensure plugin is built for gz harmonic
ENV GZ_VERSION=harmonic

RUN colcon build --symlink-install --merge-install --cmake-args \
-DCMAKE_BUILD_TYPE=RelWithDebInfo \
-DBUILD_TESTING=ON \
-DCMAKE_CXX_STANDARD=17

# RUN source ./install/setup.bash

WORKDIR /root/gz_ws/src/asv_wave_sim/gz-waves/src/gui/plugins/waves_control
RUN mkdir build
WORKDIR /root/gz_ws/src/asv_wave_sim/gz-waves/src/gui/plugins/waves_control/build
RUN cmake .. && make

ENV ROS_DOMAIN_ID=46
ENV ROS_LOCALHOST_ONLY=0

ENV GZ_VERSION=harmonic
ENV GZ_IP=127.0.0.1
ENV GZ_SIM_RESOURCE_PATH=\
/root/gz_ws/src/asv_wave_sim/gz-waves-models/models:\
/root/gz_ws/src/asv_wave_sim/gz-waves-models/world_models:\
/root/gz_ws/src/asv_wave_sim/gz-waves-models/worlds:\
/root/sim_ws/src/usv_description/models

ENV GZ_SIM_SYSTEM_PLUGIN_PATH=\
/root/gz_ws/install/lib

ENV GZ_GUI_PLUGIN_PATH=\
/root/gz_ws/src/asv_wave_sim/gz-waves/src/gui/plugins/waves_control/build

WORKDIR /root


# Build ros2 ws and source packages
RUN /bin/bash -c "source /opt/ros/humble/setup.bash && \
                  cd sim_ws && \
                  colcon build"

RUN echo "source /opt/ros/humble/setup.bash" >> ~/.bashrc
RUN echo "source /root/sim_ws/install/setup.bash" >> ~/.bashrc
RUN echo "source /root/gz_ws/install/setup.bash" >> ~/.bashrc

ENV XDG_RUNTIME_DIR=/tmp/runtime-root




