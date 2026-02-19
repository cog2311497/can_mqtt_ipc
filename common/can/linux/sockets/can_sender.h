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

#pragma once

#include <string>
#include <utility>
#include <mutex>

#include "can/ican_sender.h"


class LinuxSocketCanSender : public ICanSender {
public:
    explicit LinuxSocketCanSender(std::string ifname)
        : ifname_(std::move(ifname)) {}

    bool open() override;

    void close() override;

    bool send(const CanFrame& frame) override;

    bool is_open() const override {
        return open_;
    }

    std::string name() const override {
        return ifname_;
    }

private:
    std::string ifname_;
    int sock_{-1};
    bool open_{false};
    std::mutex send_mutex_;
};
