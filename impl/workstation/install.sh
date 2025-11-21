#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SERVICE_NAME="deauth-lock.service"
SERVICE_FILE="$SCRIPT_DIR/$SERVICE_NAME"
SYSTEMD_USER_DIR="$HOME/.config/systemd/user"

echo "Installing DeAuth Lock service..."

mkdir -p "$SYSTEMD_USER_DIR"

sed "s|%h|$HOME|g" "$SERVICE_FILE" > "$SYSTEMD_USER_DIR/$SERVICE_NAME"

# Make scripts executable
chmod +x "$SCRIPT_DIR/client.py"
chmod +x "$SCRIPT_DIR/deauth.py"

systemctl --user daemon-reload
systemctl --user enable "$SERVICE_NAME"
systemctl --user start "$SERVICE_NAME"

echo "To check status: systemctl --user status $SERVICE_NAME"
echo "To stop: systemctl --user stop $SERVICE_NAME"
echo "To disable: systemctl --user disable $SERVICE_NAME"