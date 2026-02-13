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
