# GitMaster - AI Assistant Context 

## Project Overview
GitMaster is a lightweight, fast, cross-platform GUI client for Git, built using C++ (C++20), Qt6, and libgit2. It offers repository management, an integrated diff viewer, visualization for commit histories, and remote operations fetching/pushing/pulling. 

## Technology Stack
- **Language**: C++20
- **GUI Framework**: Qt6 (Modules: Core, Gui, Widgets, Network, Svg)
- **Git Backend**: `libgit2` abstraction wrappers
- **Build System**: CMake (via `CMakeLists.txt` / Makefile or Ninja generator)

## Architecture Overview
- **`qgit/` (`qgit.h` / `qgit.cpp`)**: Contains the core encapsulation for `libgit2` commands, operating as a backend wrapper.
- **`QGitMasterMainWindow`**: The main entry point for the window logic (`qgitmastermainwindow.cpp`, `.h`, `.ui`), containing toolbars and tabs.
- **`QGitRepository`**: Maps to an active repository widget tab, holding the state of file indices, branches, staged/unstaged changes, and orchestrating backend `libgit2` calls.
- **Dialogs**: Features are broken down into descriptive UI files and controllers, e.g., `QGitPushDialog`, `QGitBranchDialog`, `QGitMergeDialog`, `QGitCloneRepositoryDialog`, etc.

## Key Paradigms
1. **Signals and Slots**: Standard Qt signaling is heavily used. Backend events from the `QGit` class are broadcasted to the frontend via signals (e.g., `repositoryFetchReply`).
2. **Threading**: The `QGit` backend object is instantiated and moved to its own `QThread` inside `QGitRepository`. This is critical, as it ensures the GUI remains responsive and doesn't freeze during heavy network bindings or large diff calculations.

## Build Instructions 
From the repository root context, you can run the helper script `build.sh`:
```bash
./build.sh
```

Alternatively, you can build manually:
```bash
mkdir build && cd build
cmake ..
ninja # or make -j$(nproc)
```

## Relevant Development Notes
1. **Adding Backend Features**: Implementing a new backend action typically involves:
   - Defining a new method spanning `QGit` functionality around a `libgit2` call.
   - Emitting a custom signal from `QGit` after the operation completes.
   - Hooking the frontend (`QGitRepository` slots) to process the `QGit` signal.
2. **Adding UI Toolbars / Actions**: UI tools are typically mounted via `QAction` in the UI designer (`qgitmastermainwindow.ui`) and handled automatically leveraging standard Qt naming mappings (`void on_action*Name*_triggered()`).
3. **Refreshing State Models**: Ensure to call `refreshData()` across the `QGitRepository` reference when operations finalize to invalidate and update list widgets properly (e.g., after staging, un-staging, fetching, commits).
