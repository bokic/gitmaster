# 🚀 GitMaster Roadmap

Development plan and upcoming features for GitMaster.

---

## 📈 Version 0.7 - Visual & Log Enhancements
*Enhancing the primary control surfaces and utility access.*

- [x] **Merge Dialog**: Comprehensive interface for merging branches with conflict resolution entry points.
- [ ] **Git Flow Dialog**: Integrated support for Git Flow branching models (feature, release, hotfix).
- [x] **Quick Terminal**: One-click button to launch the system terminal directly in the current project directory.
- [ ] **Settings Portal**: Centralized configuration for UI preferences, Git credentials, and global behaviors.
- [ ] **Enhanced Branch Management**: New commit history view optimized for branch-specific workflows.
- [ ] **Conflict Resolution UI**: Implement a Conflict Resolution UI (likely a new widget or tool) to resolve line-by-line conflicts.
- [ ] **Pull Conflict Resolution**: Add a conflict resolution workflow if the pull results in conflicts.

---

## 🌍 Version 0.8 - Finalizing work & Branding
*Focusing on new features, data visualization and repository history clarity.*

### New git operations
- [ ] **Rebase**: Implement rebasing with a toolbar button.
- [ ] **Cherry-pick**: Extend context menu to cherry-pick the selected commit to other local branches.

### Log History
- [x] **Rich Metadata Rendering**: Draw tags and branch names directly within the commit graph.
- [x] **Advanced Graphing**: Implement smooth, multi-color graph lines for complex branching history.

### File Status
- [ ] **Combo Functionality**: Complete the implementation of all remaining filtering and view options in the status combos.

### Search
- [x] **Global Search Suite**: Full implementation of commit message, author, and file content searching.

---

## 📦 Version 0.9 - Localization, Distribution & Platform Support
*Preparing for a global audience and professional aesthetic.*

### Translation (String Freeze)
- [ ] **I18n Infrastructure**: Setup all strings to be translatable via standard Qt linguist patterns.

### Visual Identity
- [ ] **Iconography Overhaul**: Redesign and polish all system icons and brand assets for a premium feel.

---

## 🏁 Version 1.0 - Production Readiness
*Ensuring seamless installation across all major operating systems.*

### MacOS Support
- [x] **App Bundling**: Create a `.app` bundle for macOS users.

### Linux Distribution
- [ ] **RPM Support**: Native package for Fedora/RHEL/openSUSE.
- [ ] **Snap Package**: Universal Linux distribution via Canonical's Snapcraft.
- [ ] **AppImage**: Portable, single-file Linux executable.
- [ ] **Flatpak**: Modern, sandboxed Linux application distribution.
