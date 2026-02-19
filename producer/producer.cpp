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

#include "producer.h"

#include <iostream>
#include <cstdint>
#include <csignal>

#include "sensors/sensors_data.h"
#include "can/linux/sockets/can_sender.h"
#include "config/config_parser.h"


Producer::Producer() {
    signal(SIGTERM, Producer::signal_handler);  // Handle service stop signal
    signal(SIGINT, Producer::signal_handler);   // Handle Ctrl+C in terminal
}

bool Producer::initialize(int argc, char* argv[]) {
    auto config_opt = load_config(argc, argv);
    if (!config_opt) {
        std::cerr << "Using default configuration" << std::endl;
        return false;
    }
    config_ = *config_opt;

    if (!config_.contains("can_interfaces") || !config_.contains("producer") || !config_["producer"].contains("data_binding")) {
        std::cerr << "Invalid config file structure" << std::endl;
        return false;
    }
    return true;
}

bool Producer::start() {

    if (!setup_data_bindings()) {
        std::cerr << "Failed to set up data bindings" << std::endl;
        return false;
    }

    if (!setup_can_senders()) {
        std::cerr << "Failed to set up CAN senders" << std::endl;
        return false;
    }

    if (!setup_data_sending_callbacks()) {
        std::cerr << "Failed to set up data sending callbacks" << std::endl;
        return false;
    }

    return true;
}

void Producer::stop() {
    for (auto& data_source : data_sources_) {
        data_source->stop();
    }
}

void Producer::wait() {
    for (auto& data_source : data_sources_) {
        data_source->wait();
    }
}


bool Producer::setup_data_bindings() {
    nlohmann::json data_binding = config_["producer"]["data_binding"];

    // load data bindings from config
    // maps a data source (e.g. "temperature_sensor") to a CAN interface (e.g. "can0") and message ID (e.g. 0x100)
    for (auto& item : data_binding) {
        DataBinding binding;
        if (!(item.contains("source") && item.contains("destination") && item["destination"].contains("interface") && item["destination"].contains("msg_id"))) {
            std::cerr << "Invalid data_binding entry in config" << std::endl;
            continue;
        }
        binding.data_source = item["source"];
        binding.can_interface = item["destination"]["interface"];
        binding.can_msg_id =  std::stoul(item["destination"]["msg_id"].get<std::string>(), nullptr, 16);
        bindings_.push_back(binding);
        std::cout << binding.data_source << " -> " << binding.can_interface << " (0x" << std::hex << binding.can_msg_id << std::dec << ")" << std::endl;
    }

    if (bindings_.empty()) {
        std::cerr << "No valid data bindings found in config" << std::endl;
        return false;
    }
    return true;
}

bool Producer::setup_can_senders() {
    // Set up CAN senders for each unique CAN interface in the bindings
    for(const auto& can_interface  : config_["can_interfaces"]) {
        std::cout << "Setting up CAN interface: " << can_interface << std::endl;
        can_senders_[can_interface] = std::make_shared<LinuxSocketCanSender>(can_interface);
        can_senders_[can_interface]->open();
    }
    return true;
}

bool Producer::setup_data_sending_callbacks() {
    // Set up data sources and register callbacks to send CAN frames when new data is received
    for (const auto& binding : bindings_) {
        auto data_source = std::make_shared<SensorDataSource>(binding.data_source);
        data_source->register_callback([this, binding](const SensorData& data) {
            std::cout << "Received data from " << binding.data_source << ": Sensor ID=" << static_cast<int>(data.sensor_id) << " Value=" << data.value << std::endl;
            if (can_senders_.find(binding.can_interface) != can_senders_.end()) {
                CanFrame frame;
                frame.id = binding.can_msg_id;
                //frame.is_extended = binding.can_msg_id > CAN_SFF_MASK;   ?????????????????????????????
                frame.is_rtr = false;
                // Simple encoding: 1 bytes for sensor_id, 4 bytes for value
                frame.data.resize(5);
                std::memcpy(frame.data.data(), &data.sensor_id, sizeof(data.sensor_id));
                std::memcpy(frame.data.data() + sizeof(data.sensor_id), &data.value, sizeof(data.value));
                
                if (!can_senders_[binding.can_interface]->send(frame)) {
                    std::cerr << "Failed to send CAN frame on " << binding.can_interface << std::endl;
                } else {
                    std::cout << "Sent CAN frame on " << binding.can_interface << ": ID=0x" << std::hex << frame.id << std::dec 
                              << " Data(" << frame.data.size() << " bytes)" << std::endl;
                }
            } else {
                std::cerr << "CAN interface not found for binding: " << binding.can_interface << std::endl;
            }
        });
        data_source->start();
        data_sources_.push_back(std::move(data_source));
    }
    return true;
}

void Producer::signal_handler(int signum) {
    if (signum == SIGTERM || signum == SIGINT) {
        std::cout << "Received stop signal (SIGTERM or SIGINT). Shutting down gracefully..." << std::endl;
        if (instance_) {
            for(const auto& data_source : instance_->data_sources_) {
                data_source->stop();
            }
        }
    }
}

std::shared_ptr<Producer> Producer::instance_ = nullptr;