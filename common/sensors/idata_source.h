#pragma once

#include <cstdint>
#include <functional>
#include <string>


template<typename T>
using DataCallback = std::function<void(const T&)>;


template<typename T>
class IDataSource {
public:
    virtual ~IDataSource() = default;

    virtual void register_callback(DataCallback<T> callback) = 0;

    virtual bool start() = 0;

    virtual void stop() = 0;

    virtual bool is_running() const = 0;

    virtual void wait() = 0;

    virtual const std::string& get_name() const = 0;
};
