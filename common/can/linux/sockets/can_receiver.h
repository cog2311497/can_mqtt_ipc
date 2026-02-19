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

#include <atomic>
#include <cstring>
#include <mutex>
#include <thread>
#include <vector>

#include "can/ican_receiver.h"


class LinuxSocketCanReceiver : public ICanReceiver
{
public:
    explicit LinuxSocketCanReceiver(std::string ifname)
        : ifname_(std::move(ifname))
    {
    }

    ~LinuxSocketCanReceiver() override
    {
        close();
    }

    bool open() override;
    bool start() override;
    void stop() override;
    void close() override;

    bool is_open() const override
    {
        return socket_fd_ >= 0;
    }

    void wait() override;

    std::string name() const override
    {
        return ifname_;
    }

    SubscriptionPtr subscribe(Callback cb) override;

private:
    void unsubscribe(uint64_t id);

    void receive_loop();

private:
    std::string ifname_;
    int socket_fd_{-1};

    std::atomic<bool> running_{false};
    std::thread worker_;

    std::mutex mutex_;
    std::vector<std::pair<uint64_t, Callback>> subscribers_;
    uint64_t next_id_{0};
};
