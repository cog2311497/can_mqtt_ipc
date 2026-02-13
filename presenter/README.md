# Presenter

Data presenter for CAN MQTT IPC system. Subscribes to MQTT topics and displays messages.

## Setup

```bash
poetry install
```

## Run

```bash
poetry run python presenter/main.py --config config.json
```

## Systemd

See `services/presenter.service` for systemd unit file.
