#!/bin/bash

# Script to install NemoS

# Ensure the script is being run with root privileges
if [[ $EUID -ne 0 ]]; then
  echo "Error 1: This script must be run as root." >&2
  exit 1
fi

# Check if the binary exists
if [[ ! -f "nemos" ]]; then
  echo "Error: 'nemos' binary not found in the current directory. Please compile the program first." >&2
  exit 1
fi
# Installing xclip
sudo apt install xclip
sudo packman -S install xclip
sudo dnf install xclip
sudo zypper install xclip
sudo xbps-install -S xclip
sudo apk add xclip
sudo emerge xclip

echo "If the install for xclip is not working and dont have it installed. Please look online installing xclip for your distrobution!"
# Copy the binary to /usr/local/bin
cp nemos /usr/local/bin/
chmod +x /usr/local/bin/nemos

# Notify the user of successful installation
echo "NemoS has been installed successfully!"
echo "You can now run it from any directory using the command: nemos"
