# Docker x11
xhost +local:docker > /dev/null

sudo docker run -it \
  --net=host \
  --gpus all \
  -e NVIDIA_DRIVER_CAPABILITIES=all \
  -e DISPLAY=$DISPLAY \
  -v /tmp/.X11-unix:/tmp/.X11-unix:rw \
  usv-gz:latest
