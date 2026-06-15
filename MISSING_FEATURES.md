# GitMaster - Proposed Missing Features (libgit2 Integration)

A comprehensive list of git features that can be integrated into GitMaster using the available `libgit2` C API headers.

---

## 📈 Proposed Features

### 1. Rebase Operations (`git2/rebase.h` & `git2/annotated_commit.h`)
*   **libgit2 APIs**: `git_rebase_init`, `git_rebase_next`, `git_rebase_commit`, `git_rebase_abort`, `git_rebase_finish`
*   **Description**: Implement a **Rebase Workflow Dialog**. Currently, only merge and pull are supported. We can add a rebase interface to step through commits, resolve conflicts line-by-line, and commit or abort the rebase step-by-step.

### 2. Cherry-Picking (`git2/cherrypick.h`)
*   **libgit2 APIs**: `git_cherrypick`, `git_cherrypick_commit`
*   **Description**: **Cherry-pick from History**. Extend the log history context menu to allow cherry-picking a selected commit directly onto the current checked-out branch.

### 3. Reverting Commits (`git2/revert.h`)
*   **libgit2 APIs**: `git_revert`, `git_revert_commit`
*   **Description**: **Revert Selected Commit**. Add a "Revert" action in the commit log view that automatically creates a revert commit (reversing the changes of the selected commit) and handles merge conflicts.

### 4. Pointer Resetting (`git2/reset.h`)
*   **libgit2 APIs**: `git_reset`, `git_reset_from_annotated`
*   **Description**: **Reset Branch Pointer (Soft/Mixed/Hard)**. Allow users to right-click any commit in the log history and choose:
    *   `Soft Reset` (`GIT_RESET_SOFT`): Moves branch pointer (retains staged changes).
    *   `Mixed Reset` (`GIT_RESET_MIXED`): Moves branch pointer (unstages changes, retains workdir).
    *   `Hard Reset` (`GIT_RESET_HARD`): Moves branch pointer (discards all changes in index and workdir).

### 5. Git Settings Portal (`git2/config.h`)
*   **libgit2 APIs**: `git_config_open_default`, `git_config_set_string`, `git_config_get_string`
*   **Description**: **Git Configuration Editor**. Add a settings dialog to read/write global and local config options (e.g., user identity `user.name` and `user.email`, line endings configuration, default remote, credentials).

### 6. Submodule Management (`git2/submodule.h`)
*   **libgit2 APIs**: `git_submodule_add_setup`, `git_submodule_init`, `git_submodule_update`, `git_submodule_status`
*   **Description**: **Submodules Explorer**. Integrate submodules into the branch tree view, showing submodule status, enabling init/update/sync actions, and checking out specific submodule pins.

### 7. Code Blaming (`git2/blame.h`)
*   **libgit2 APIs**: `git_blame_file`, `git_blame_get_line_index`
*   **Description**: **Blame Annotations**. Display the author name, commit date, and short hash next to each line of code in the file view (accessible from commit file lists or diff views).

### 8. Worktree Support (`git2/worktree.h`)
*   **libgit2 APIs**: `git_worktree_add`, `git_worktree_list`, `git_worktree_prune`
*   **Description**: **Worktrees Manager**. Allow developers to checkout branches into separate work directories directly from the GUI, letting them work on multiple branches concurrently without stashing or switching.

### 9. Apply Patches (`git2/apply.h`)
*   **libgit2 APIs**: `git_apply`
*   **Description**: **Apply Diff/Patch File**. Let users apply a standard unified diff or `.patch` file directly to the workspace/index.

### 10. Reflog Navigation (`git2/reflog.h`)
*   **libgit2 APIs**: `git_reflog_read`, `git_reflog_entry_byindex`
*   **Description**: **Reflog Console**. A history list of reference logs (`HEAD` pointer updates), showing checkout histories and actions. This serves as a safety net to find and recover lost commits or aborted rebases.

### 11. Git Describe (`git2/describe.h`)
*   **libgit2 APIs**: `git_describe_commit`, `git_describe_format`
*   **Description**: **Describe Hash**. Display tags relative to a commit (e.g., `v1.2.0-4-g8c41174`) inside the commit description panel.

### 12. Ignore Rules Integration (`git2/ignore.h`)
*   **libgit2 APIs**: `git_ignore_add_rule`, `git_ignore_path_is_ignored`
*   **Description**: **Context-Menu .gitignore Addition**. Allow users to right-click an untracked file in the changed files list and choose to ignore the file pattern (appending rules directly to `.gitignore`).
