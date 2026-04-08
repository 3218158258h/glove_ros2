cmake_minimum_required(VERSION 3.8)
project(glove_udp_bridge)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(glove_hand_msgs REQUIRED)

add_executable(glove_udp_bridge_node src/glove_udp_bridge_node.cpp)
target_compile_features(glove_udp_bridge_node PUBLIC cxx_std_17)
ament_target_dependencies(glove_udp_bridge_node
  rclcpp
  glove_hand_msgs
)

install(TARGETS glove_udp_bridge_node
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
