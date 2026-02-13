
#include "can/linux/sockets/can_sender.h"

#include <cstring>
#include <unistd.h>
#include <iostream>

#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <sys/ioctl.h>


bool LinuxSocketCanSender::open() {
    
    if ((sock_ = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
        return false;
    }

    struct ifreq ifr{};
    std::strncpy(ifr.ifr_name, ifname_.c_str(), IFNAMSIZ);
    if (ioctl(sock_, SIOCGIFINDEX, &ifr) < 0) {
        return false;
    }

    struct sockaddr_can addr{};
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(sock_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        return false;
    }

    open_ = true;
    return true;
}

void LinuxSocketCanSender::close() {
    if (open_) {
        ::close(sock_);
        open_ = false;
    }
}

bool LinuxSocketCanSender::send(const CanFrame& frame) {
    if (!open_) {
        return false;
    }

    struct can_frame cf{};
    cf.can_id = frame.id | (frame.is_extended ? CAN_EFF_FLAG : 0);
    cf.can_dlc = frame.data.size();
    std::memcpy(cf.data, frame.data.data(), cf.can_dlc);
    int nbytes = write(sock_, &cf, CAN_MTU);

    return nbytes == CAN_MTU;
}
