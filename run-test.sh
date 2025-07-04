#!/bin/bash

# Linux AI Mode Test Runner
echo "=== Linux AI Mode Test Runner ==="

# Check if we're in a graphical environment
if [ -z "$DISPLAY" ] && [ -z "$WAYLAND_DISPLAY" ]; then
    echo "Warning: No display environment detected."
    echo "This application requires a graphical environment (X11 or Wayland)."
    exit 1
fi

# Set up environment
export GDK_BACKEND=x11
echo "Using display: ${DISPLAY:-$WAYLAND_DISPLAY}"

# Run the application
echo "Starting Linux AI Mode..."
cd /home/mahi-pasarkar/Github/Linux-AI-Mode/build

if [ -f "./linux-ai-mode" ]; then
    echo "Executable found, launching..."
    exec ./linux-ai-mode "$@"
else
    echo "Error: linux-ai-mode executable not found in build directory"
    exit 1
fi
