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

#include <memory>
#include <mutex>
#include <vector>

#include <nlohmann/json.hpp>

#include "sensors/idata_source.h"
#include "sensors/emulated/sensor_data_source.h"
#include "can/ican_sender.h"


class Producer
{
public:
    Producer();
    ~Producer() = default;

    bool initialize(int argc, char* argv[]);
    bool start();
    void stop();
    void wait();

    static void set_instance(std::shared_ptr<Producer> app) {
        instance_ = app;
    }

protected:
    static void signal_handler(int signum);
    bool setup_data_bindings();
    bool setup_can_senders();
    bool setup_data_sending_callbacks();

private:

    struct DataBinding {
        std::string data_source;
        std::string can_interface;
        uint32_t can_msg_id;
    };

    nlohmann::json config_;
    std::vector<std::shared_ptr<IDataSource<SensorData>>> data_sources_;
    std::vector<DataBinding> bindings_;
    std::map<std::string, std::shared_ptr<ICanSender>> can_senders_;

    static std::shared_ptr<Producer> instance_;
};