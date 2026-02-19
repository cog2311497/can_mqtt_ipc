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

#include <iostream>
#include <memory>

#include "bridge.h"


int main(int argc, char* argv[])
{
    auto app = std::make_shared<Bridge>();
    Bridge::set_instance(app);

    if (!app->initialize(argc, argv)) {
        std::cerr << "Failed to initialize the bridge" << std::endl;
        return 1;
    }

    if (!app->start()) {
        std::cerr << "Failed to start the bridge" << std::endl;
        return 1;
    }

    app->wait();
    app->stop();

    return 0;
}