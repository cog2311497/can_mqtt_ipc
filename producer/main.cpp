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

#include <memory>
#include <iostream>

#include "producer.h"

int main(int argc, char* argv[])
{
    // fix: producer log not interleave with other logs - just for better readability during development, can be removed later
    std::setvbuf(stdout, nullptr, _IOLBF, 0);
    std::setvbuf(stderr, nullptr, _IOLBF, 0);

    auto app = std::make_shared<Producer>();
    Producer::set_instance(app);

    if (!app->initialize(argc, argv)) {
        std::cerr << "Failed to initialize the producer" << std::endl;
        return 1;
    }

    if (!app->start()) {
        std::cerr << "Failed to start the producer" << std::endl;
        return 1;
    }

    app->wait();    

    return 0;
}
