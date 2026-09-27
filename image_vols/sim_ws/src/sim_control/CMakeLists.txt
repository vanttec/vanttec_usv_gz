cmake_minimum_required(VERSION 3.8)
project(sim_control)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(usv_interfaces REQUIRED)
find_package(sbg_driver REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)


add_executable(localization src/localization.cpp)
ament_target_dependencies(localization rclcpp std_msgs geometry_msgs sbg_driver tf2 tf2_geometry_msgs usv_interfaces)

add_executable(simplistic_control src/simplistic_control.cpp)
ament_target_dependencies(simplistic_control rclcpp std_msgs usv_interfaces tf2 tf2_geometry_msgs geometry_msgs)

install(TARGETS localization simplistic_control 
  DESTINATION lib/${PROJECT_NAME})

# Resources
install(DIRECTORY
  launch
  DESTINATION share/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
