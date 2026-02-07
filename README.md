# 🎮 SmashEdit

> A lightning-fast, terminal-based text editor built with C and ncurses

[![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)](https://github.com/your-repo/smashedit)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20macOS%20%7C%20Windows-lightgrey.svg)]()

---

## ✨ Features

- 🚀 **Lightning Fast** — Written in C for maximum performance
- 📝 **Modal Editing** — Intuitive keyboard-driven interface
- 🎨 **Beautiful TUI** — Colorful terminal interface with Unicode box-drawing
- ↩️ **Undo/Redo** — Full history with up to 100 levels
- 🔍 **Search & Replace** — Find text with case-sensitive toggle
- 📋 **Clipboard Support** — Cut, copy, and paste with ease
- 🔢 **Line Numbers** — Toggle-able line number display
- 📑 **Multi-Select** — Select multiple occurrences with Ctrl+D
- 🌐 **Unicode Support** — Full UTF-8 and wide character support

---

## 📸 Screenshot

```
┌─────────────────────────────────────────────────────────────┐
│  File   Edit   Search   View   Help                         │
├─────────────────────────────────────────────────────────────┤
│  1 │ #include <stdio.h>                                     │
│  2 │                                                        │
│  3 │ int main() {                                           │
│  4 │     printf("Hello, SmashEdit!\n");                     │
│  5 │     return 0;                                          │
│  6 │ }                                                      │
│                                                             │
├─────────────────────────────────────────────────────────────┤
│  Line: 4  Col: 12  | hello.c [Modified]                     │
└─────────────────────────────────────────────────────────────┘
```

---

## 🛠️ Installation

### Quick Install (Recommended)

#### Linux / macOS

```bash
curl -fsSL https://raw.githubusercontent.com/RealOrko/smash-editor/main/scripts/install.sh | bash
```

#### Windows (PowerShell)

```powershell
irm https://raw.githubusercontent.com/RealOrko/smash-editor/main/scripts/install.ps1 | iex
```

This will:
- Download the latest release for your platform
- Install to `~/.smash`
- Add to your PATH automatically

---

### Build from Source

#### Prerequisites

- 🔧 CMake 3.10+
- 🖥️ GCC or compatible C compiler
- 📚 ncurses library with wide character support

#### Install dependencies

<details>
<summary>🐧 Ubuntu/Debian</summary>

```bash
sudo apt-get update
sudo apt-get install build-essential cmake libncursesw5-dev
```
</details>

<details>
<summary>🐧 Fedora/RHEL</summary>

```bash
sudo dnf install gcc cmake ncurses-devel
```
</details>

<details>
<summary>🐧 Arch Linux</summary>

```bash
sudo pacman -S base-devel cmake ncurses
```
</details>

<details>
<summary>🍎 macOS</summary>

```bash
brew install cmake ncurses
```
</details>

<details>
<summary>🪟 Windows (MSYS2)</summary>

```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-ncurses make
```
</details>

#### Build & Install

```bash
# Clone the repository
git clone https://github.com/RealOrko/smash-editor.git
cd smash-editor

# Build
make build

# Install to ~/.smash/
make install
```

### Run

```bash
# After installation (restart terminal first)
smashedit

# Or run directly
~/.smash/smashedit

# Open a file
smashedit myfile.txt
```

---

## ⌨️ Keyboard Shortcuts

### 📁 File Operations

| Shortcut | Action |
|----------|--------|
| `Ctrl+N` | 📄 New file |
| `Ctrl+O` | 📂 Open file |
| `Ctrl+S` | 💾 Save file |
| `Ctrl+Shift+S` | 💾 Save as |
| `Ctrl+Q` | 🚪 Quit |

### ✏️ Editing

| Shortcut | Action |
|----------|--------|
| `Ctrl+Z` | ↩️ Undo |
| `Ctrl+Y` | ↪️ Redo |
| `Ctrl+X` | ✂️ Cut (or cut line if no selection) |
| `Ctrl+C` | 📋 Copy |
| `Ctrl+V` | 📌 Paste |
| `Ctrl+A` | 🔲 Select all |
| `Ctrl+D` | ➕ Add next occurrence to selection |

### 🔍 Search

| Shortcut | Action |
|----------|--------|
| `Ctrl+F` | 🔎 Find |
| `F3` | ⏭️ Find next |
| `Ctrl+H` | 🔄 Replace |
| `Ctrl+G` | 🔢 Go to line |

### 🧭 Navigation

| Shortcut | Action |
|----------|--------|
| `↑ ↓ ← →` | Move cursor |
| `Ctrl+← / →` | Move by word |
| `Home / End` | Line start/end |
| `Ctrl+Home / End` | Document start/end |
| `Ctrl+T` | 🔝 Go to start (alternative) |
| `Ctrl+B` | 🔚 Go to end (alternative) |
| `PgUp / PgDn` | Scroll page |

### 🎯 Selection

| Shortcut | Action |
|----------|--------|
| `Shift+Arrows` | Select characters |
| `Shift+Home/End` | Select to line start/end |
| `Ctrl+Shift+← / →` | Select word |
| `Esc` | Clear selection |

### 📋 Menu

| Shortcut | Action |
|----------|--------|
| `F10` | Open menu bar |
| `Esc` | Close menu |

---

## 🏗️ Project Structure

```
smashedit/
├── 📄 CMakeLists.txt      # CMake build configuration
├── 📄 Makefile            # Build wrapper
├── 📁 include/
│   └── 📄 smashedit.h     # Main header file
├── 📁 src/
│   ├── 📄 main.c          # Entry point & signal handling
│   ├── 📄 editor.c        # Core editor logic
│   ├── 📄 buffer.c        # Gap buffer implementation
│   ├── 📄 display.c       # UI rendering
│   ├── 📄 input.c         # Keyboard input handling
│   ├── 📄 dialog.c        # Dialog boxes
│   ├── 📄 search.c        # Find/replace functionality
│   ├── 📄 smenu.c         # Menu system
│   ├── 📄 undo.c          # Undo/redo stack
│   ├── 📄 file.c          # File I/O operations
│   └── 📄 clipboard.c     # Clipboard management
└── 📁 bin/                # Build output (generated)
```

---

## ⚙️ Configuration

SmashEdit can be configured at compile-time by modifying `include/smashedit.h`:

```c
#define TAB_WIDTH 2           // Tab width in spaces
#define MAX_UNDO_LEVELS 100   // Maximum undo history
#define MAX_SELECTIONS 1024   // Maximum multi-select ranges
#define MAX_LINE_LENGTH 4096  // Maximum line length
```

### 🎨 Color Scheme

| Element | Colors |
|---------|--------|
| Editor Area | ⬜ White on 🟦 Blue |
| Menu Bar | ⬛ Black on 🩵 Cyan |
| Selected Text | 🟨 Yellow on 🟦 Blue |
| Dialog Boxes | ⬜ White on 🟦 Blue |
| Status Bar | ⬛ Black on 🩵 Cyan |

---

## 🔧 Building from Source

### Using Make (Recommended)

```bash
# Build the project
make build

# Clean build artifacts
make clean

# Install to ~/.smashedit/
make install
```

### Using CMake Directly

```bash
# Configure
cmake -B bin -S . -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build bin

# The binary will be at bin/smashedit
```

---

## 📊 Technical Details

| Metric | Value |
|--------|-------|
| 💻 Language | C (C11 standard) |
| 📐 Lines of Code | ~4,000 |
| 📦 Dependencies | ncurses only |
| 🧩 Modules | 11 components |
| 📏 Buffer Type | Gap Buffer |

### 🏛️ Architecture

- **Gap Buffer** — Efficient text storage for real-time editing
- **Modular Design** — Clean separation of concerns
- **Signal Handling** — Graceful shutdown and terminal resize
- **Memory Safe** — Proper allocation and cleanup

---

## 🗺️ Roadmap

- [ ] 🎨 Syntax highlighting
- [ ] 📑 Multiple buffers/tabs
- [ ] 🔌 Plugin system
- [ ] ⚙️ Configuration file support
- [ ] 🖱️ Mouse support
- [ ] 📜 Macro recording

---

## 🤝 Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

1. 🍴 Fork the repository
2. 🌿 Create your feature branch (`git checkout -b feature/amazing-feature`)
3. 💾 Commit your changes (`git commit -m 'Add amazing feature'`)
4. 📤 Push to the branch (`git push origin feature/amazing-feature`)
5. 🔀 Open a Pull Request

---

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

## 🙏 Acknowledgments

- 📚 [ncurses](https://invisible-island.net/ncurses/) — The terminal UI library that makes this possible
- 💡 Inspired by classic terminal editors like nano, vim, and micro

---

<p align="center">
  Made with ❤️ and ☕
</p>

<p align="center">
  <b>⭐ Star this repo if you find it useful!</b>
</p>
