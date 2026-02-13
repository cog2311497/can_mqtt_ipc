#include <string>
#include <vector>
#include <map>
#include <memory>
#include <iostream>
#include <cstdint>
#include <csignal>

#include <nlohmann/json.hpp>

#include "can/linux/sockets/can_sender.h"
#include "sensors/idata_source.h"
#include "sensors/sensors_data.h"
#include "sensors/emulated/sensor_data_source.h"
#include "config/config_parser.h"


struct DataBinding {
    std::string data_source;
    std::string can_interface;
    uint32_t can_msg_id;
};

std::vector<std::shared_ptr<IDataSource<SensorData>>> data_sources;


void signal_handler(int signum) {
    if (signum == SIGTERM || signum == SIGINT) {
        std::cout << "Received stop signal (SIGTERM or SIGINT). Shutting down gracefully..." << std::endl;
        for(auto& data_source : data_sources) {
            data_source->stop();
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

    if (!config.contains("can_interfaces") || !config.contains("producer") || !config["producer"].contains("data_binding")) {
        std::cerr << "Invalid config file structure\n";
        return 1;
    }

    nlohmann::json data_binding = config["producer"]["data_binding"];
    std::vector<DataBinding> bindings;

    // load data bindings from config
    // maps a data source (e.g. "temperature_sensor") to a CAN interface (e.g. "can0") and message ID (e.g. 0x100)
    for (auto& item : data_binding) {
        DataBinding binding;
        if (!(item.contains("source") && item.contains("destination") && item["destination"].contains("interface") && item["destination"].contains("msg_id"))) {
            std::cerr << "Invalid data_binding entry in config\n";
            continue;
        }
        binding.data_source = item["source"];
        binding.can_interface = item["destination"]["interface"];
        binding.can_msg_id =  std::stoul(item["destination"]["msg_id"].get<std::string>(), nullptr, 16);
        bindings.push_back(binding);
        std::cout << binding.data_source << " -> " << binding.can_interface << " (0x" << std::hex << binding.can_msg_id << std::dec << ")\n";
    }

    if (bindings.empty()) {
        std::cerr << "No valid data bindings found in config\n";
        return 1;
    }

    // Set up CAN senders for each unique CAN interface in the bindings
    std::map<std::string, std::shared_ptr<ICanSender>> can_senders;
    for(const auto& can_interface  : config["can_interfaces"]) {
        std::cout << "Setting up CAN interface: " << can_interface << "\n";
        can_senders[can_interface] = std::make_shared<LinuxSocketCanSender>(can_interface);
        can_senders[can_interface]->open();
    }


    // Set up data sources and register callbacks to send CAN frames when new data is received
    for (const auto& binding : bindings) {
        auto data_source = std::make_shared<SensorDataSource>(binding.data_source);
        data_source->register_callback([&binding, &can_senders](const SensorData& data) {
            std::cout << "Received data from " << binding.data_source << ": Sensor ID=" << static_cast<int>(data.sensor_id) << " Value=" << data.value << "\n";
            if (can_senders.find(binding.can_interface) != can_senders.end()) {
                CanFrame frame;
                frame.id = binding.can_msg_id;
                //frame.is_extended = binding.can_msg_id > CAN_SFF_MASK;   ?????????????????????????????
                frame.is_rtr = false;
                // Simple encoding: 1 bytes for sensor_id, 4 bytes for value
                frame.data.resize(5);
                std::memcpy(frame.data.data(), &data.sensor_id, sizeof(data.sensor_id));
                std::memcpy(frame.data.data() + sizeof(data.sensor_id), &data.value, sizeof(data.value));
                
                if (!can_senders[binding.can_interface]->send(frame)) {
                    std::cerr << "Failed to send CAN frame on " << binding.can_interface << "\n";
                } else {
                    std::cout << "Sent CAN frame on " << binding.can_interface << ": ID=0x" << std::hex << frame.id << std::dec 
                              << " Data(" << frame.data.size() << " bytes)\n";
                }
            } else {
                std::cerr << "CAN interface not found for binding: " << binding.can_interface << "\n";
            }
        });
        data_source->start();
        data_sources.push_back(std::move(data_source));
    }

    // Wait util all data sources have finished (in this example, 
    // they run indefinitely, so this will block until the program is terminated)
    for (auto& data_source : data_sources) {
        data_source->wait();
    }

    return 0;
}
