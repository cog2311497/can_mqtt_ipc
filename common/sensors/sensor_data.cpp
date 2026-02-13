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