# Finger index
uint8 THUMB=0
uint8 INDEX=1
uint8 MIDDLE=2
uint8 RING=3
uint8 PINKY=4

# Joint code
uint8 CMC=0
uint8 MCP=1
uint8 PIP=2
uint8 DIP=3
uint8 IP=4
uint8 UNKNOWN=255

# Unix epoch time
int64 stamp_sec
uint32 stamp_nanosec

bool is_right_hand

# Number of valid entries in arrays below (0..15)
uint8 valid_joint_count

# Fixed-size layout: 5 fingers * 3 joints = 15
uint8[15] finger
uint8[15] joint_index
uint8[15] joint
float32[15] roll
float32[15] pitch
float32[15] yaw
