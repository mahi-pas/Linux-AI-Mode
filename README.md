# Linux AI Mode

A modern, transparent GTK application for Linux that provides quick access to multiple AI assistants (Gemini, Claude, and ChatGPT) with a beautiful, blur-enhanced interface.

## Features

### 🚀 **Quick Access**
- Global hotkey (Super+Space) for instant AI assistance
- Transparent background with blur effects
- Always-on-top window option
- Lightweight and fast startup

### 🤖 **Multi-LLM Support**
- **Google Gemini** integration
- **Anthropic Claude** support
- **OpenAI ChatGPT** connectivity
- Easy switching between providers
- Individual API key management

### 💬 **Chat Interface**
- Modern, responsive chat UI
- Real-time message streaming
- Syntax highlighting for code blocks
- Markdown rendering support
- Typing indicators

### 📚 **Chat History**
- Persistent chat storage
- Sidebar with chat history
- Search through past conversations
- Export chat functionality
- Organized by date and provider

### 🎨 **Beautiful UI**
- Transparent background with blur effects
- Dark theme optimized for readability
- Smooth animations and transitions
- Modern GTK4 interface
- Customizable appearance

## Screenshots

```
┌─────────────────────────────────────────────────────┐
│  Linux AI Mode                    [Gemini ▼] [⚙️]  │
├─────────────────────────────────────────────────────┤
│ Chat History    │ 🤖 AI: Hello! How can I help you │
│ ┌─────────────┐ │     today?                       │
│ │ Python Help │ │                                  │
│ │ Web Design  │ │ 👤 You: Help me create a GTK app │
│ │ Bug Fix     │ │                                  │
│ │ + New Chat  │ │ 🤖 AI: I'd be happy to help you │
│ └─────────────┘ │     create a GTK application...  │
│                 │                                  │
│                 │ ┌─────────────────────────────┐  │
│                 │ │ Type your message...        │🚀│
│                 │ └─────────────────────────────┘  │
└─────────────────────────────────────────────────────┘
```

## Installation

### Prerequisites

#### Ubuntu/Debian:
```bash
sudo apt update
sudo apt install cmake gcc g++ libgtk-4-dev libcurl4-openssl-dev libjsoncpp-dev
```

#### Fedora:
```bash
sudo dnf install cmake gcc gcc-c++ gtk4-devel libcurl-devel jsoncpp-devel
```

#### Arch Linux:
```bash
sudo pacman -S cmake gcc gtk4 curl jsoncpp
```

### Build and Install

1. **Clone the repository:**
```bash
git clone https://github.com/yourusername/Linux-AI-Mode.git
cd Linux-AI-Mode
```

2. **Run the installation script:**
```bash
./install.sh
```

3. **Or build manually:**
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
sudo make install
```

## Configuration

### API Keys

The application supports multiple ways to configure API keys:

#### 1. Through the GUI
- Launch the application
- Go to Settings → API Keys
- Enter your keys for each provider

#### 2. Environment Variables
```bash
export GEMINI_API_KEY="your_gemini_key_here"
export CLAUDE_API_KEY="your_claude_key_here"
export OPENAI_API_KEY="your_openai_key_here"
```

#### 3. Configuration File
Create/edit `~/.config/linux-ai-mode/api_keys.json`:
```json
{
  "gemini": "your_gemini_key_here",
  "claude": "your_claude_key_here",
  "chatgpt": "your_openai_key_here"
}
```

### Getting API Keys

#### Google Gemini
1. Visit [Google AI Studio](https://makersuite.google.com/app/apikey)
2. Create a new API key
3. Copy the key to your configuration

#### Anthropic Claude
1. Visit [Anthropic Console](https://console.anthropic.com/)
2. Create an account and get API access
3. Generate an API key from the dashboard

#### OpenAI ChatGPT
1. Visit [OpenAI Platform](https://platform.openai.com/api-keys)
2. Create an account or log in
3. Generate a new API key

### Global Hotkey

The application sets up a global hotkey (Super+Space) automatically. You can change it in GNOME Settings:

1. Open Settings → Keyboard → View and Customize Shortcuts
2. Find "Linux AI Mode" under Custom Shortcuts
3. Click to modify the key combination

## Usage

### Quick Access
- Press **Super+Space** to open the AI assistant
- Type your question and press **Enter** or **Ctrl+Enter**
- Switch between AI providers using the dropdown
- Press **Escape** to minimize the window

### Chat Management
- Click "New Chat" to start a fresh conversation
- Previous chats are saved in the sidebar
- Click any chat in the history to resume it
- Each chat remembers which AI provider was used

### Keyboard Shortcuts
- **Super+Space**: Open/Show window
- **Ctrl+Enter**: Send message
- **Escape**: Hide window
- **Ctrl+N**: New chat
- **Ctrl+,**: Open settings

## Advanced Features

### Transparency Control
Adjust window transparency in the settings:
```bash
gsettings set org.gnome.desktop.wm.preferences compositing-manager true
```

### Custom Themes
Modify the CSS file at `~/.local/share/linux-ai-mode/style.css` to customize the appearance.

### Chat Export
Export conversations to various formats:
- Plain text
- Markdown
- JSON
- HTML

## Development

### Building from Source
```bash
git clone https://github.com/yourusername/Linux-AI-Mode.git
cd Linux-AI-Mode
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

### Contributing
1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests if applicable
5. Submit a pull request

### Project Structure
```
Linux-AI-Mode/
├── src/
│   ├── main.cpp          # Application entry point
│   ├── AIChat.cpp        # Main chat interface
│   ├── LLMProvider.cpp   # AI provider implementations
│   ├── ChatHistory.cpp   # Chat storage and management
│   └── Utils.cpp         # Utility functions
├── include/              # Header files
├── css/                  # Stylesheets
├── CMakeLists.txt        # Build configuration
└── install.sh           # Installation script
```

## Troubleshooting

### Common Issues

#### Application won't start
```bash
# Check if all dependencies are installed
ldd /usr/local/bin/linux-ai-mode

# Run with debug output
linux-ai-mode --debug
```

#### API requests failing
- Verify your API keys are correct
- Check your internet connection
- Ensure you have API quota remaining

#### Transparency not working
```bash
# Enable compositor
gsettings set org.gnome.desktop.wm.preferences compositing-manager true

# Or try with different window manager
export GDK_BACKEND=wayland
```

#### Global hotkey not working
- Check GNOME Settings → Keyboard → Shortcuts
- Ensure the command path is correct: `/usr/local/bin/linux-ai-mode`
- Try setting a different key combination

### Logs and Debugging

Application logs are stored in:
```
~/.local/share/linux-ai-mode/logs/
```

Enable debug mode:
```bash
linux-ai-mode --debug --verbose
```

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- GTK Development Team for the amazing toolkit
- All the AI providers for their APIs
- The Linux community for inspiration and support

## Roadmap

- [ ] Plugin system for custom AI providers
- [ ] Voice input/output support
- [ ] Multi-language support
- [ ] Theme customization UI
- [ ] Integration with system clipboard
- [ ] Floating window mode
- [ ] Chat templates and shortcuts
- [ ] Image generation support
- [ ] File upload capability
- [ ] System tray integration

---

**Made with ❤️ for the Linux community**
