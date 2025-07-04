#!/bin/bash

# Install script for Linux AI Mode

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}Linux AI Mode Installation Script${NC}"
echo "=================================="

# Check if running as root
if [[ $EUID -eq 0 ]]; then
   echo -e "${RED}Error: This script should not be run as root${NC}" 
   exit 1
fi

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Check dependencies
echo -e "${YELLOW}Checking dependencies...${NC}"

MISSING_DEPS=()

if ! command_exists cmake; then
    MISSING_DEPS+=("cmake")
fi

if ! command_exists gcc; then
    MISSING_DEPS+=("gcc")
fi

if ! command_exists g++; then
    MISSING_DEPS+=("g++")
fi

if ! pkg-config --exists gtk4; then
    MISSING_DEPS+=("libgtk-4-dev")
fi

if ! pkg-config --exists libcurl; then
    MISSING_DEPS+=("libcurl4-openssl-dev")
fi

if ! pkg-config --exists jsoncpp; then
    MISSING_DEPS+=("libjsoncpp-dev")
fi

if [ ${#MISSING_DEPS[@]} -ne 0 ]; then
    echo -e "${RED}Missing dependencies:${NC}"
    printf '%s\n' "${MISSING_DEPS[@]}"
    echo ""
    echo -e "${YELLOW}Please install them using:${NC}"
    echo "sudo apt update"
    echo "sudo apt install ${MISSING_DEPS[*]}"
    exit 1
fi

echo -e "${GREEN}All dependencies found!${NC}"

# Create build directory
echo -e "${YELLOW}Creating build directory...${NC}"
mkdir -p build
cd build

# Configure with CMake
echo -e "${YELLOW}Configuring with CMake...${NC}"
cmake ..

# Build the project
echo -e "${YELLOW}Building the project...${NC}"
make -j$(nproc)

# Install the application
echo -e "${YELLOW}Installing application...${NC}"
sudo make install

# Create necessary directories
echo -e "${YELLOW}Creating user directories...${NC}"
mkdir -p ~/.config/linux-ai-mode
mkdir -p ~/.local/share/linux-ai-mode

# Set up global hotkey (optional)
echo -e "${YELLOW}Setting up global hotkey...${NC}"
SHORTCUT_CMD="gsettings set org.gnome.settings-daemon.plugins.media-keys custom-keybindings \"['/org/gnome/settings-daemon/plugins/media-keys/custom-keybindings/custom0/']\""
SHORTCUT_NAME="gsettings set org.gnome.settings-daemon.plugins.media-keys.custom-keybinding:/org/gnome/settings-daemon/plugins/media-keys/custom-keybindings/custom0/ name 'Linux AI Mode'"
SHORTCUT_COMMAND="gsettings set org.gnome.settings-daemon.plugins.media-keys.custom-keybinding:/org/gnome/settings-daemon/plugins/media-keys/custom-keybindings/custom0/ command 'linux-ai-mode'"
SHORTCUT_BINDING="gsettings set org.gnome.settings-daemon.plugins.media-keys.custom-keybinding:/org/gnome/settings-daemon/plugins/media-keys/custom-keybindings/custom0/ binding '<Super>space'"

if command_exists gsettings; then
    eval $SHORTCUT_CMD
    eval $SHORTCUT_NAME
    eval $SHORTCUT_COMMAND
    eval $SHORTCUT_BINDING
    echo -e "${GREEN}Global hotkey set to Super+Space${NC}"
else
    echo -e "${YELLOW}gsettings not found. You'll need to set up the hotkey manually.${NC}"
fi

echo ""
echo -e "${GREEN}Installation completed successfully!${NC}"
echo ""
echo -e "${YELLOW}Setup Instructions:${NC}"
echo "1. Launch the application from the applications menu or run 'linux-ai-mode'"
echo "2. Configure your API keys for Gemini, Claude, and/or ChatGPT"
echo "3. Press Super+Space to quickly access the AI assistant"
echo ""
echo -e "${YELLOW}API Key Configuration:${NC}"
echo "The application will create a configuration file at:"
echo "~/.config/linux-ai-mode/api_keys.json"
echo ""
echo "You can also set environment variables:"
echo "export GEMINI_API_KEY=\"your_key_here\""
echo "export CLAUDE_API_KEY=\"your_key_here\""
echo "export OPENAI_API_KEY=\"your_key_here\""
