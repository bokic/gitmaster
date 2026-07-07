# GitMaster - Proposed Missing Features (libgit2 Integration)

A comprehensive reanalysis of `libgit2` features for integration into GitMaster, based on a full audit of the current codebase (`QGit` wrapper, `QGitRepository`, and UI components).

---

## 📊 Summary of Implementation Status

| Feature Area | Current Status | Relevant libgit2 Headers | Priority |
| :--- | :--- | :--- | :--- |
| **Code Blaming (Line Annotations)** | Unimplemented | `git2/blame.h` | Medium |
| **Worktree Support** | Unimplemented | `git2/worktree.h` | Medium |
| **Reflog Navigation** | Unimplemented | `git2/reflog.h` | Medium |
| **Remote Repository CRUD & Refspecs** | Partial (list/fetch/push) | `git2/remote.h` | Medium |
| **Ahead/Behind Graph Analysis** | Partial (Ancestor check implemented) | `git2/graph.h` | Medium |

---

## 📈 Detailed Feature Breakdown

### 1. Code Blaming (`git2/blame.h`)
* **libgit2 APIs**: `git_blame_file`, `git_blame_buffer`, `git_blame_get_hunk_byindex`
* **Description**: **Blame Annotations**. Display author name, commit date, and short hash next to each line of code in the file view (accessible from commit file lists or diff views).

### 2. Worktree Support (`git2/worktree.h`)
* **libgit2 APIs**: `git_worktree_add`, `git_worktree_list`, `git_worktree_prune`, `git_worktree_validate`
* **Description**: **Worktrees Manager**. Allow developers to checkout branches into separate work directories directly from the GUI, letting them work on multiple branches concurrently without stashing or switching.

### 3. Reflog Navigation (`git2/reflog.h`)
* **libgit2 APIs**: `git_reflog_read`, `git_reflog_entry_byindex`
* **Description**: **Reflog Console**. A history list of reference logs (`HEAD` pointer updates), showing checkout histories and actions. This serves as a safety net to find and recover lost commits or aborted rebases.

### 4. Remote Repository CRUD & Refspecs (`git2/remote.h`)
* **libgit2 APIs**: `git_remote_create`, `git_remote_delete`, `git_remote_rename`, `git_remote_set_url`
* **Description**: **Remote Configuration Manager**. Provide UI to add new remote repositories, delete remotes, rename remotes, and edit fetch/push refspecs.

### 5. Ahead/Behind Graph Analysis (`git2/graph.h`)
* **libgit2 APIs**: `git_graph_ahead_behind`, `git_graph_descendant_of`
* **Status**: **Partial**. Ancestry checks (`git_graph_descendant_of`) have been wrapped in `QGit::isAncestor` and hooked into the log context menu to prevent invalid/redundant rebases. Branch sync counters (ahead/behind indicators in repository trees) remain unimplemented.
