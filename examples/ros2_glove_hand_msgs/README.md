# glove_hand_msgs

> English: This package defines custom ROS2 messages for SenseGlove Euler joint data.

用于导入到 ROS2 工作空间的自定义消息包，定义了手套欧拉角关节数据：

- `FingerJointEuler.msg`
- `HandEuler.msg`

关节命名采用通用解剖名：

- 拇指：`CMC -> MCP -> IP`
- 其余四指：`MCP -> PIP -> DIP`

## 导入与构建

将本目录复制到你的 ROS2 工作空间 `src/` 下，例如：

```bash
cp -r examples/ros2_glove_hand_msgs ~/ros2_ws/src/glove_hand_msgs
cd ~/ros2_ws
colcon build --packages-select glove_hand_msgs
source install/setup.bash
```

## 验证

```bash
ros2 interface show glove_hand_msgs/msg/HandEuler
```
