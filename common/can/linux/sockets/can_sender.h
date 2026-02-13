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
