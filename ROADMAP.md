# 🚀 GitMaster Roadmap

Development plan and upcoming features for GitMaster.

---

## 🌍 Version 0.7 - Finalizing features
*Focusing on new features, data visualization and repository history clarity.*

### New git operations
- [x] **Rebase**: Implement rebasing with a toolbar button.
- [x] **Cherry-pick**: Extend context menu to cherry-pick the selected commit onto the current branch.
- [x] **Submodules**: Fully integrated support for Git Submodules (listing, statuses, init, update, sync, and tabbed sub-repository opening).
- [x] **Worktree Support**: Full support for linked working trees (listing, creating, locking, unlocking, pruning, and opening in separate tabs).
- [x] **Tag Creation**: Create lightweight and annotated tags from the log history context menu, with optional message and force-overwrite support.
- [x] **Ignore Rules**: Right-click any untracked file to append a customisable pattern to `.gitignore` directly from the UI.
- [x] **Git Describe**: The commit info panel now shows a **Describe:** line with the nearest reachable tag (e.g., `v1.2.0-4-g8c41174`) for any selected commit.

### Log History
- [x] **Rich Metadata Rendering**: Draw tags and branch names directly within the commit graph.
- [x] **Advanced Graphing**: Implement smooth, multi-color graph lines for complex branching history.

### File Status
- [x] **Combo Functionality**: Complete the implementation of all remaining filtering and view options in the status combos.
- [x] **View Layout Options**: Wire up and implement the Flat list and Tree view options in the View Options combo.
- [ ] **Staging Mode Options**: Wire up and implement the No staging and Split view staging options in the View Options combo.

### Search
- [x] **Global Search Suite**: Full implementation of commit message, author, and file content searching.

---

## 🌍 Version 0.8 - Finalizing work & Branding
*Desloping/bugfixing codebase.

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
- [x] **App Bundling**: Create a `.dmg` bundle for macOS users.

### Linux Distribution
- [ ] **RPM Support**: Native package for Fedora/RHEL/openSUSE.
- [ ] **Snap Package**: Universal Linux distribution via Canonical's Snapcraft.
- [ ] **AppImage**: Portable, single-file Linux executable.
- [ ] **Flatpak**: Modern, sandboxed Linux application distribution.
