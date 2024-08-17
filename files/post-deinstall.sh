#!/bin/sh

# Ensure pkg is available
if ! command -v pkg >/dev/null 2>&1; then
    echo "Error: pkg is not installed."
    exit 1
fi

# Remove zerotier package
echo "Removing zerotier package..."
pkg delete -y zerotier
if [ $? -ne 0 ]; then
    echo "Failed to remove zerotier package"
    exit 1
fi

echo "zerotier package removed successfully."
exit 0