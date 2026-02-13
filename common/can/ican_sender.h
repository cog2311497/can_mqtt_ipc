#pragma once

#include <vector>
#include <string>

#include "can/can_frame.h"


class ICanSender {
public:
    virtual ~ICanSender() = default;

    virtual bool open() = 0;

    virtual void close() = 0;

    virtual bool send(const CanFrame& frame) = 0;

    virtual bool is_open() const = 0;

    virtual std::string name() const = 0;
};
