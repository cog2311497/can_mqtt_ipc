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
#include <string>


enum class SensorId : uint8_t {
    Unknown = 0,
    Temperature1 = 1,
    Temperature2 = 2,
    Speed1 = 8,
    Speed2 = 9
};

struct SensorData {
    uint8_t sensor_id;
    float value;
};


std::string sensor_id_to_string(SensorId id);

std::string sensor_id_to_units(SensorId id);

std::string sensor_id_to_type(SensorId id);
