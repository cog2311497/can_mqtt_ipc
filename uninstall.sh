
#!/usr/bin/env bash
set -euo pipefail

DEST="/opt/can_mqtt_ipc"
SYS_DEST="/etc/systemd/system"

echo "Stopping and disabling services..."

# Read interfaces from config if available
INTERFACES=""
if [[ -f ./config.json ]]; then
  INTERFACES=$(jq -r '.can_interfaces[]' "./config.json" 2>/dev/null || true)
fi

# Stop services
sudo systemctl stop presenter.service || true
sudo systemctl stop bridge.service || true
sudo systemctl stop producer.service || true

for iface in $INTERFACES; do
  sudo systemctl stop "vcan-iface@${iface}.service" || true
done

sudo systemctl stop vcan.service || true

# Disable services
sudo systemctl disable presenter.service || true
sudo systemctl disable bridge.service || true
sudo systemctl disable producer.service || true

for iface in $INTERFACES; do
  sudo systemctl disable "vcan-iface@${iface}.service" || true
done

sudo systemctl disable vcan.service || true

echo "Removing systemd unit files..."
sudo rm -f \
  "$SYS_DEST/vcan.service" \
  "$SYS_DEST/vcan-iface.target" \
  "$SYS_DEST/vcan-iface@.service" \
  "$SYS_DEST/producer.service" \
  "$SYS_DEST/bridge.service" \
  "$SYS_DEST/presenter.service"

echo "Reloading systemd..."
sudo systemctl daemon-reload
sudo systemctl reset-failed || true

echo "Removing application directory $DEST"
sudo rm -rf "$DEST"

echo "Uninstall complete."
