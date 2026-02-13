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
