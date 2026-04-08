# CycloneDDS 原生发布示例（C++）

> English: Native CycloneDDS C++ publishing example for SenseGlove Euler joint data.

本示例展示如何：

1. 使用 `examples/ros2_glove_hand_msgs` 的同构消息定义；
2. 用 CycloneDDS 原生 API 直接发布手套欧拉角；
3. 在 ROS2 侧通过同名同结构消息进行订阅/`ros2 topic echo`。

---

## 1) 先构建 ROS2 消息包（在你的 ROS2 工作空间）

```bash
cp -r examples/ros2_glove_hand_msgs ~/ros2_ws/src/glove_hand_msgs
cd ~/ros2_ws
source /opt/ros/$ROS_DISTRO/setup.bash
colcon build --packages-select glove_hand_msgs
source install/setup.bash
```

验证消息包是否已加载：

```bash
ros2 interface packages | grep glove_hand_msgs
ros2 interface show glove_hand_msgs/msg/HandEuler
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
REPO_ROOT=/path/to/repository
SGCORE_LIB_PATH=${REPO_ROOT}/lib/linux/vXX/x86-64/debug  # replace vXX with your installed version
c++ -std=c++17 \
  ${REPO_ROOT}/examples/cyclonedds/glove_dds_publisher.cpp \
  ${REPO_ROOT}/examples/cyclonedds/generated/glove_hand_msgs.c \
  -I generated \
  -I ${REPO_ROOT}/include \
  -L ${SGCORE_LIB_PATH} \
  -lsgcore -lsgconnect -lddsc -lpthread \
  -o glove_dds_publisher
```

---

## 4) ROS2 命令行查看

在 ROS2 环境（已 source `install/setup.bash`）中：

```bash
ros2 topic echo /glove/hand_euler glove_hand_msgs/msg/HandEuler
```

本示例原生 DDS 发布使用 `rt/glove/hand_euler`（对应 ROS2 话题 `/glove/hand_euler`），请勿去掉 `rt/` 前缀。

---

## 关节编码约定

- finger: `0..4 = Thumb/Index/Middle/Ring/Pinky`
- joint（关节类型枚举值，不是每根手指内部关节序号）:
  - `Thumb` 三个关节序号是 `0/1/2`，对应 joint code: `0(CMC), 1(MCP), 4(IP)`
  - `Other fingers` 三个关节序号是 `0/1/2`，对应 joint code: `1(MCP), 2(PIP), 3(DIP)`
