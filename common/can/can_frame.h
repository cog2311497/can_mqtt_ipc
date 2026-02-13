#pragma once

#include <cstdint>
#include <vector>

struct CanFrame {
    uint32_t id;                    // 11-bit (standard) or 29-bit (extended)
    std::vector<uint8_t> data;      // 0 .. 8 bytes (CAN), up to 64 for CAN-FD
    bool is_extended{false};        // true if this is an extended frame (29-bit ID)
    bool is_fd{false};              // true if this is a CAN-FD frame
    bool is_rtr{false};             // true if this is a Remote Transmission Request frame
};

