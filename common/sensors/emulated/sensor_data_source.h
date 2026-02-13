#pragma once

#include <thread>
#include <mutex>
#include <atomic>
#include <memory>
#include <chrono>

#include "sensors/idata_source.h"
#include "sensors/sensors_data.h"


class SensorDataSource : public IDataSource<SensorData> {
public:
    SensorDataSource(std::string name);
    ~SensorDataSource() override;

    void register_callback(DataCallback<SensorData> callback) override;

    bool start() override;

    void stop() override;

    bool is_running() const override;

    void wait() override;

    const std::string& get_name() const override {
        return name_;
    }

protected:
    void thread_worker();
    SensorId get_sensor_id() const;
    float get_sensor_value() const;
    std::chrono::seconds get_sensor_response_time() const;

private:
    std::unique_ptr<std::thread> worker_thread_;
    std::atomic<bool> running_{false};
    std::mutex callback_mutex_;
    DataCallback<SensorData> data_callback_;
    std::string name_;

};
