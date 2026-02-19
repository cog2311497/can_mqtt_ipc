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

#include <functional>
#include <memory>
#include <string>

#include "can/can_frame.h"

class ICanReceiver
{
public:
    using Callback = std::function<void(const CanFrame&)>;

    class Subscription
    {
    public:
        virtual ~Subscription() = default;
    };

    using SubscriptionPtr = std::unique_ptr<Subscription>;

    virtual ~ICanReceiver() = default;

    virtual bool open() = 0;
    virtual bool start() = 0;

    virtual void close() = 0;
    virtual void stop() = 0;

    virtual SubscriptionPtr subscribe(Callback cb) = 0;

    virtual bool is_open() const = 0;

    virtual void wait() = 0;

    virtual std::string name() const = 0;
};
