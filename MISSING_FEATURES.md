# GitMaster - Proposed Missing Features (libgit2 Integration)

A comprehensive reanalysis of `libgit2` features for integration into GitMaster, based on a full audit of the current codebase (`QGit` wrapper, `QGitRepository`, and UI components).

---

## 📊 Summary of Implementation Status

| Feature Area | Current Status | Relevant libgit2 Headers | Priority |
| :--- | :--- | :--- | :--- |
| **Git Settings & Config Editor** | Minimal (signature only) | `git2/config.h` | High |
| **3-Way Conflict Resolution API** | Partial (merge check only) | `git2/index.h`, `git2/merge.h` | High |
| **Submodule Management** | Unimplemented | `git2/submodule.h` | Medium |
| **Code Blaming (Line Annotations)** | Unimplemented | `git2/blame.h` | Medium |
| **Worktree Support** | Unimplemented | `git2/worktree.h` | Medium |
| **Apply Patches/Diffs** | Unimplemented | `git2/apply.h` | Medium |
| **Reflog Navigation** | Unimplemented | `git2/reflog.h` | Medium |
| **Git Tag Creation (Annotated/Lightweight)** | Partial (list/rename/delete) | `git2/tag.h` | Medium |
| **Remote Repository CRUD & Refspecs** | Partial (list/fetch/push) | `git2/remote.h` | Medium |
| **Ahead/Behind Graph Analysis** | Partial (Ancestor check implemented) | `git2/graph.h` | Medium |
| **Commit Amending & GPG/SSH Signing** | Partial (basic commit only) | `git2/commit.h`, `git2/sys/commit.h` | Medium |
| **Stash Advanced Options & Patch Inspection** | Partial (basic save/pop/apply) | `git2/stash.h` | Low |
| **Git Describe** | Unimplemented | `git2/describe.h` | Low |
| **Ignore Rules Integration** | Unimplemented | `git2/ignore.h` | Low |
| **Git Notes Management** | Unimplemented | `git2/note.h` | Low |
| **Mailmap Author Mapping** | Unimplemented | `git2/mailmap.h` | Low |
| **Revision Parsing Engine** | Partial (direct OID/ref only) | `git2/revparse.h` | Low |
| **Clean Workspace (`git clean`)** | Unimplemented | `git2/status.h`, filesystem | Low |

---

## 📈 Detailed Feature Breakdown

### 1. Git Settings Portal (`git2/config.h`)
* **libgit2 APIs**: `git_config_open_default`, `git_config_set_string`, `git_config_get_string`, `git_config_delete_entry`
* **Description**: **Git Configuration Editor**. Add a settings dialog to read/write global and local config options (e.g., user identity `user.name` and `user.email`, line endings configuration, default remote, proxy settings, SSH/GPG credentials).

### 2. Submodule Management (`git2/submodule.h`)
* **libgit2 APIs**: `git_submodule_add_setup`, `git_submodule_init`, `git_submodule_update`, `git_submodule_status`, `git_submodule_sync`
* **Description**: **Submodules Explorer**. Integrate submodules into the branch tree view, showing submodule status, enabling init/update/sync actions, and checking out specific submodule pins.

### 3. Code Blaming (`git2/blame.h`)
* **libgit2 APIs**: `git_blame_file`, `git_blame_buffer`, `git_blame_get_hunk_byindex`
* **Description**: **Blame Annotations**. Display author name, commit date, and short hash next to each line of code in the file view (accessible from commit file lists or diff views).

### 4. Worktree Support (`git2/worktree.h`)
* **libgit2 APIs**: `git_worktree_add`, `git_worktree_list`, `git_worktree_prune`, `git_worktree_validate`
* **Description**: **Worktrees Manager**. Allow developers to checkout branches into separate work directories directly from the GUI, letting them work on multiple branches concurrently without stashing or switching.

### 5. Apply Patches (`git2/apply.h`)
* **libgit2 APIs**: `git_apply`
* **Description**: **Apply Diff/Patch File**. Let users apply a standard unified diff or `.patch` file directly to the workspace/index.

### 6. Reflog Navigation (`git2/reflog.h`)
* **libgit2 APIs**: `git_reflog_read`, `git_reflog_entry_byindex`
* **Description**: **Reflog Console**. A history list of reference logs (`HEAD` pointer updates), showing checkout histories and actions. This serves as a safety net to find and recover lost commits or aborted rebases.

### 7. Tag Creation (Annotated & Lightweight) (`git2/tag.h`)
* **libgit2 APIs**: `git_tag_create`, `git_tag_create_lightweight`, `git_tag_annotation_create`
* **Description**: **Tag Creation UI**. While tag listing, deletion, and renaming exist, GitMaster currently lacks a GUI to create new lightweight or annotated tags attached to a specific commit.

### 8. Remote Repository CRUD & Refspecs (`git2/remote.h`)
* **libgit2 APIs**: `git_remote_create`, `git_remote_delete`, `git_remote_rename`, `git_remote_set_url`
* **Description**: **Remote Configuration Manager**. Provide UI to add new remote repositories, delete remotes, rename remotes, and edit fetch/push refspecs.

### 9. Ahead/Behind Graph Analysis (`git2/graph.h`)
* **libgit2 APIs**: `git_graph_ahead_behind`, `git_graph_descendant_of`
* **Status**: **Partial**. Ancestry checks (`git_graph_descendant_of`) have been wrapped in `QGit::isAncestor` and hooked into the log context menu to prevent invalid/redundant rebases. Branch sync counters (ahead/behind indicators in repository trees) remain unimplemented.

### 10. 3-Way Conflict Resolution & Staged Conflict Removal (`git2/index.h` & `git2/merge.h`)
* **libgit2 APIs**: `git_index_conflict_get`, `git_index_conflict_remove`, `git_index_add_from_buffer`
* **Description**: **Conflict Resolution UI**. Extract ancestor, ours, and theirs stages from libgit2 index during merge/rebase conflicts to render a 3-way side-by-side conflict solver.

### 11. Commit Amending & GPG/SSH Signature Signing (`git2/commit.h` & `git2/sys/commit.h`)
* **libgit2 APIs**: `git_commit_amend`, `git_commit_create_with_signature`
* **Description**: **Amend & Signed Commits**. Allow users to amend the last commit message or staged files, and optionally sign commits using GPG or SSH keys.

### 12. Stash Advanced Options & Patch Inspection (`git2/stash.h`)
* **libgit2 APIs**: `git_stash_save2` (`GIT_STASH_INCLUDE_UNTRACKED`, `GIT_STASH_KEEP_INDEX`), `git_stash_apply`
* **Description**: **Advanced Stash Options**. Allow users to include untracked/ignored files in stashes, view diffs of individual stashes before applying, or apply specific stash elements.

### 13. Git Describe (`git2/describe.h`)
* **libgit2 APIs**: `git_describe_commit`, `git_describe_format`
* **Description**: **Describe Hash**. Display tags relative to a commit (e.g., `v1.2.0-4-g8c41174`) inside the commit description panel.

### 14. Ignore Rules Integration (`git2/ignore.h`)
* **libgit2 APIs**: `git_ignore_add_rule`, `git_ignore_path_is_ignored`
* **Description**: **Context-Menu .gitignore Addition**. Allow users to right-click an untracked file in the changed files list and choose to ignore the file pattern (appending rules directly to `.gitignore`).

### 15. Git Notes Management (`git2/note.h`)
* **libgit2 APIs**: `git_note_create`, `git_note_read`, `git_note_remove`
* **Description**: **Commit Notes**. View and attach metadata notes to commits without modifying commit hashes.

### 16. Revision Parsing Engine (`git2/revparse.h`)
* **libgit2 APIs**: `git_revparse_single`, `git_revparse_ext`
* **Description**: **Git Revision Query**. Support parsing expressions like `HEAD~3`, `main@{1}`, or `HEAD^2` in search inputs or checkout dialogs.

### 17. Clean Workspace (`git clean`) (`git2/status.h`)
* **libgit2 APIs**: Status filtering + `git_status_byindex`
* **Description**: **Clean Untracked Files**. Provide a "Clean Working Directory" action to batch-remove untracked files or ignored build artifacts with prompt confirmations.

### 18. Mailmap Support (`git2/mailmap.h`)
* **libgit2 APIs**: `git_mailmap_new`, `git_mailmap_resolve`
* **Description**: **Author Identity Normalization**. Resolve author and committer names/emails according to repository `.mailmap` rules when generating commit log tables.
