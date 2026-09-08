# VANTTEC USV Gazebo Simulator

This repository has the aim of facilitating the use of the ASV wave sim plugin for gazebo (https://github.com/srmainwaring/asv_wave_sim). By using docker, this repository enables anyone utilizing a linux distribution and an x11 display server to quickly and easily run the full simulation.


## Prerequisites
- Docker installed on your machine
- An x11 display server running on a linux machine
- Decent hardware (CPU/GPU) for smooth performance

Note: This repository has only been tested on Ubuntu 22.04, although it should probably work on any linux distribution that uses x11.

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
This will create a container from the built image and open a terminal in its /root directory. This container is already mostly set up to run the simulation. However, due to volume configurations the ros2 workspace used for topic bridging must still be built. Therefore the following process is necessary.

```
cd /root/sim_ws
colcon build
source ./install/setup.bash
```

Now the simulation can be ran. A common command to launch it is:

```
ros2 launch usv_description gazebo_launch.py
```
Or, to run the simulation in headless mode (ros2 topics should be visible in the host machine):
```
ros2 launch usv_description headless_gazebo_launch.py
```
Have fun!!

### Modifying the simulation

The start_container.sh script links the external ./image_volumes/sim_ws folder on your machine to the internal /root/sim_ws folder inside the container. This allows you to modify the files in this ros2 workspace comfortably from your own machine without shutting down the container or manually copying files. This also means that there should be no risk of losing changes when shutting down the container, and including the changes in the git repository is simplified. However, remember that **after every modification, the workspace should be rebuild and sourced**. 
```
cd /root/sim_ws
colcon build
source ./install/setup.bash
```