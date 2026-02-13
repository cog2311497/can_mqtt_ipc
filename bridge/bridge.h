#pragma once

#include <memory>
#include <mutex>
#include <vector>
#include <map>
#include <string>

#include <nlohmann/json.hpp>
#include <mqtt/async_client.h>

#include "can/linux/sockets/can_receiver.h"


class Bridge
{
public:
    Bridge();
    ~Bridge() = default;

    bool initialize(int argc, char* argv[]);
    bool start();
    void stop();
    void wait();

    static void set_instance(std::shared_ptr<Bridge> app) {
        instance_ = app;
    }

protected:
    static void signal_handler(int signum);
    bool connect_mqtt();
    bool setup_can_readers();

private:
    nlohmann::json config_;
    std::unique_ptr<mqtt::async_client> client_;
    std::map<std::string, std::shared_ptr<ICanReceiver>> can_receivers_;
    std::vector<ICanReceiver::SubscriptionPtr> subscriptions_;
    mutable std::mutex mqtt_mutex_;
    std::vector<std::string> mqtt_topics_;

    static std::shared_ptr<Bridge> instance_;
};