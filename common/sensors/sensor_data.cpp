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

#include "sensors/sensors_data.h"


std::string sensor_id_to_string(SensorId id) {
    switch (id) {
        case SensorId::Temperature1: return "temperature_sensor1";
        case SensorId::Temperature2: return "temperature_sensor2";
        case SensorId::Speed1: return "speed_sensor1";
        case SensorId::Speed2: return "speed_sensor2";
        default: return "unknown_sensor";
    }
}

std::string sensor_id_to_units(SensorId id) {
    switch (id) {
        case SensorId::Temperature1:
        case SensorId::Temperature2:
            return "°C";
        case SensorId::Speed1:
        case SensorId::Speed2:
            return "km/h";
        default:
            return "";
    }
}

std::string sensor_id_to_type(SensorId id) {
    switch (id) {
        case SensorId::Temperature1:
        case SensorId::Temperature2:
            return "temperature";
        case SensorId::Speed1:
        case SensorId::Speed2:
            return "speed";
        default:
            return "";
    }
}