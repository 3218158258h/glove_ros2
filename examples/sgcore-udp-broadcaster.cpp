#include <array>
#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <cstring>
#include <exception>
#include <iostream>
#include <string>
#include <thread>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <SenseGlove/Core/HandLayer.hpp>
#include <SenseGlove/Core/HandPose.hpp>
#include <SenseGlove/Core/Library.hpp>
#include <SenseGlove/Core/SenseCom.hpp>
#include <SenseGlove/Core/Vect3D.hpp>

using namespace SGCore;
using namespace SGCore::Kinematics;

static std::atomic<bool> g_stop{false};
static constexpr std::chrono::milliseconds kPublishInterval{15};

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

static void SignalHandler(int)
{
    g_stop = true;
}

static bool EnsureSenseCom()
{
    if (SenseCom::ScanningActive())
    {
        return true;
    }
    std::cout << "SenseCom not active, trying to start..." << std::endl;
    return SenseCom::StartupSenseCom();
}

static bool BuildPacketFromPose(const HandPose &pose, HandAnglesUdpPacket &outPacket)
{
    std::memset(&outPacket, 0, sizeof(outPacket));
    outPacket.magic[0] = 'S';
    outPacket.magic[1] = 'G';
    outPacket.magic[2] = 'A';
    outPacket.magic[3] = '1';
    outPacket.version = 1;
    outPacket.is_right_hand = pose.IsRight() ? 1 : 0;

    const auto now = std::chrono::system_clock::now().time_since_epoch();
    const auto sec = std::chrono::duration_cast<std::chrono::seconds>(now);
    const auto nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(now - sec);
    outPacket.stamp_sec = static_cast<int64_t>(sec.count());
    outPacket.stamp_nanosec = static_cast<uint32_t>(nsec.count());

    const auto &angles = pose.GetHandAngles();
    size_t outIndex = 0;
    for (size_t fingerIndex = 0; fingerIndex < angles.size() && outIndex < 15; ++fingerIndex)
    {
        for (size_t jointIndex = 0; jointIndex < angles[fingerIndex].size() && outIndex < 15; ++jointIndex)
        {
            const Vect3D &e = angles[fingerIndex][jointIndex];
            outPacket.joints[outIndex].roll = e.GetX();
            outPacket.joints[outIndex].pitch = e.GetY();
            outPacket.joints[outIndex].yaw = e.GetZ();
            ++outIndex;
        }
    }

    outPacket.valid_joint_count = static_cast<uint8_t>(outIndex);
    return outIndex > 0;
}

int main(int argc, char **argv)
{
    const std::string broadcastIp = (argc > 1) ? argv[1] : "255.255.255.255";
    int port = 9000;
    if (argc > 2)
    {
        try
        {
            port = std::stoi(argv[2]);
        }
        catch (const std::exception &)
        {
            std::cerr << "Invalid port argument: " << argv[2] << std::endl;
            return 1;
        }
    }
    if (port < 1 || port > 65535)
    {
        std::cerr << "Port out of range [1, 65535]: " << port << std::endl;
        return 1;
    }

    signal(SIGINT, SignalHandler);
    signal(SIGTERM, SignalHandler);

    std::cout << "SGCore version: " << Library::Version() << std::endl;
    std::cout << "UDP broadcast target: " << broadcastIp << ":" << port << std::endl;

    if (!EnsureSenseCom())
    {
        std::cerr << "Failed to start SenseCom." << std::endl;
        return 1;
    }

    int udpSocket = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket < 0)
    {
        std::cerr << "Failed to create UDP socket." << std::endl;
        return 1;
    }

    int enableBroadcast = 1;
    if (::setsockopt(udpSocket, SOL_SOCKET, SO_BROADCAST, &enableBroadcast, sizeof(enableBroadcast)) < 0)
    {
        std::cerr << "Failed to enable SO_BROADCAST." << std::endl;
        ::close(udpSocket);
        return 1;
    }

    sockaddr_in dest{};
    dest.sin_family = AF_INET;
    dest.sin_port = htons(static_cast<uint16_t>(port));
    if (::inet_pton(AF_INET, broadcastIp.c_str(), &dest.sin_addr) != 1)
    {
        std::cerr << "Invalid broadcast ip: " << broadcastIp << std::endl;
        ::close(udpSocket);
        return 1;
    }

    while (!g_stop.load())
    {
        for (bool isRightHand : {true, false})
        {
            if (!HandLayer::DeviceConnected(isRightHand))
            {
                continue;
            }

            HandPose pose;
            if (!HandLayer::GetHandPose(isRightHand, pose))
            {
                continue;
            }

            HandAnglesUdpPacket packet{};
            if (!BuildPacketFromPose(pose, packet))
            {
                continue;
            }

            const ssize_t sent = ::sendto(
                udpSocket,
                &packet,
                sizeof(packet),
                0,
                reinterpret_cast<const sockaddr *>(&dest),
                sizeof(dest));

            if (sent != static_cast<ssize_t>(sizeof(packet)))
            {
                std::cerr << "sendto failed." << std::endl;
            }
        }

        std::this_thread::sleep_for(kPublishInterval);
    }

    ::close(udpSocket);
    return 0;
}
