#include "can/linux/sockets/can_receiver.h"

#include <cstring>
#include <algorithm>
#include <iostream>
#include <unistd.h>
#include <cerrno>
#include <poll.h>

#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>



bool LinuxSocketCanReceiver::open()
{
    if (is_open())
        return true;

    socket_fd_ = ::socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (socket_fd_ < 0)
        return false;

    struct ifreq ifr {};
    std::strncpy(ifr.ifr_name, ifname_.c_str(), IFNAMSIZ - 1);

    if (ioctl(socket_fd_, SIOCGIFINDEX, &ifr) < 0)
    {
        ::close(socket_fd_);
        socket_fd_ = -1;
        return false;
    }

    sockaddr_can addr {};
    addr.can_family  = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(socket_fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0)
    {
        ::close(socket_fd_);
        socket_fd_ = -1;
        return false;
    }

    return true;
}

bool LinuxSocketCanReceiver::start()
{
    running_.store(true);
    worker_ = std::thread(&LinuxSocketCanReceiver::receive_loop, this);

    return true;
}

void LinuxSocketCanReceiver::stop()
{
    running_.store(false);
}

void LinuxSocketCanReceiver::close()
{
    running_.store(false);

    if (socket_fd_ >= 0)
    {
        ::shutdown(socket_fd_, SHUT_RDWR);
    }

    if (worker_.joinable())
        worker_.join();

    if (socket_fd_ >= 0)
    {
        ::close(socket_fd_);
        socket_fd_ = -1;
    }
}

void LinuxSocketCanReceiver::wait() {
    if (worker_.joinable()) {
        worker_.join();
    }
}

ICanReceiver::SubscriptionPtr LinuxSocketCanReceiver::subscribe(Callback cb)
{
    std::lock_guard lock(mutex_);
    auto id = ++next_id_;
    subscribers_.emplace_back(id, std::move(cb));

    struct SubImpl : Subscription
    {
        SubImpl(LinuxSocketCanReceiver* p, uint64_t id)
            : parent(p), id(id) {}

        ~SubImpl()
        {
            if (parent)
                parent->unsubscribe(id);
        }

        LinuxSocketCanReceiver* parent;
        uint64_t id;
    };

    return std::make_unique<SubImpl>(this, id);
}

void LinuxSocketCanReceiver::unsubscribe(uint64_t id)
{
    std::lock_guard lock(mutex_);
    subscribers_.erase(
        std::remove_if(subscribers_.begin(), subscribers_.end(), [id](auto& s) { return s.first == id; }), subscribers_.end());
}

void LinuxSocketCanReceiver::receive_loop()
{
    struct pollfd pfd{};

    while (running_.load())
    {
        if (socket_fd_ < 0) {
            // Socket closed or invalid; wait a bit and retry
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        pfd.fd = socket_fd_;
        pfd.events = POLLIN | POLLPRI;

        int ret = ::poll(&pfd, 1, 1000); // Infinite timeout
        if (ret < 0) {
            if (errno == EINTR)
                continue;
            std::cerr << "poll() error: " << std::strerror(errno) << std::endl;
            break;
        }

        if (ret == 0)
            continue; // timeout, loop again

        if (pfd.revents & (POLLERR | POLLNVAL)) {
            std::cerr << "poll() returned error on socket" << std::endl;
            break;
        }

        if (pfd.revents & (POLLIN | POLLPRI)) {
            struct can_frame frame {};
            ssize_t n = ::read(socket_fd_, &frame, sizeof(frame));
            if (n <= 0)
                continue;

            CanFrame f;
            f.id = frame.can_id & CAN_EFF_MASK;
            f.is_extended = frame.can_id & CAN_EFF_FLAG;
            f.is_rtr = frame.can_id & CAN_RTR_FLAG;
            f.data.resize(frame.len);

            std::memcpy(f.data.data(), frame.data, frame.len);

            std::vector<Callback> callbacks_copy;

            {
                std::lock_guard lock(mutex_);
                for (auto& [_, cb] : subscribers_)
                    callbacks_copy.push_back(cb);
            }

            for (auto& cb : callbacks_copy) {
                try {
                    cb(f);
                } catch (const std::exception& e) {
                    std::cerr << "Subscriber callback threw: " << e.what() << std::endl;
                } catch (...) {
                    std::cerr << "Subscriber callback threw unknown exception" << std::endl;
                }
            }
        }
    }
    std::cout << "Worker thread exiting" << std::endl;
}
