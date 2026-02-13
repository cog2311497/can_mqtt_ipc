#include "bridge.h"

#include <iostream>
#include <csignal>
#include <format>

#include "config/config_parser.h"
#include "sensors/sensors_data.h"


Bridge::Bridge() {
    signal(SIGTERM, Bridge::signal_handler);  // Handle service stop signal
    signal(SIGINT, Bridge::signal_handler);   // Handle Ctrl+C in terminal
}

bool Bridge::initialize(int argc, char* argv[])
{
    auto config_opt = load_config(argc, argv);
    if (!config_opt) {
        std::cerr << "Failed to load configuration" << std::endl;
        return false;
    }
    config_ = *config_opt;

    if (!config_.contains("can_interfaces") || !config_.contains("bridge") || !config_["bridge"].contains("mqtt_broker") || !config_["bridge"].contains("mqtt_port") || !config_["bridge"].contains("mqtt_topics")) {
        std::cerr << "Invalid config file structure" << std::endl;
        return false;
    }
    return true;
}

bool Bridge::start()
{
    if (!connect_mqtt()) {
        return false;
    }

    if (!setup_can_readers()) {
        return false;
    }

    return true;
}

void Bridge::stop()
{
    if (client_) {
        try {
            client_->disconnect()->wait();
            std::cout << "Disconnected from broker" << std::endl;
        }
        catch (const mqtt::exception& e) {
            std::cerr << "MQTT Error during disconnect: " << e.what() << std::endl;
        }
    }
}


bool Bridge::connect_mqtt()
{
    const std::string mgtt_broker = config_["bridge"]["mqtt_broker"];
    const int mqtt_port = config_["bridge"]["mqtt_port"];
    mqtt_topics_ = config_["bridge"]["mqtt_topics"];
    const std::string server_address = "mqtt://" + mgtt_broker + ":" + std::to_string(mqtt_port);

    client_ = std::make_unique<mqtt::async_client>(server_address, "bridge_client_1");
    
    mqtt::connect_options conn_opts;
    conn_opts.set_clean_session(true);
    try
    {
        client_->connect(conn_opts)->wait();
        std::cout << "Connected to broker" << std::endl;
    }
    catch (const mqtt::exception& e)
    {
        std::cerr << "MQTT Error: " << e.what() << std::endl;
        return false;
    }
    return true;
}

bool Bridge::setup_can_readers()
{
    // Set up CAN readers for each unique CAN interface in the bindings
    for(const auto& can_interface  : config_["can_interfaces"]) {
        std::cout << "Setting up CAN interface: " << can_interface << std::endl;
        can_receivers_[can_interface] = std::make_shared<LinuxSocketCanReceiver>(can_interface);

        auto sub = can_receivers_[can_interface]->subscribe([this](const CanFrame& f)
        {
            std::cout << "ID: 0x" << std::hex << f.id << " len: " << f.data.size() << " data:";
            for(size_t i = 0; i < f.data.size(); ++i)
            {
                std::cout << " " << std::hex << static_cast<int>(f.data[i]);
            }
            std::cout << std::endl;

            SensorData data;
            if (f.data.size() >= 5) { // Expecting at least 5 bytes: 1 for sensor_id and 4 for value
                std::memcpy(&data.sensor_id, f.data.data(), sizeof(data.sensor_id));
                std::memcpy(&data.value, f.data.data() + sizeof(data.sensor_id), sizeof(data.value));
                std::cout << "Parsed Sensor Data - ID: " << static_cast<int>(data.sensor_id) << " Value: " << data.value << std::endl;
            } else {
                std::cerr << "Received CAN frame with insufficient data length" << std::endl;
                return;
            }
            
            nlohmann::json j;
            j["device"] = sensor_id_to_string(static_cast<SensorId>(data.sensor_id));
            j["value"] = std::format("{:.2f}", data.value);
            j["unit"] = sensor_id_to_units(static_cast<SensorId>(data.sensor_id));

            std::string payload = j.dump();

            auto sensor_type = sensor_id_to_type(static_cast<SensorId>(data.sensor_id));
            if (sensor_type.empty()) {
                std::cerr << "Unknown sensor type for sensor ID: " << static_cast<int>(data.sensor_id) << std::endl;
                return;
            }

            std::string topic;
            for(const auto& t : mqtt_topics_) {
                if (t.find(sensor_type) != std::string::npos) {
                    topic = t;
                    break;
                }
            }

            if (topic.empty()) {
                std::cerr << "No MQTT topic found for sensor type: " << sensor_type << std::endl;
                return;
            }

            auto msg = mqtt::make_message(topic, payload);
            msg->set_qos(1);
            msg->set_retained(false);

            {
                std::lock_guard<std::mutex> lock(mqtt_mutex_);
                if (client_) {
                    client_->publish(msg)->wait();
                    std::cout << "Message published: " << payload << std::endl;
                } else {
                    std::cerr << "MQTT client not initialized" << std::endl;
                }
            }
        });
        subscriptions_.push_back(std::move(sub)); // Keep subscription alive

        can_receivers_[can_interface]->open();
        can_receivers_[can_interface]->start();
    }
    return true;
}

void Bridge::wait() {
    for(const auto& [name, reader] : can_receivers_) {
        reader->wait();
        reader->close();
    }
    subscriptions_.clear();
}


void Bridge::signal_handler(int signum) {
    if (signum == SIGTERM || signum == SIGINT) {
        std::cout << "Received stop signal (SIGTERM or SIGINT). Shutting down gracefully..." << std::endl;
        if (instance_) {
            for(const auto& [name, reader] : instance_->can_receivers_) {
                reader->stop();
            }
        }
    }
}

std::shared_ptr<Bridge> Bridge::instance_ = nullptr;
