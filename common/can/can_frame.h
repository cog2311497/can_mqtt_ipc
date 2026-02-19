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
#include <vector>

struct CanFrame {
    uint32_t id;                    // 11-bit (standard) or 29-bit (extended)
    std::vector<uint8_t> data;      // 0 .. 8 bytes (CAN), up to 64 for CAN-FD
    bool is_extended{false};        // true if this is an extended frame (29-bit ID)
    bool is_fd{false};              // true if this is a CAN-FD frame
    bool is_rtr{false};             // true if this is a Remote Transmission Request frame
};

