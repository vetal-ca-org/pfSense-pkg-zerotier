#!/bin/sh

# Ensure pkg is available
if ! command -v pkg >/dev/null 2>&1; then
    echo "Error: pkg is not installed."
    exit 1
fi

# Install zerotier package
echo "Installing zerotier-${ZEROTIER_VERSION}.pkg..."
pkg add https://pkg.freebsd.org/FreeBSD:14:amd64/latest/All/zerotier-${ZEROTIER_VERSION}.pkg
if [ $? -ne 0 ]; then
    echo "Failed to install zerotier-${ZEROTIER_VERSION}.pkg"
    exit 1
fi

echo "zerotier-${ZEROTIER_VERSION}.pkg installed successfully."
exit 0