
#!/usr/bin/env bash
set -euo pipefail

DEST="/opt/can_mqtt_ipc"
SYS_DEST="/etc/systemd/system"

echo "Installing to $DEST..."

# Clean destination
sudo mkdir -p "$DEST"
sudo rm -rf "$DEST"/*

sudo mkdir -p \
  "$DEST/presenter" \
  "$DEST/bridge" \
  "$DEST/producer"

copy_file() {
  local src="$1" dst="$2"
  sudo install -m 644 "$src" "$dst"
  echo "Copied: $src -> $dst"
}

copy_exec() {
  local src="$1" dst="$2"
  sudo install -m 755 "$src" "$dst"
  echo "Installed executable: $src -> $dst"
}

copy_dir() {
  local src="$1" dst="$2"
  sudo cp -r "$src"/* "$dst/"
  echo "Copied directory: $src -> $dst"
}

# Runtime files
copy_file config.json "$DEST/"

copy_exec build/bridge/bridge "$DEST/bridge/"
copy_exec build/producer/producer "$DEST/producer/"

copy_file presenter/presenter/main.py "$DEST/presenter/"

# Virtual environment
sudo mkdir -p "$DEST/presenter/venv"
venv_dir=$(echo build/presenter/presenter-*-py* 2>/dev/null || true)
if [[ -d "$venv_dir" ]]; then
  copy_dir "$venv_dir" "$DEST/presenter/venv"
else
  echo "Error: No presenter virtual environment found. Aborting."
  exit 1
fi

# Systemd services
copy_file services/vcan.service           "$SYS_DEST/vcan.service"
copy_file services/vcan-iface.target     "$SYS_DEST/vcan-iface.target"
copy_file services/vcan-iface@.service   "$SYS_DEST/vcan-iface@.service"
copy_file services/producer.service      "$SYS_DEST/producer.service"
copy_file services/bridge.service        "$SYS_DEST/bridge.service"
copy_file services/presenter.service     "$SYS_DEST/presenter.service"

# Reload systemd
echo "Reloading systemd..."
sudo systemctl daemon-reload

# Enable interface list
INTERFACES=$(jq -r '.can_interfaces[]' "./config.json" 2>/dev/null || true)

echo "Enabling and starting services..."
sudo systemctl enable --now vcan.service

for iface in $INTERFACES; do
  sudo systemctl enable --now "vcan-iface@${iface}.service"
done

sudo systemctl enable --now producer.service
sudo systemctl enable --now bridge.service
sudo systemctl enable --now presenter.service

echo "Installation complete."
