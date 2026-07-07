# GitMaster - Proposed Missing Features (libgit2 Integration)

A comprehensive reanalysis of `libgit2` features for integration into GitMaster, based on a full audit of the current codebase (`QGit` wrapper, `QGitRepository`, and UI components).

---

## 📊 Summary of Implementation Status

| Feature Area | Current Status | Relevant libgit2 Headers | Priority |
| :--- | :--- | :--- | :--- |
| **Code Blaming (Line Annotations)** | Unimplemented | `git2/blame.h` | Medium |
| **Worktree Support** | Unimplemented | `git2/worktree.h` | Medium |

---

## 📈 Detailed Feature Breakdown

### 1. Code Blaming (`git2/blame.h`)
* **libgit2 APIs**: `git_blame_file`, `git_blame_buffer`, `git_blame_get_hunk_byindex`
* **Description**: **Blame Annotations**. Display author name, commit date, and short hash next to each line of code in the file view (accessible from commit file lists or diff views).

### 2. Worktree Support (`git2/worktree.h`)
* **libgit2 APIs**: `git_worktree_add`, `git_worktree_list`, `git_worktree_prune`, `git_worktree_validate`
* **Description**: **Worktrees Manager**. Allow developers to checkout branches into separate work directories directly from the GUI, letting them work on multiple branches concurrently without stashing or switching.
