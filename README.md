# VANTTEC USV Gazebo Simulator

This repository has the aim of facilitating the use of the ASV wave sim plugin for gazebo (https://github.com/srmainwaring/asv_wave_sim). By using docker, this repository enables anyone utilizing a linux distribution and an x11 display server to quickly and easily run the full simulation.


## Prerequisites
- Docker installed on your machine
- Decent hardware (CPU/GPU) for smooth performance

## Installation

### Step 1: Clone the repository and its submodules

```
git clone https://github.com/vanttec/vanttec_usv_gz.git
cd vanttec_usv_gz
git submodule update --init --recursive
```

### Step 2: Build the Docker image

```
sudo docker build . -t usv-gz:latest
```
The build process may take a few minutes, be patient.

### Step 3: Run the container and simulation

Once the image is built, the following command can be used:

```
chmod +x start_container.sh
./start_container.sh
```
This will create a container from the built image and open a terminal in its /root directory. This container is already set up to run the simulation. To run the simulation in headless mode (ros2 topics should be visible in the host machine):
```
ros2 launch usv_description headless_gazebo_launch.py
```

### Auxiliary step: synchronize ros2 environment variables

To be able to see ros2 topics outside the container, which is the intended use case for this repository, multiple conditions must be met:

- You have ros2 humble installed on your machine, and it is using fast dds
- You have the ROS_LOCALHOST_ONLY environment variable set to "0"
- You have the ROS_DOMAIN_ID environment variable set to "46"

If you desire to use a distinct domain id, simply run the following command inside the container before launching the simulation (or modify in the Dockerfile before building):

```
export ROS_DOMAIN_ID=<your_desired_id>
```

## Simulation ros2 topic configuration

The ros2 topics published by the simulation are designed to mimic the topics the physical USV publishes. Some notable topics are:

1. /usv/left_thruster [std_msgs/msg/Float64] and /usv/right_thruster [std_msgs/msg/Float64]: thruster inputs to move the boat
2. /beeblebrox/video [sensor_msgs/msg/Image]: rgb camera positioned in the same place as real zed x mini. (although with no depth capabilities)
3. /sbg/ekf_quat [sbg_driver/msg/SbgEkfQuat], /imu/nav_sat_fix [sensor_msgs/msg/NavSatFix], and more Sbg topics recreating real INS measurements while deployed

To see all ros2 topics available, simply run the following command on your host machine (Assuming you have the ROS_DOMAIN_ID setup correctly)

```
ros2 topic list -t
```

Have fun!!