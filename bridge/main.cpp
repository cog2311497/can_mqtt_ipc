#include <iostream>
#include <csignal>
#include <format>

#include <mqtt/async_client.h>
#include <nlohmann/json.hpp>

#include "can/linux/sockets/can_receiver.h"
#include "config/config_parser.h"
#include "sensors/sensors_data.h"

std::map<std::string, std::shared_ptr<ICanReceiver>> can_readers;

void signal_handler(int signum) {
    if (signum == SIGTERM || signum == SIGINT) {
        std::cout << "Received stop signal (SIGTERM or SIGINT). Shutting down gracefully..." << std::endl;
        for(const auto& [name, reader] : can_readers) {
            reader->stop();
        }
    }
}


int main(int argc, char* argv[])
{
    // Register signal handlers for termination signals
    signal(SIGTERM, signal_handler);  // Handle service stop signal
    signal(SIGINT, signal_handler);   // Handle Ctrl+C in terminal

    auto config_opt = load_config(argc, argv);
    if (!config_opt) {
        std::cerr << "Using default configuration\n";
        return 1;
    }
    nlohmann::json config = *config_opt;

    if (!config.contains("can_interfaces") || !config.contains("bridge") || !config["bridge"].contains("mqtt_broker") || !config["bridge"].contains("mqtt_port") || !config["bridge"].contains("mqtt_topics")) {
        std::cerr << "Invalid config file structure\n";
        return 1;
    }

    const std::string mgtt_broker = config["bridge"]["mqtt_broker"];
    const int mqtt_port = config["bridge"]["mqtt_port"];
    const std::vector<std::string> mqtt_topics = config["bridge"]["mqtt_topics"];
    const std::string server_address = "mqtt://" + mgtt_broker + ":" + std::to_string(mqtt_port);

    mqtt::async_client client(server_address, "bridge_client_1");
    mqtt::connect_options conn_opts;
    conn_opts.set_clean_session(true);
    try
    {

        client.connect(conn_opts)->wait();
        std::cout << "Connected to broker\n";
    }
    catch (const mqtt::exception& e)
    {
        std::cerr << "MQTT Error: " << e.what() << "\n";
        return 1;
    }


    {
        // Set up CAN readers for each unique CAN interface in the bindings
        std::vector<ICanReceiver::SubscriptionPtr> subscriptions; // Keep subscriptions alive
        for(const auto& can_interface  : config["can_interfaces"]) {
            std::cout << "Setting up CAN interface: " << can_interface << "\n";
            can_readers[can_interface] = std::make_shared<LinuxSocketCanReceiver>(can_interface);

            auto sub = can_readers[can_interface]->subscribe([&](const CanFrame& f)
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
                    std::cout << "Parsed Sensor Data - ID: " << static_cast<int>(data.sensor_id) << " Value: " << data.value << "\n";
                } else {
                    std::cerr << "Received CAN frame with insufficient data length\n";
                    return;
                }
                
                // Create JSON payload
                nlohmann::json j;
                j["device"] = sensor_id_to_string(static_cast<SensorId>(data.sensor_id));
                j["value"] = std::format("{:.2f}", data.value);
                j["unit"] = sensor_id_to_units(static_cast<SensorId>(data.sensor_id));

                std::string payload = j.dump();

                auto sensor_type = sensor_id_to_type(static_cast<SensorId>(data.sensor_id));
                if (sensor_type.empty()) {
                    std::cerr << "Unknown sensor type for sensor ID: " << static_cast<int>(data.sensor_id) << "\n";
                    return;
                }

                std::string topic;
                for(const auto& t : mqtt_topics) {
                    if (t.find(sensor_type) != std::string::npos) {
                        topic = t;
                        break;
                    }
                }

                if (topic.empty()) {
                    std::cerr << "No MQTT topic found for sensor type: " << sensor_type << "\n";
                    return;
                }

                // Create message
                auto msg = mqtt::make_message(topic, payload);
                msg->set_qos(1);   // QoS 0, 1, or 2
                msg->set_retained(false);

                // Publish
                client.publish(msg)->wait();
                std::cout << "Message published:\n" << payload << "\n";
            });
            subscriptions.push_back(std::move(sub)); // Keep subscription alive

            can_readers[can_interface]->open();
            can_readers[can_interface]->start();
        }

        for(const auto& [name, reader] : can_readers) {
            reader->wait(); // Wait for the receiver thread to finish (if applicable)
            reader->close();
        }
    }


    try
    {
        client.disconnect()->wait();
    }
    catch (const mqtt::exception& e)
    {
        std::cerr << "MQTT Error: " << e.what() << "\n";
        return 1;
    }

}