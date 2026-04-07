# Unix epoch time
int32 stamp_sec
uint32 stamp_nanosec

bool is_right_hand

# Number of valid entries in joints[] (0..15)
uint8 valid_joint_count

# Fixed-size layout: 5 fingers * 3 joints = 15
glove_hand_msgs/FingerJointEuler[15] joints
