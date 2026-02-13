# CAN MQTT IPC Bridge Demo

A system for bridging CAN interfaces with MQTT, featuring sensor data producer, MQTT bridge, and consumer/presenter components.

## Project Structure

- **producer/** — C++ app that generates sensor data and publishes to CAN interfaces
- **bridge/** — C++ app that bridges CAN messages to MQTT
- **presenter/** — Python app that subscribes to MQTT topics and displays messages
- **common/** — Shared headers for CAN frames, CAN reader/writer config parsing, and sensor data
- **services/** — Systemd unit files for process management
- **vcan/** — Virtual CAN network setup

## Prerequisites

### System Dependencies

```bash
# Ubuntu/Debian
sudo apt update
sudo apt install -y \
  build-essential \
  cmake \
  cmake-gui \
  libssl-dev \
  git \
  libpaho-mqtt-dev \
  can-utils \
  python3.12 \
  python3-pip \
  mosquitto \
  mosquitto-clients \
  python3-poetry

# clone and build and install paho.mqtt c++ wrapper
git clone https://github.com/eclipse/paho.mqtt.cpp.git
cd paho.mqtt.cpp
mkdir build && cd build
cmake ..
make
sudo make install
sudo ldconfig


### Virtual CAN Setup

Enable virtual CAN interfaces for testing (if you build and run apps manually, before call install):

```bash
# Create virtual CAN interfaces
sudo modprobe vcan
sudo ip link add dev vcan0 type vcan
sudo ip link add dev vcan1 type vcan
sudo ip link set up vcan0
sudo ip link set up vcan1

# Verify
ip link show | grep vcan

# check traffic on vcan0
candump vcan0
```

### Mosquitto MQTT Broker

```bash
# Start Mosquitto (if not running as systemd service)
sudo systemctl start mosquitto
sudo systemctl enable mosquitto

# Or run directly
mosquitto -c /etc/mosquitto/mosquitto.conf
```

## Build

### Build All Components

```bash
# Configure
mkdir -p build
cd build
cmake ..

# Build
cmake --build .
```

Binaries will be in:
- `build/bridge/bridge`
- `build/producer/producer`

### Build Individual Components

```bash
# Producer
cd build
cmake --build . --target producer

# Bridge
cmake --build . --target bridge
```

## Run

### Development (Direct Execution)

#### 1. Start Virtual CAN and Mosquitto

```bash
# Terminal 1: Ensure vcan and MQTT are running
sudo ip link set up vcan0
sudo ip link set up vcan1
mosquitto  # or sudo systemctl start mosquitto
```

#### 2. Run Producer

```bash
# Terminal 2: Start the producer (generates sensor data to CAN)
cd build
./producer/producer --config ../../../config.json
```

#### 3. Run Bridge

```bash
# Terminal 3: Start the bridge (CAN -> MQTT)
./bridge/bridge  --config ../../../config.json
```

#### 4. Run Presenter/Consumer

```bash
# Terminal 4: Start the presenter (subscribes to MQTT)
cd presenter
poetry install
poetry run python presenter/main.py --config ../config.json
```

#### 5. Monitor MQTT Messages

```bash
# Terminal 5: Subscribe to all MQTT topics
mosquitto_sub -t '#' -v
```

### Production (Systemd Services)

Install as systemd services:

```bash
sudo bash install.sh
```

This script:
- Installs binaries to `/opt/can_mqtt_ipc/`
- Installs systemd units in `/etc/systemd/system/`
- Enables and starts all services

### Service Management

```bash
# start/stop/restart services
sudo systemctl start vcan.service
sudo systemctl start producer.service
sudo systemctl start bridge.service
sudo systemctl start presenter.service

# View logs
sudo journalctl -u producer.service -f -b
sudo journalctl -u bridge.service -f -b
sudo journalctl -u presenter.service -f -b

# Enable at boot
sudo systemctl enable producer.service bridge.service presenter.service
```

## Configuration

Edit `config.json` to customize:
- CAN interfaces and message IDs
- Sensor data sources and bindings
- MQTT broker address and port
- Logger settings

---
**NOTE**

After change in `can_interfaces` also `vcan.service` must be aligned

---

Example:
```json
{
    "can_interfaces": ["vcan0", "vcan1"],
    "producer": { ... },
    "bridge": { ... },
    "presenter": { ... }
}
```

## Component Details

### Producer
- Reads sensor data (emulated)
- Maps data to CAN frame format
- Publishes to CAN interfaces

### Bridge
- Subscribes to CAN message traffic
- Parses CAN frames
- Publishes to MQTT topics

### Presenter (Consumer)
- Subscribes to MQTT topics
- Logs received messages to console and file
- Configured via `--config` argument


## License

Cognizant
