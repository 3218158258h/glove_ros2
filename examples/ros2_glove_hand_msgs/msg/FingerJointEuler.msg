# Finger index (from SenseGlove / common robotics hand indexing)
uint8 THUMB=0
uint8 INDEX=1
uint8 MIDDLE=2
uint8 RING=3
uint8 PINKY=4

# Joint code (generic anatomical naming).
# Note: this is an anatomical enum code, not the per-finger joint array index.
# 注意：这是“关节类型枚举值”，不是每根手指内部的数组下标。
#
# Anatomical meaning:
# - CMC: Carpometacarpal
# - MCP: Metacarpophalangeal
# - PIP: Proximal interphalangeal
# - DIP: Distal interphalangeal
# - IP : Interphalangeal
# Mapping by finger:
# - Thumb uses CMC / MCP / IP
# - Index/Middle/Ring/Pinky use MCP / PIP / DIP
uint8 CMC=0
uint8 MCP=1
uint8 PIP=2
uint8 DIP=3
uint8 IP=4
uint8 UNKNOWN=255

uint8 finger
uint8 joint
float32 roll
float32 pitch
float32 yaw
