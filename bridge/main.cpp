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