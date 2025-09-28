# Bearbeiten

A lightweight text editor built with C++ and Qt 6.

Influenced by all the great lightweight editors out there, Bearbeiten aims to be simple, fast, and focused on the essentials of text editing.

## Installation

### Prerequisites

- Qt 6.2+ development libraries
- CMake 3.16+
- C++17 compatible compiler

### Linux (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install qt6-base-dev qt6-tools-dev cmake build-essential

git clone <repository-url>
cd bearbeiten
mkdir build && cd build
cmake ..
make
./bearbeiten
```

### Linux (Fedora/CentOS)

```bash
sudo dnf install qt6-qtbase-devel qt6-qttools-devel cmake gcc-c++

git clone <repository-url>
cd bearbeiten
mkdir build && cd build
cmake ..
make
./bearbeiten
```

### macOS

```bash
brew install qt@6 cmake

git clone <repository-url>
cd bearbeiten
mkdir build && cd build
cmake -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/qt@6 ..
make
./bearbeiten
```

### Windows

1. Install [Qt 6](https://www.qt.io/download)
2. Install [CMake](https://cmake.org/download/)
3. Open Qt Creator and import the CMakeLists.txt file
4. Build and run

## Feature Requests

Got an idea for a feature? Please [open an issue](https://github.com/username/bearbeiten/issues) and let me know what you'd like to see!

I'm particularly interested in:
- New syntax highlighting languages
- UI/UX improvements
- Performance optimizations
- Accessibility features
- Platform-specific integrations

## License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details.

---

**Bearbeiten**