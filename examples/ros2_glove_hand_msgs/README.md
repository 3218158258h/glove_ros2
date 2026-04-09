# glove_hand_msgs

> English: This package defines custom ROS2 messages for SenseGlove Euler joint data.

用于导入到 ROS2 工作空间的自定义消息包，定义了手套欧拉角文本数据：

- `HandEuler.msg`（单一消息字段：`string euler_text`）

文本推荐格式（多行）：

- `Thumb: CMC:x,y,z   MCP:x,y,z   IP:x,y,z`
- `Index: MCP:x,y,z   PIP:x,y,z   DIP:x,y,z`

## 导入与构建

将本目录复制到你的 ROS2 工作空间 `src/` 下，例如：

```bash
cp -r examples/ros2_glove_hand_msgs ~/ros2_ws/src/glove_hand_msgs
cd ~/ros2_ws
source /opt/ros/$ROS_DISTRO/setup.bash
colcon build --packages-select glove_hand_msgs
source install/setup.bash
```

## 验证

```bash
ros2 interface packages | grep glove_hand_msgs
ros2 interface show glove_hand_msgs/msg/HandEuler
```
