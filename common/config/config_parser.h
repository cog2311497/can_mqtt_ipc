#pragma once

#include <optional>

#include <nlohmann/json.hpp>


std::optional<nlohmann::json> load_config(int argc, char* argv[]);
