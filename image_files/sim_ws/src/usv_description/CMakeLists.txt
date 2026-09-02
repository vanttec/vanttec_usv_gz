cmake_minimum_required(VERSION 3.8)
project(usv_description)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(urdf REQUIRED)

find_package(rclcpp REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(sbg_driver REQUIRED)


add_executable(quat_conversion_node src/quatConversionNode.cpp)
ament_target_dependencies(quat_conversion_node 
  rclcpp 
  nav_msgs 
  std_msgs 
  tf2 
  tf2_ros
  sbg_driver
  sensor_msgs
)

install(TARGETS
  quat_conversion_node
  DESTINATION lib/${PROJECT_NAME}
)

#============================================================================
# Resources
install(DIRECTORY
  launch
  urdf
  models
  worlds
  rviz
  config
  DESTINATION share/${PROJECT_NAME})


if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()