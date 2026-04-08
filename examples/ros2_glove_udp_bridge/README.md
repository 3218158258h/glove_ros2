# glove_udp_bridge

ROS2 功能包：接收 SenseGlove UDP 欧拉角数据并发布到左右手两个话题。

## 1) 先构建消息包

```bash
cp -r examples/ros2_glove_hand_msgs ~/ros2_ws/src/glove_hand_msgs
```

## 2) 导入并构建桥接包

```bash
cp -r examples/ros2_glove_udp_bridge ~/ros2_ws/src/glove_udp_bridge
cd ~/ros2_ws
source /opt/ros/$ROS_DISTRO/setup.bash
colcon build --packages-select glove_hand_msgs glove_udp_bridge
source install/setup.bash
```

## 3) 运行桥接节点

```bash
ros2 run glove_udp_bridge glove_udp_bridge_node \
  --ros-args \
  -p bind_address:=0.0.0.0 \
  -p udp_port:=9000 \
  -p right_topic:=/glove/right/hand_euler \
  -p left_topic:=/glove/left/hand_euler
```

## 4) 运行 UDP 发送端（本仓库）

先在本仓库编译：

```bash
cd /path/to/glove_ros2
cmake -S . -B build
cmake --build build --target udp_broadcaster
```

运行发送：

```bash
./build/udp_broadcaster 255.255.255.255 9000
```

参数：

- argv[1] 广播地址，默认 `255.255.255.255`
- argv[2] 端口，默认 `9000`
