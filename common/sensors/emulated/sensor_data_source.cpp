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

#include "sensor_data_source.h"

#include <iostream>
#include <limits>
#include <cstdlib>
#include <cstring>


#include "sensors/sensors_data.h"

SensorDataSource::SensorDataSource(std::string name) : name_(std::move(name)) {}

SensorDataSource::~SensorDataSource() {
    stop();
}

void SensorDataSource::register_callback(DataCallback<SensorData> callback) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    data_callback_ = std::move(callback);
}

bool SensorDataSource::start() {
    if (running_.exchange(true)) {
        return false;
    }

    try {
        worker_thread_ = std::make_unique<std::thread>(&SensorDataSource::thread_worker, this);
        std::cout << "Data source thread started" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to start data source thread: " << e.what() << std::endl;
        running_.store(false);
        return false;
    }
}

void SensorDataSource::stop() {
    running_.exchange(false);
}

bool SensorDataSource::is_running() const {
    return running_.load();
}

void SensorDataSource::wait() {
    if (worker_thread_ && worker_thread_->joinable()) {
        worker_thread_->join();
    }
}


void SensorDataSource::thread_worker() {
    std::cout << "Worker thread started (ID: " << std::this_thread::get_id() << ")" << std::endl;

    while (running_.load()) {
        {
            std::lock_guard<std::mutex> cb_lock(callback_mutex_);
            if (data_callback_) {
                SensorData data;
                data.sensor_id = static_cast<uint8_t>(get_sensor_id());
                data.value = get_sensor_value();
                data_callback_(data);
            }
        }
        std::this_thread::sleep_for(get_sensor_response_time());
    }

    std::cout << "Worker thread exiting (ID: " << std::this_thread::get_id() << ")" << std::endl;
}

SensorId SensorDataSource::get_sensor_id() const {
    if (name_ == "temperature_sensor1") {
        return SensorId::Temperature1;
    } else if (name_ == "temperature_sensor2") {
        return SensorId::Temperature2;
    } else if (name_ == "speed_sensor1") {
        return SensorId::Speed1;
    } else if (name_ == "speed_sensor2") {
        return SensorId::Speed2;
    } else {
        return SensorId::Unknown;
    }
}

float SensorDataSource::get_sensor_value() const {
    if (name_ == "temperature_sensor1") {
        return 20.0f + static_cast<float>(rand() % 1000) / 100.0f; // 20.00 to 30.00 °C
    } else if (name_ == "temperature_sensor2") {
        return -(0.0f + static_cast<float>(rand() % 1000) / 100.0f); // -10.00 to -0.00 °C
    } else if (name_ == "speed_sensor1") {
        return 0.0f + static_cast<float>(rand() % 1000) / 100.0f; // 0.00 to 10.00 km/h
    } else if (name_ == "speed_sensor2") {
        return 100.0f + static_cast<float>(rand() % 1000) / 100.0f; // 100.00 to 110.00 km/h
    } else {
        return std::numeric_limits<float>::max();
    }
}

std::chrono::seconds SensorDataSource::get_sensor_response_time() const {
    if (name_ == "temperature_sensor1") {
        return std::chrono::seconds(3);
    } else if (name_ == "temperature_sensor2") {
        return std::chrono::seconds(1);
    } else if (name_ == "speed_sensor1") {
        return std::chrono::seconds(1);
    } else if (name_ == "speed_sensor2") {
        return std::chrono::seconds(5);
    } else {
        return std::chrono::seconds(1);
    }
}
