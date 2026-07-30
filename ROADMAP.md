# 🚀 GitMaster Roadmap

Development plan and upcoming features for GitMaster.

---

## 🌍 Version 0.8 - Finalizing work & Branding
*Desloping/bugfixing codebase.*

---

## 📦 Version 0.9 - Feature Enhancements & Advanced Git Tools

### Interactive Staging & Diffing
- [ ] **Stash Inspection**: Allow double-clicking stashes in the stash dialog to view file diffs directly in the main diff widget.

### Advanced Commit & Rebase Tools
- [ ] **Commit Reword & Amend UI**: Enable instant commit message editing for HEAD and UI options for amending.
- [ ] **Interactive Rebase Dialog**: Support squashing, rewording, dropping, and reordering commits from the commit log view.
- [ ] **Cherry-Pick / Rebase Controls**: Add UI controls to continue or abort active merge/rebase/cherry-pick operations.

### Submodules & Repository Management
- [ ] **Add Submodule Dialog**: Implement dialog for cloning and adding new submodules (`git submodule add`).
- [ ] **Recursive Submodule Fetch/Pull**: Option to automatically fetch and update submodules recursively.
- [ ] **Patch Export & Import**: Export commits as `.patch` files (`git format-patch`) and apply external patches via file selection.
- [ ] **Archive Export**: Export repository tree at specific tags or commits as `.zip` or `.tar.gz` (`git archive`).

---

## 🎨 Version 1.0 - Localization, Distribution & Production Readiness
*Preparing for a global audience and professional aesthetic.*

### Translation (String Freeze)
- [ ] **I18n Infrastructure**: Setup all strings to be translatable via standard Qt linguist patterns.

### Visual Identity & Distribution
- [ ] **Iconography Overhaul**: Redesign and polish all system icons and brand assets for a premium feel.
- [ ] **Cross-Platform Packaging**: AppImage, Flatpak, and macOS bundle distribution pipelines.
