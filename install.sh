#!/usr/bin/bash

set -euo pipefail

# Install runtime files to /opt/can_mqtt_ipc
# Copies:
#  - config.json
#  - bridge/build/bridge -> /opt/can_mqtt_ipc/bin/bridge
#  - producer/build/producer -> /opt/can_mqtt_ipc/bin/producer
#  - presenter/presenter/ -> /opt/can_mqtt_ipc/presenter/

DEST=/opt/can_mqtt_ipc

echo "Installing to $DEST (requires root)"

if [ "$EUID" -ne 0 ]; then
  echo "This script should be run as root (or via sudo)." >&2
  echo "Run: sudo $0" >&2
  exit 1
fi

rm -rf "$DEST"/{*,.*} 2>/dev/null

mkdir -p "$DEST/presenter"
mkdir -p "$DEST/bridge"
mkdir -p "$DEST/producer"

copy_files() {
  local src="$1" dst="$2"
  sudo cp "$src" "$dst"
  echo "Copied: $src -> $dst"
}

copy_dir() {
  local src="$1" dst="$2"
  sudo cp -r "$src"/* "$dst"
  echo "Copied: $src -> $dst"
}

copy_files config.json "$DEST/"
copy_files build/bridge/bridge "$DEST/bridge/"
copy_files build/producer/producer "$DEST/producer/"
copy_files presenter/presenter/main.py "$DEST/presenter/"

chmod +x "$DEST/bridge/bridge"
chmod +x "$DEST/producer/producer"
chmod +x "$DEST/presenter/main.py"

copy_files services/vcan.service /etc/systemd/system/vcan.service
copy_files services/vcan-iface.target /etc/systemd/system/vcan-iface.target
copy_files services/vcan-iface@.service /etc/systemd/system/vcan-iface@.service
copy_files services/producer.service /etc/systemd/system/producer.service
copy_files services/bridge.service /etc/systemd/system/bridge.service
copy_files services/presenter.service /etc/systemd/system/presenter.service

mkdir -p "$DEST/presenter/venv"
venv_dir=$(echo build/presenter/presenter-*-py*)
if [ -d "$venv_dir" ]; then
  copy_dir "$venv_dir" "$DEST/presenter/venv"
else
  echo "Erro: No presenter virtual environment found. Skipping."
  exit 1
fi

INTERFACES=$(jq -r '.can_interfaces[]' "./config.json" 2>/dev/null)

# Enable start services, make auto-start on boot
echo "Enabling and starting services..."
sudo systemctl daemon-reload
sudo systemctl enable --now vcan.service
for iface in $INTERFACES; do
    sudo systemctl enable --now "vcan-iface@${iface}.service"
done
sudo systemctl enable --now producer.service
sudo systemctl enable --now bridge.service
sudo systemctl enable --now presenter.service


