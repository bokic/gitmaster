# GitMaster [![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/bokic/gitmaster)

[![License: LGPL v3](https://img.shields.io/badge/License-LGPL%20v3-blue.svg)](https://www.gnu.org/licenses/lgpl-3.0)

GitMaster is a powerful, lightweight Git GUI client built with C++, **Qt6**, and **libgit2**. It provides an intuitive interface for managing your Git repositories, aimed at developers who want a fast and responsive tool for their daily workflow.

![screenshot](https://raw.githubusercontent.com/bokic/gitmaster/master/docs/gitmaster-screenshot.png)

## Features

- **Repository Management**: Initialize new repositories, clone existing ones, and manage your local projects.
- **Branching & Tagging**: Create, switch, and manage branches and tags with ease.
- **Remote Operations**: Seamlessly Fetch, Pull, and Push to remote servers.
- **History Visualization**: Browse commit history with a clear and detailed log table.
- **Integrated Diff Viewer**: View line-by-line changes with integrated diff highlighting.
- **Staging Area**: Easily stage and unstage files for your next commit.
- **Cross-Platform**: Designed for Linux, with support for Windows and macOS.

## Prerequisites

To build GitMaster from source, you will need:

- **C++ Compiler**: Supporting C++20 (e.g., GCC 10+, Clang 10+, or MSVC 2019+).
- **CMake**: Version 3.12 or higher.
- **Qt6**: Following modules are required:
  - Gui
  - Widgets
  - Network
  - Svg
- **libgit2**: Development files for the libgit2 library.

## Build Instructions

### Linux / macOS

1.  **Clone the repository**:
    ```bash
    git clone https://github.com/bokic/gitmaster.git
    cd gitmaster
    ```

2.  **Create a build directory**:
    ```bash
    mkdir build && cd build
    ```

3.  **Run CMake**:
    ```bash
    cmake ..
    ```

4.  **Compile**:
    ```bash
    make -j$(nproc)
    ```

### Arch Linux (AUR)

GitMaster is available in the **Arch User Repository (AUR)**. Arch users can install it using an AUR helper like `yay`:

```bash
yay -S gitmaster
```

### Windows

You can use the provided batch scripts in the `windows` directory:
- `windows/build_deps.bat`: Builds dependencies.
- `windows/build.bat`: Builds the application.
- `windows/build_installer.bat`: Generates an installer using Inno Setup.

Alternatively, you can use CMake directly with a generator like Visual Studio or MinGW.

## Installation

On Linux, you can install GitMaster to your system using:

```bash
sudo make install
```

This will install the binary and the `.desktop` file for system integration.

## License

This project is licensed under the **GNU Lesser General Public License v3.0** - see the [LICENSE](LICENSE) file for details.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request or open an Issue for any bugs or feature requests.
