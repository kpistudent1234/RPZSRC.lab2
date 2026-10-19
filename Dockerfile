FROM osrf/ros:jazzy-desktop

RUN apt-get update && apt-get install -y \
    ros-jazzy-rclcpp-components \
    ros-jazzy-rclcpp-lifecycle \
    python3-colcon-common-extensions \
    && rm -rf /var/lib/apt/lists/*

COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

WORKDIR /ros2_ws

ENTRYPOINT ["/entrypoint.sh"]
CMD ["bash"]
