/*
 * <CAN MQTT IPC>
 *
 * Copyright (c) 2026 Cognizant.
 * All Rights Reserved.
 *
 * This software and associated documentation files (the "Software")
 * are the property of Cognizant.
 *
 * Permission is granted to use this Software solely in accordance
 * with the terms of a valid license agreement with Cognizant.
 *
 * Redistribution, modification, sublicensing, or commercial use
 * of this Software, in whole or in part, is prohibited except as
 * expressly authorized in writing by Cognizant.
 *
 * This Software is provided "AS IS" without warranty of any kind,
 * express or implied, including but not limited to the warranties
 * of merchantability, fitness for a particular purpose, and
 * non-infringement.
 *
 */

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
    int nbytes = 0;
    {
        std::lock_guard<std::mutex> lock(send_mutex_);
        nbytes = write(sock_, &cf, CAN_MTU);
    }

    return nbytes == CAN_MTU;
}
