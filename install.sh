#!/bin/bash
# Script to install NemoS on Linux.
# Please run this script in the Terminal application.
# Ensure the script is being run with root privileges.
if [[ $EUID -ne 0 ]]; then
  echo "Error: This script must be run as root." >&2
  exit 1
fi

# Check if the binary exists.
if [[ ! -f "nemos" ]]; then
  echo "Error: 'nemos' binary not found in the current directory. Please download the binary." >&2
  exit 1
fi
# Installing xclip and lpr.
sudo apt install xclip lpr
sudo packman -S install xclip lpr
sudo dnf install xclip lpr
sudo zypper install xclip lpr
sudo xbps-install -S xclip lpr
sudo apk add xclip lpr
sudo emerge xclip lpr

echo "If the install for xclip and lpr is not working and dont have it installed. Please look online installing xclip and lpr for your distribution!"
# Copy the binary to /usr/local/bin
cp nemos /usr/local/bin/
chmod +x /usr/local/bin/nemos

# Notify the user of successful installation
echo "NemoS has been installed successfully!"
echo "You can now run it from any directory using the command: 'nemos' or 'nemos file.txt'." 
