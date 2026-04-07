# CycloneDDS 原生发布示例（C++）

本示例展示如何：

1. 使用 `examples/ros2_glove_hand_msgs` 的同构消息定义；
2. 用 CycloneDDS 原生 API 直接发布手套欧拉角；
3. 在 ROS2 侧通过同名同结构消息进行订阅/`ros2 topic echo`。

---

## 1) 先构建 ROS2 消息包（在你的 ROS2 工作空间）

```bash
cp -r examples/ros2_glove_hand_msgs ~/ros2_ws/src/glove_hand_msgs
cd ~/ros2_ws
colcon build --packages-select glove_hand_msgs
source install/setup.bash
```

---

## 2) 生成 CycloneDDS 类型代码

在本仓库目录执行：

```bash
cd examples/cyclonedds
mkdir -p generated
idlc -l c -o generated idl/glove_hand_msgs.idl
```

生成后应有 `generated/glove_hand_msgs.h` 与 `generated/glove_hand_msgs.c`。

---

## 3) 编译原生发布程序（示例命令）

> 你需要按本机安装路径调整 `-I/-L` 参数（CycloneDDS 与 SenseGlove 库）。

```bash
REPO_ROOT=/path/to/glove_ros2
c++ -std=c++17 \
  ${REPO_ROOT}/examples/cyclonedds/glove_dds_publisher.cpp \
  ${REPO_ROOT}/examples/cyclonedds/generated/glove_hand_msgs.c \
  -I generated \
  -I ${REPO_ROOT}/include \
  -L ${REPO_ROOT}/lib/linux/v22/x86-64/debug \
  -lsgcore -lsgconnect -lddsc -lpthread \
  -o glove_dds_publisher
```

---

## 4) ROS2 命令行查看

在 ROS2 环境（已 source `install/setup.bash`）中：

```bash
ros2 topic echo /glove/hand_euler glove_hand_msgs/msg/HandEuler
```

如果你修改了 topic 名，请和 `glove_dds_publisher.cpp` 中一致。

---

## 关节编码约定

- finger: `0..4 = Thumb/Index/Middle/Ring/Pinky`
- joint:
  - `Thumb`: `0(CMC), 1(MCP), 4(IP)`
  - `Other fingers`: `1(MCP), 2(PIP), 3(DIP)`
