#!/bin/sh

set -e

# Check if Homebrew is installed and install
if ! command -v brew &> /dev/null; then
    echo "Homebrew not found. Installing Homebrew..."
    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
fi

# Check if Python is installed and install
if ! command -v python3 &> /dev/null; then
    echo "Python not found. Installing Python..."
    if ! brew install python; then
        echo "Failed to install Python. Exiting..."
        exit 1
    fi
fi

# Check if PlatformIO Core is installed and install
if ! command -v platformio &> /dev/null; then
    echo "PlatformIO Core not found. Installing PlatformIO Core..."
    if ! brew install platformio; then
        echo "Failed to install PlatformIO Core. Exiting..."
        exit 1
    fi
fi

