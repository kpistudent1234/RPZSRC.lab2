cmake_minimum_required(VERSION 3.8)
project(data_filter_pkg)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(std_msgs REQUIRED)
find_package(std_srvs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(pcl_conversions REQUIRED)
find_package(PCL REQUIRED)
find_package(data_filter_interfaces REQUIRED)

include_directories(${PCL_INCLUDE_DIRS})
link_directories(${PCL_LIBRARY_DIRS})
add_definitions(${PCL_DEFINITIONS})

add_executable(data_filter src/data_filter.cpp)
ament_target_dependencies(data_filter
  rclcpp
  std_msgs
  std_srvs
)

add_executable(reset_client src/reset_client.cpp)
ament_target_dependencies(reset_client
  rclcpp
  std_srvs
)

add_executable(action_server src/action_server.cpp)
ament_target_dependencies(action_server
  rclcpp
  rclcpp_action
  data_filter_interfaces
)

add_executable(point_cloud_demo src/point_cloud_demo.cpp)
ament_target_dependencies(point_cloud_demo
  rclcpp
  sensor_msgs
  pcl_conversions
)
target_link_libraries(point_cloud_demo ${PCL_LIBRARIES})

install(TARGETS
  data_filter
  reset_client
  action_server
  point_cloud_demo
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
