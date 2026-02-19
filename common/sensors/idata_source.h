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

#include <cstdint>
#include <functional>
#include <string>


template<typename T>
using DataCallback = std::function<void(const T&)>;


template<typename T>
class IDataSource {
public:
    virtual ~IDataSource() = default;

    virtual void register_callback(DataCallback<T> callback) = 0;

    virtual bool start() = 0;

    virtual void stop() = 0;

    virtual bool is_running() const = 0;

    virtual void wait() = 0;

    virtual const std::string& get_name() const = 0;
};
