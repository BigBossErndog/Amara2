#!/bin/bash

# Check for dependencies
if ! command -v make &> /dev/null
then
    echo "make could not be found. Please install it to continue."
    exit 1
fi

if ! command -v clang &> /dev/null
then
    echo "clang could not be found. Please install it to continue."
    exit 1
fi

# Check for SDL3 development library
if ! dpkg -l | grep -q libsdl3-dev; then
    echo "SDL3 development library (libsdl3-dev) not found."
    echo "Please install it to continue."
    echo "For Debian-based systems (like Ubuntu), run: sudo apt-get install libsdl3-dev"
    echo "For Fedora/CentOS, you might need to find a similar package using dnf or yum."
    echo "For Arch Linux, you might find it in the AUR or a community repository."
    exit 1
fi

# Build the project using the Makefile's linux target
echo "Building project for Linux..."
make linux

if [ $? -eq 0 ]; then
    echo "Build successful."
else
    echo "Build failed."
fi
