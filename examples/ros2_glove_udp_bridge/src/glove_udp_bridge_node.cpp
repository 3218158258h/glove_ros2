#include <algorithm>
#include <array>
#include <atomic>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <thread>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <rclcpp/rclcpp.hpp>
#include <glove_hand_msgs/msg/finger_joint_euler.hpp>
#include <glove_hand_msgs/msg/hand_euler.hpp>

#pragma pack(push, 1)
struct JointEulerPacket
{
    float roll;
    float pitch;
    float yaw;
};

struct HandAnglesUdpPacket
{
    char magic[4];
    uint16_t version;
    uint16_t reserved;
    int64_t stamp_sec;
    uint32_t stamp_nanosec;
    uint8_t is_right_hand;
    uint8_t valid_joint_count;
    uint8_t reserved2[2];
    JointEulerPacket joints[15];
};
#pragma pack(pop)

static_assert(sizeof(HandAnglesUdpPacket) == 204, "Unexpected UDP packet size");

class GloveUdpBridgeNode : public rclcpp::Node
{
public:
    GloveUdpBridgeNode()
        : Node("glove_udp_bridge_node"), running_(true)
    {
        bind_address_ = declare_parameter<std::string>("bind_address", "0.0.0.0");
        udp_port_ = declare_parameter<int>("udp_port", 9000);
        right_topic_ = declare_parameter<std::string>("right_topic", "/glove/right/hand_euler");
        left_topic_ = declare_parameter<std::string>("left_topic", "/glove/left/hand_euler");

        right_pub_ = create_publisher<glove_hand_msgs::msg::HandEuler>(right_topic_, 10);
        left_pub_ = create_publisher<glove_hand_msgs::msg::HandEuler>(left_topic_, 10);

        socket_fd_ = ::socket(AF_INET, SOCK_DGRAM, 0);
        if (socket_fd_ < 0)
        {
            throw std::runtime_error("Failed to create UDP socket");
        }

        int reuse = 1;
        (void)::setsockopt(socket_fd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(static_cast<uint16_t>(udp_port_));
        if (::inet_pton(AF_INET, bind_address_.c_str(), &addr.sin_addr) != 1)
        {
            ::close(socket_fd_);
            throw std::runtime_error("Invalid bind_address parameter");
        }
        if (::bind(socket_fd_, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0)
        {
            ::close(socket_fd_);
            throw std::runtime_error("Failed to bind UDP socket");
        }

        receive_thread_ = std::thread(&GloveUdpBridgeNode::ReceiveLoop, this);
        RCLCPP_INFO(get_logger(), "Listening UDP %s:%d", bind_address_.c_str(), udp_port_);
    }

    ~GloveUdpBridgeNode() override
    {
        running_.store(false);
        if (socket_fd_ >= 0)
        {
            ::close(socket_fd_);
            socket_fd_ = -1;
        }
        if (receive_thread_.joinable())
        {
            receive_thread_.join();
        }
    }

private:
    static uint8_t JointCode(uint8_t finger, uint8_t joint_index)
    {
        using glove_hand_msgs::msg::FingerJointEuler;
        if (finger == FingerJointEuler::THUMB)
        {
            static const std::array<uint8_t, 3> thumb_map = {
                FingerJointEuler::CMC,
                FingerJointEuler::MCP,
                FingerJointEuler::IP};
            return (joint_index < thumb_map.size()) ? thumb_map[joint_index] : FingerJointEuler::UNKNOWN;
        }

        static const std::array<uint8_t, 3> finger_map = {
            FingerJointEuler::MCP,
            FingerJointEuler::PIP,
            FingerJointEuler::DIP};
        return (joint_index < finger_map.size()) ? finger_map[joint_index] : FingerJointEuler::UNKNOWN;
    }

    static glove_hand_msgs::msg::HandEuler ToRosMessage(const HandAnglesUdpPacket &packet)
    {
        glove_hand_msgs::msg::HandEuler msg;
        msg.stamp_sec = packet.stamp_sec;
        msg.stamp_nanosec = packet.stamp_nanosec;
        msg.is_right_hand = (packet.is_right_hand != 0);

        const uint8_t valid_count = std::min<uint8_t>(packet.valid_joint_count, 15);
        msg.valid_joint_count = valid_count;

        for (size_t i = 0; i < msg.joints.size(); ++i)
        {
            auto &j = msg.joints[i];
            const uint8_t finger = static_cast<uint8_t>(i / 3);
            const uint8_t joint_index = static_cast<uint8_t>(i % 3);

            j.finger = finger;
            j.joint_index = joint_index;
            j.joint = JointCode(finger, joint_index);

            if (i < valid_count)
            {
                j.roll = packet.joints[i].roll;
                j.pitch = packet.joints[i].pitch;
                j.yaw = packet.joints[i].yaw;
            }
            else
            {
                j.roll = 0.0f;
                j.pitch = 0.0f;
                j.yaw = 0.0f;
            }
        }
        return msg;
    }

    void ReceiveLoop()
    {
        while (running_.load() && rclcpp::ok())
        {
            HandAnglesUdpPacket packet{};
            sockaddr_in src_addr{};
            socklen_t src_len = sizeof(src_addr);
            const ssize_t received = ::recvfrom(
                socket_fd_,
                &packet,
                sizeof(packet),
                0,
                reinterpret_cast<sockaddr *>(&src_addr),
                &src_len);

            if (received < 0)
            {
                if (running_.load())
                {
                    RCLCPP_WARN_THROTTLE(
                        get_logger(),
                        *get_clock(),
                        2000,
                        "recvfrom failed.");
                }
                continue;
            }
            if (received != static_cast<ssize_t>(sizeof(HandAnglesUdpPacket)))
            {
                continue;
            }
            if (std::memcmp(packet.magic, "SGA1", 4) != 0 || packet.version != 1)
            {
                continue;
            }

            auto msg = ToRosMessage(packet);
            if (msg.is_right_hand)
            {
                right_pub_->publish(msg);
            }
            else
            {
                left_pub_->publish(msg);
            }
        }
    }

private:
    std::string bind_address_;
    int udp_port_;
    std::string right_topic_;
    std::string left_topic_;

    int socket_fd_{-1};
    std::atomic<bool> running_;
    std::thread receive_thread_;

    rclcpp::Publisher<glove_hand_msgs::msg::HandEuler>::SharedPtr right_pub_;
    rclcpp::Publisher<glove_hand_msgs::msg::HandEuler>::SharedPtr left_pub_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<GloveUdpBridgeNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
