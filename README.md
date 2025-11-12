# Eddy - Text Editor

A lightweight text editor built with C++ and Qt 6 for developers and writers.

Designed with functional minimalism and precision, focusing on the essentials of text editing with advanced features for coding.

## Features

- **Multi-tab editing** - Work on multiple files simultaneously with split-view support
- **Syntax highlighting** - JSON-based extensible language support
- **Project panel** - File browser for easy project navigation
- **Find and replace** - Advanced search with regex support
- **Auto-save** - Configurable automatic file saving
- **Responsive design** - Optimized for MNT Pocket Reform and desktop displays
- **Modern UI** - Clean design inspired by NumWorks and elementary OS
- **Internationalization** - Support for 7 languages
- **Keyboard shortcuts** - Efficient workflow without mouse

## Installation

### Prerequisites

- Qt 6.2+ development libraries
- CMake 3.16+
- C++17 compatible compiler

### Linux (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install qt6-base-dev qt6-tools-dev cmake build-essential

cd build
cmake ..
make -j4
./eddy
```

### Linux (Fedora/CentOS)

```bash
sudo dnf install qt6-qtbase-devel qt6-qttools-devel cmake gcc-c++

cd build
cmake ..
make -j4
./eddy
```

### macOS

```bash
brew install qt@6 cmake

cd build
cmake -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/qt@6 ..
make -j4
./eddy
```

### Windows

1. Install [Qt 6](https://www.qt.io/download)
2. Install [CMake](https://cmake.org/download/)
3. Open Qt Creator and import the CMakeLists.txt file
4. Build and run

## Usage

### Keyboard Shortcuts

- **Ctrl+N** - New file
- **Ctrl+O** - Open file
- **Ctrl+S** - Save file
- **Ctrl+Shift+S** - Save as
- **Ctrl+Z** - Undo
- **Ctrl+Y** - Redo
- **Ctrl+X** - Cut
- **Ctrl+C** - Copy
- **Ctrl+V** - Paste
- **Ctrl+F** - Find
- **Ctrl+H** - Replace
- **Ctrl+\\** - Toggle split view
- **Ctrl+Shift+E** - Toggle project panel

### View Modes

- **Single view** - Standard single-pane editing
- **Split view** - Edit two files side-by-side (vertical or horizontal)
- **Project panel** - Browse and open files from directory tree

## License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details.

---

