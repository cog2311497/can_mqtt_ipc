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
