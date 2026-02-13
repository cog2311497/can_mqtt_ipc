
#include "config/config_parser.h"

#include <fstream>
#include <filesystem>
#include <iostream>

namespace {
    struct settings {
        std::string_view config_file;
    };

    std::optional<settings> parse_args(std::span<char*> args) {
        settings s;
        
        // Skip the first argument (program name)
        for (size_t i = 1; i < args.size(); ++i) {
            std::string_view arg = args[i];

            if (arg == "--config" || arg == "-c") {
                // Check if there's a next argument for the filename
                if (i + 1 < args.size()) {
                    s.config_file = args[++i];
                } else {
                    std::cerr << "Error: --config requires a filename\n";
                    return std::nullopt;
                }
            }
        }

        return s;
    }
}

std::optional<nlohmann::json> load_config(int argc, char* argv[]) {
    auto args = parse_args({argv, static_cast<size_t>(argc)});

    if (args && !args->config_file.empty()) {
        std::cout << "Loading config from: " << args->config_file << "\n";
    } else {
        std::cout << "No config file provided. Using defaults.\n";
        return std::nullopt;
    }

    std::ifstream config_file(std::filesystem::path(args->config_file));
    if (!config_file) {
        std::cerr << "Failed to open config file: " << args->config_file << "\n";
        return std::nullopt;
    }

    std::string content((std::istreambuf_iterator<char>(config_file)),
                         std::istreambuf_iterator<char>());

    if (nlohmann::json::accept(content)) {
        return nlohmann::json::parse(content);  // parse content into the json object
    } else {
        std::cerr << "Failed to parse config file: content is not valid JSON\n";
        return std::nullopt;
    }
}
