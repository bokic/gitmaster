#include <qgit.h>
#include <qgitmastermainwindow.h>
#include <qgitdifffile.h>
#include <qgitdiffhunk.h>
#include <qgitdiffline.h>
#include <qgiterror.h>

#include <QThread>
#include <QString>
#include <QVector>
#include <QDir>
#include <QFile>
#include <functional>


struct GitRepository {
    GitRepository() = default;
    GitRepository(const GitRepository&) = delete;
    GitRepository& operator=(const GitRepository&) = delete;
    GitRepository(GitRepository&&) = delete;
    GitRepository& operator=(GitRepository&&) = delete;
    operator git_repository*() { return value; }
    operator git_repository**() { return &value; }
    ~GitRepository() { if (value) { git_repository_free(value); value = nullptr; }}
    git_repository *value = nullptr;
};

struct GitReference {
    GitReference() = default;
    GitReference(const GitReference&) = delete;
    GitReference& operator=(const GitReference&) = delete;
    GitReference(GitReference&&) = delete;
    GitReference& operator=(GitReference&&) = delete;
    operator git_reference*() { return value; }
    operator git_reference**() { return &value; }
    ~GitReference() { if (value) { git_reference_free(value); value = nullptr; }}
    git_reference *value = nullptr;
};

struct GitSignature {
    GitSignature() = default;
    GitSignature(const GitSignature&) = delete;
    GitSignature& operator=(const GitSignature&) = delete;
    GitSignature(GitSignature&&) = delete;
    GitSignature& operator=(GitSignature&&) = delete;
    operator git_signature*() { return value; }
    operator git_signature**() { return &value; }
    ~GitSignature() { if (value) { git_signature_free(value); value = nullptr; }}
    git_signature *value = nullptr;
};

struct GitStatusList {
    GitStatusList() = default;
    GitStatusList(const GitStatusList&) = delete;
    GitStatusList& operator=(const GitStatusList&) = delete;
    GitStatusList(GitStatusList&&) = delete;
    GitStatusList& operator=(GitStatusList&&) = delete;
    operator git_status_list*() { return value; }
    operator git_status_list**() { return &value; }
    ~GitStatusList() { if (value) { git_status_list_free(value); value = nullptr; }}
    git_status_list *value = nullptr;
};

struct GitBranchIterator {
    GitBranchIterator() = default;
    GitBranchIterator(const GitBranchIterator&) = delete;
    GitBranchIterator& operator=(const GitBranchIterator&) = delete;
    GitBranchIterator(GitBranchIterator&&) = delete;
    GitBranchIterator& operator=(GitBranchIterator&&) = delete;
    operator git_branch_iterator*() { return value; }
    operator git_branch_iterator**() { return &value; }
    ~GitBranchIterator() { if (value) { git_branch_iterator_free(value); value = nullptr; }}
    git_branch_iterator *value = nullptr;
};

struct GitTree {
    GitTree() = default;
    GitTree(const GitTree&) = delete;
    GitTree& operator=(const GitTree&) = delete;
    GitTree(GitTree&&) = delete;
    GitTree& operator=(GitTree&&) = delete;
    operator git_tree*() { return value; }
    operator git_tree**() { return &value; }
    ~GitTree() { if (value) { git_tree_free(value); value = nullptr; }}
    git_tree *value = nullptr;
};

struct GitCommit {
    GitCommit() = default;
    GitCommit(const GitCommit&) = delete;
    GitCommit& operator=(const GitCommit&) = delete;
    GitCommit(GitCommit&&) = delete;
    GitCommit& operator=(GitCommit&&) = delete;
    operator git_commit*() { return value; }
    operator git_commit**() { return &value; }
    ~GitCommit() { if (value) { git_commit_free(value); value = nullptr; }}
    git_commit *value = nullptr;
};

struct GitDiff {
    GitDiff() = default;
    GitDiff(const GitDiff&) = delete;
    GitDiff& operator=(const GitDiff&) = delete;
    GitDiff(GitDiff&&) = delete;
    GitDiff& operator=(GitDiff&&) = delete;
    operator git_diff*() { return value; }
    operator git_diff**() { return &value; }
    ~GitDiff() { if (value) { git_diff_free(value); value = nullptr; }}
    git_diff *value = nullptr;
};

struct GitRemote {
    GitRemote() = default;
    GitRemote(const GitRemote&) = delete;
    GitRemote& operator=(const GitRemote&) = delete;
    GitRemote(GitRemote&&) = delete;
    GitRemote& operator=(GitRemote&&) = delete;
    operator git_remote*() { return value; }
    operator git_remote**() { return &value; }
    ~GitRemote() { if (value) { git_remote_free(value); value = nullptr; }}
    git_remote *value = nullptr;
};

struct GitObject {
    GitObject() = default;
    GitObject(const GitObject&) = delete;
    GitObject& operator=(const GitObject&) = delete;
    GitObject(GitObject&&) = delete;
    GitObject& operator=(GitObject&&) = delete;
    operator git_object*() { return value; }
    operator git_object**() { return &value; }
    ~GitObject() { if (value) { git_object_free(value); value = nullptr; }}
    git_object *value = nullptr;
};

struct GitPatch {
    GitPatch() = default;
    GitPatch(const GitPatch&) = delete;
    GitPatch& operator=(const GitPatch&) = delete;
    GitPatch(GitPatch&&) = delete;
    GitPatch& operator=(GitPatch&&) = delete;
    operator git_patch*() { return value; }
    operator git_patch**() { return &value; }
    ~GitPatch() { if (value) { git_patch_free(value); value = nullptr; }}
    git_patch *value = nullptr;
};

struct GitDescribeResult {
    GitDescribeResult() = default;
    GitDescribeResult(const GitDescribeResult&) = delete;
    GitDescribeResult& operator=(const GitDescribeResult&) = delete;
    GitDescribeResult(GitDescribeResult&&) = delete;
    GitDescribeResult& operator=(GitDescribeResult&&) = delete;
    operator git_describe_result*() { return value; }
    operator git_describe_result**() { return &value; }
    ~GitDescribeResult() { if (value) { git_describe_result_free(value); value = nullptr; }}
    git_describe_result *value = nullptr;
};

struct GitMailmap {
    GitMailmap() = default;
    GitMailmap(const GitMailmap&) = delete;
    GitMailmap& operator=(const GitMailmap&) = delete;
    GitMailmap(GitMailmap&&) = delete;
    GitMailmap& operator=(GitMailmap&&) = delete;
    operator git_mailmap*() { return value; }
    operator git_mailmap**() { return &value; }
    ~GitMailmap() { if (value) { git_mailmap_free(value); value = nullptr; }}
    git_mailmap *value = nullptr;
};

static void resolveMailmap(git_repository *repo, QString &name, QString &email)
{
    GitMailmap mailmap;
    if (git_mailmap_from_repository(mailmap, repo) == 0)
    {
        const char *resolved_name = nullptr;
        const char *resolved_email = nullptr;
        if (git_mailmap_resolve(&resolved_name, &resolved_email, mailmap, name.toUtf8().constData(), email.toUtf8().constData()) == 0)
        {
            if (resolved_name) name = QString::fromUtf8(resolved_name);
            if (resolved_email) email = QString::fromUtf8(resolved_email);
        }
    }
}

static int resolveToCommitOid(git_oid &oid, git_repository *repo, const QString &spec)
{
    GitObject obj;
    int res = git_revparse_single(obj, repo, spec.toUtf8().constData());
    if (res == 0)
    {
        GitCommit peeled;
        if (git_object_peel((git_object **)&peeled.value, obj, GIT_OBJECT_COMMIT) == 0)
        {
            oid = *git_commit_id(peeled);
            return 0;
        }
        oid = *git_object_id(obj);
        return 0;
    }
    return git_oid_fromstr(&oid, spec.toUtf8().constData());
}

struct GitIndex {
    GitIndex() = default;
    GitIndex(const GitIndex&) = delete;
    GitIndex& operator=(const GitIndex&) = delete;
    GitIndex(GitIndex&&) = delete;
    GitIndex& operator=(GitIndex&&) = delete;
    operator git_index*() { return value; }
    operator git_index**() { return &value; }
    ~GitIndex() { if (value) { git_index_free(value); value = nullptr; }}
    git_index *value = nullptr;
};

struct GitConfig {
    GitConfig() = default;
    GitConfig(const GitConfig&) = delete;
    GitConfig& operator=(const GitConfig&) = delete;
    GitConfig(GitConfig&&) = delete;
    GitConfig& operator=(GitConfig&&) = delete;
    operator git_config*() { return value; }
    operator git_config**() { return &value; }
    ~GitConfig() { if (value) { git_config_free(value); value = nullptr; }}
    git_config *value = nullptr;
};

struct GitBuf {
    GitBuf() = default;
    GitBuf(const GitBuf&) = delete;
    GitBuf& operator=(const GitBuf&) = delete;
    GitBuf(GitBuf&&) = delete;
    GitBuf& operator=(GitBuf&&) = delete;
    operator git_buf*() { return &value; }
    ~GitBuf() { git_buf_dispose(&value); }
    git_buf value = GIT_BUF_INIT;
};

struct GitIndexConflictIterator {
    GitIndexConflictIterator() = default;
    GitIndexConflictIterator(const GitIndexConflictIterator&) = delete;
    GitIndexConflictIterator& operator=(const GitIndexConflictIterator&) = delete;
    GitIndexConflictIterator(GitIndexConflictIterator&&) = delete;
    GitIndexConflictIterator& operator=(GitIndexConflictIterator&&) = delete;
    operator git_index_conflict_iterator*() { return value; }
    operator git_index_conflict_iterator**() { return &value; }
    ~GitIndexConflictIterator() { if (value) { git_index_conflict_iterator_free(value); value = nullptr; }}
    git_index_conflict_iterator *value = nullptr;
};

struct GitSubmodule {
    GitSubmodule() = default;
    GitSubmodule(const GitSubmodule&) = delete;
    GitSubmodule& operator=(const GitSubmodule&) = delete;
    GitSubmodule(GitSubmodule&&) = delete;
    GitSubmodule& operator=(GitSubmodule&&) = delete;
    operator git_submodule*() { return value; }
    operator git_submodule**() { return &value; }
    ~GitSubmodule() { if (value) { git_submodule_free(value); value = nullptr; }}
    git_submodule *value = nullptr;
};

struct GitAnnotatedCommit {
    GitAnnotatedCommit() = default;
    GitAnnotatedCommit(const GitAnnotatedCommit&) = delete;
    GitAnnotatedCommit& operator=(const GitAnnotatedCommit&) = delete;
    GitAnnotatedCommit(GitAnnotatedCommit&&) = delete;
    GitAnnotatedCommit& operator=(GitAnnotatedCommit&&) = delete;
    operator git_annotated_commit*() { return value; }
    operator git_annotated_commit**() { return &value; }
    ~GitAnnotatedCommit() { if (value) { git_annotated_commit_free(value); value = nullptr; }}
    git_annotated_commit *value = nullptr;
};

struct GitBlob {
    GitBlob() = default;
    GitBlob(const GitBlob&) = delete;
    GitBlob& operator=(const GitBlob&) = delete;
    GitBlob(GitBlob&&) = delete;
    GitBlob& operator=(GitBlob&&) = delete;
    operator git_blob*() { return value; }
    operator git_blob**() { return &value; }
    ~GitBlob() { if (value) { git_blob_free(value); value = nullptr; }}
    git_blob *value = nullptr;
};

struct GitRevWalk {
    GitRevWalk() = default;
    GitRevWalk(const GitRevWalk&) = delete;
    GitRevWalk& operator=(const GitRevWalk&) = delete;
    GitRevWalk(GitRevWalk&&) = delete;
    GitRevWalk& operator=(GitRevWalk&&) = delete;
    operator git_revwalk*() { return value; }
    operator git_revwalk**() { return &value; }
    ~GitRevWalk() { if (value) { git_revwalk_free(value); value = nullptr; }}
    git_revwalk *value = nullptr;
};

struct GitStrArray {
    GitStrArray() = default;
    GitStrArray(const GitStrArray&) = delete;
    GitStrArray& operator=(const GitStrArray&) = delete;
    GitStrArray(GitStrArray&&) = delete;
    GitStrArray& operator=(GitStrArray&&) = delete;
    operator git_strarray&() { return value; }
    operator git_strarray*() { return &value; }
    ~GitStrArray() {  git_strarray_dispose(&value); value = {nullptr, 0}; }
    git_strarray value = {nullptr, 0};
};


struct GitTag {
    GitTag() = default;
    GitTag(const GitTag&) = delete;
    GitTag& operator=(const GitTag&) = delete;
    GitTag(GitTag&&) = delete;
    GitTag& operator=(GitTag&&) = delete;
    operator git_tag*() { return value; }
    operator git_tag**() { return &value; }
    ~GitTag() {  git_tag_free(value); value = nullptr; }
    git_tag *value = nullptr;
};

#define LINE_END '\n'


static int sshKeyCredentialCallback(
    git_credential **out, 
    const char *url, 
    const char *username_from_url, 
    unsigned int allowed_types, 
    void *payload)
{
    Q_UNUSED(url);
    Q_UNUSED(payload);

    if (allowed_types & GIT_CREDENTIAL_SSH_KEY) {
        int agent_res = git_credential_ssh_key_from_agent(out, username_from_url);
        if (agent_res == 0) {
            return 0;
        }

        QString pass;
        QMetaObject::invokeMethod(QGitMasterMainWindow::instance(), &QGitMasterMainWindow::getPassword, 
                                  Qt::BlockingQueuedConnection, Q_RETURN_ARG(QString, pass));

        if (!pass.isEmpty())
        {
            QString sshDir = QDir::homePath() + "/.ssh";
            QDir dir(sshDir);
            
            QStringList preferredKeys = {"id_ed25519", "id_rsa", "id_ecdsa", "id_dsa"};
            QFileInfo pubkeyFileInfo;
            QFileInfo privkeyFileInfo;
            bool found = false;

            for (const QString &keyName : preferredKeys) {
                QFileInfo privKey(dir, keyName);
                QFileInfo pubKey(dir, keyName + ".pub");
                if (privKey.exists() && pubKey.exists()) {
                    pubkeyFileInfo = pubKey;
                    privkeyFileInfo = privKey;
                    found = true;
                    break;
                }
            }

            if (!found) {
                auto keys = dir.entryInfoList({"*.pub"});
                if (keys.size() == 1)
                {
                    pubkeyFileInfo = keys.first();
                    privkeyFileInfo = QFileInfo(pubkeyFileInfo.dir(), pubkeyFileInfo.completeBaseName());
                    if (privkeyFileInfo.exists()) {
                        found = true;
                    }
                }
            }

            if (found)
            {
                auto pubkeyPathname = pubkeyFileInfo.absoluteFilePath().toUtf8();
                auto privkeyPathname = privkeyFileInfo.absoluteFilePath().toUtf8();

                git_credential_ssh_key_new(out, username_from_url, pubkeyPathname, privkeyPathname, pass.toUtf8().constData());
                return 0;
            }
        }
    }

    return -1;
}


QGit::QGit(const QDir &path, QObject *parent)
    : QObject(parent)
    , m_path(path)
{
}

QGit::QGit(QObject *parent)
    : QObject(parent)
{
}

bool QGit::setPath(const QDir &path)
{
    m_path = path;

    return true;
}

QDir QGit::path()
{
    return m_path;
}

QList<QGitRemote> QGit::remotes() const
{
    QList<QGitRemote> ret;

    GitRepository repo;
    int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
    if (res)
    {
        return ret;
    }

    GitStrArray remotes;
    git_remote_list(remotes, repo);

    for(size_t c = 0; c < remotes.value.count; c++)
    {
        GitRemote remote;

        const char *remote_name = remotes.value.strings[c];

        if (git_remote_lookup(remote, repo, remote_name) < 0)
        {
            continue;
        }

        QGitRemote item;

        const char *remote_url = git_remote_url(remote);

        item.name = QString::fromUtf8(remote_name);
        item.url = QString::fromUtf8(remote_url);

        ret.append(item);
    }

    return ret;
}

bool QGit::hasCommitsToPush() const
{
    if (m_path.absolutePath().isEmpty())
        return false;

    GitRepository repo;
    int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
    if (res)
    {
        return false;
    }

    GitBranchIterator it;
    res = git_branch_iterator_new(it, repo, GIT_BRANCH_LOCAL);
    if (res)
    {
        return false;
    }

    bool hasCommits = false;
    git_branch_t type;

    forever
    {
        GitReference ref;
        if (git_branch_next(ref, &type, it) != 0)
        {
            break;
        }

        const git_oid *local_oid = git_reference_target(ref);
        if (!local_oid)
        {
            continue;
        }

        GitReference upstream;
        res = git_branch_upstream(upstream, ref);
        if (res == GIT_ENOTFOUND || res < 0)
        {
            int unborn = git_repository_head_unborn(repo);
            if (unborn == 0)
            {
                hasCommits = true;
                break;
            }
            continue;
        }

        const git_oid *upstream_oid = git_reference_target(upstream);
        if (upstream_oid)
        {
            size_t ahead = 0;
            size_t behind = 0;
            res = git_graph_ahead_behind(&ahead, &behind, repo, local_oid, upstream_oid);
            if (res == 0 && ahead > 0)
            {
                hasCommits = true;
                break;
            }
        }
        else
        {
            hasCommits = true;
            break;
        }
    }

    return hasCommits;
}

QString QGit::currentBranch() const
{
    GitRepository repo;
    int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
    if (res)
    {
        return QString();
    }

    GitReference ref;
    res = git_repository_head(ref, repo);
    if (res)
    {
        return QString();
    }

    const char *branch = nullptr;
    res = git_branch_name(&branch, ref);
    if (res)
    {
        return QString();
    }

    return branch;
}

QString QGit::headCommitId() const
{
    GitRepository repo;
    int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
    if (res)
    {
        return QString();
    }

    GitReference ref;
    res = git_repository_head(ref, repo);
    if (res)
    {
        return QString();
    }

    const git_oid *oid = git_reference_target(ref);
    if (oid)
    {
        return QString::fromUtf8(git_oid_tostr_s(oid));
    }
    return QString();
}

QString QGit::headCommitMessage() const
{
    GitRepository repo;
    int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
    if (res)
    {
        return QString();
    }

    GitReference ref;
    res = git_repository_head(ref, repo);
    if (res)
    {
        return QString();
    }

    const git_oid *oid = git_reference_target(ref);
    if (!oid)
    {
        return QString();
    }

    GitCommit commit;
    res = git_commit_lookup(commit, repo, oid);
    if (res)
    {
        return QString();
    }

    const char *message = git_commit_message(commit);
    if (message)
    {
        return QString::fromUtf8(message);
    }
    return QString();
}

bool QGit::isAncestor(const QString &ancestor, const QString &descendant) const
{
    GitRepository repo;
    int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
    if (res)
    {
        return false;
    }

    git_oid ancestor_oid, descendant_oid;
    res = resolveToCommitOid(ancestor_oid, repo, ancestor);
    if (res) return false;
    res = resolveToCommitOid(descendant_oid, repo, descendant);
    if (res) return false;

    // Returns 1 if descendant is a descendant of ancestor (meaning ancestor is an ancestor of descendant)
    return git_graph_descendant_of(repo, &descendant_oid, &ancestor_oid) == 1;
}

QList<QGitBranch> QGit::branches(git_branch_t type) const
{
    QList<QGitBranch> ret;

    GitRepository repo;
    int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
    if (res)
    {
        return ret;
    }

    GitBranchIterator it;
    res = git_branch_iterator_new(it, repo, type);
    if (res)
    {
        return ret;
    }

    forever
    {
        git_branch_t type;

        GitReference ref;
        if (git_branch_next(ref, &type, it) != 0)
        {
            break;
        }

        const char *name = nullptr;
        res = git_branch_name(&name, ref);
        if (res)
        {
            return ret;
        }

        if (git_reference_type(ref) == GIT_REFERENCE_DIRECT)
        {
            GitObject obj;
            res = git_reference_peel(obj, ref, GIT_OBJ_COMMIT);
            if (res)
            {
                throw QGitError("git_reference_peel", res);
            }

            git_time_t commit_time = git_commit_time(reinterpret_cast<const git_commit *>(obj.value));
            char oid_str[GIT_OID_HEXSZ + 1];
            git_oid_tostr(oid_str, sizeof(oid_str), git_object_id(obj));

            QGitBranch branch = QGitBranch(name, QString::fromLatin1(oid_str), commit_time, type);
            ret.append(branch);
        }
    }

    return ret;
}

bool QGit::hasCommitId(const QString &commitId) const
{
    GitRepository repo;
    GitObject obj;

    int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
    if (res)
    {
        return false;
    }

    res = git_revparse_single(obj, repo, commitId.toUtf8().constData());
    if (res)
    {
        return false;
    }

    return true;
}

QString QGit::getBranchNameFromPath(const QString &path)
{
    QString ret;

    GitRepository repo;
    int res = git_repository_open(repo, path.toUtf8().constData());
    if (res)
    {
        return ret;
    }

    GitReference ref;
    res = git_repository_head(ref, repo);
    if (res)
    {
        return ret;
    }

    const char *branch = nullptr;
    git_branch_name(&branch, ref);

    return branch;
}

int QGit::createLocalRepository(const QDir &path)
{
    GitRepository repo;
    int res = git_repository_init(repo, path.absolutePath().toUtf8().constData(), 0);

    return res;
}

bool QGit::isGitRepository(const QDir &path)
{
    GitRepository repo;
    int res = git_repository_open(repo, path.absolutePath().toUtf8().constData());
    if (res)
    {
        return false;
    }

    return true;
}

bool QGit::gitRepositoryDefaultSignature(const QDir &path, QString &name, QString &email)
{
    GitRepository repo;
    int res = git_repository_open(repo, path.absolutePath().toUtf8().constData());
    if (res)
    {
        return false;
    }

    GitSignature me;
    res = git_signature_default(me, repo);
    if (res)
    {
        return false;
    }

    name = me.value->name;
    email = me.value->email;

    return true;
}

void QGit::createLocalBranch(const QString &name, const QString &commit_id, bool checkout, bool force)
{
    GitRepository repo;
    GitReference branch;
    GitObject target_obj;
    GitCommit commit_obj;
    git_oid oid;

    int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
    if(res)
    {
        throw QGitError("git_repository_open", res);
    }

    if (commit_id.isEmpty())
    {

        res = git_reference_name_to_id(&oid, repo, "HEAD");
        if(res)
        {
            throw QGitError("git_reference_name_to_id", res);
        }
    }
    else
    {
        res = resolveToCommitOid(oid, repo, commit_id);
        if(res)
        {
            throw QGitError("resolveToCommitOid", res);
        }
    }

    res = git_commit_lookup(commit_obj, repo, &oid);
    if(res)
    {
        throw QGitError("git_commit_lookup", res);
    }

    res = git_branch_create(branch, repo, name.toUtf8().constData(), commit_obj, force);
    if(res)
    {
        throw QGitError("git_branch_create", res);
    }

    if (checkout)
    {
        GitObject treeish;
        git_checkout_options opts = GIT_CHECKOUT_OPTIONS_INIT;

        opts.checkout_strategy = GIT_CHECKOUT_SAFE;

        QString refName = "refs/heads/" + name;

        res = git_revparse_single(treeish, repo, refName.toUtf8().constData());
        if(res)
        {
            throw QGitError("git_revparse_single", res);
        }

        res = git_checkout_tree(repo, treeish, &opts);
        if(res)
        {
            throw QGitError("git_checkout_tree", res);
        }

        res = git_repository_set_head(repo, refName.toUtf8().constData());
        if(res)
        {
            throw QGitError("git_repository_set_head", res);
        }
    }
}

void QGit::cherrypick(const QString &commitId)
{
    GitRepository repo;
    int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
    if (res)
    {
        throw QGitError("git_repository_open", res);
    }

    git_repository_state_t state = (git_repository_state_t)git_repository_state(repo);
    if (state != GIT_REPOSITORY_STATE_NONE)
    {
        throw QGitError(tr("Repository is not in a clean state. Please resolve or abort the active merge, rebase, or cherry-pick first."), -1);
    }

    git_oid oid;
    res = resolveToCommitOid(oid, repo, commitId);
    if (res)
    {
        throw QGitError("resolveToCommitOid", res);
    }

    GitCommit commit;
    res = git_commit_lookup(commit, repo, &oid);
    if (res)
    {
        throw QGitError("git_commit_lookup", res);
    }

    git_cherrypick_options opts = GIT_CHERRYPICK_OPTIONS_INIT;
    opts.checkout_opts.checkout_strategy = GIT_CHECKOUT_SAFE;

    res = git_cherrypick(repo, commit, &opts);
    if (res)
    {
        throw QGitError("git_cherrypick", res);
    }

    GitIndex index;
    res = git_repository_index(index, repo);
    if (res)
    {
        throw QGitError("git_repository_index", res);
    }

    if (git_index_has_conflicts(index))
    {
        throw QGitError(tr("Cherry-pick completed with conflicts. Please resolve conflicts and commit manually."), -1);
    }

    // No conflicts, auto-commit
    git_oid tree_id;
    res = git_index_write_tree(&tree_id, index);
    if (res)
    {
        throw QGitError("git_index_write_tree", res);
    }

    GitTree tree;
    res = git_tree_lookup(tree, repo, &tree_id);
    if (res)
    {
        throw QGitError("git_tree_lookup", res);
    }

    GitSignature committer;
    res = git_signature_default(committer, repo);
    if (res)
    {
        throw QGitError("git_signature_default", res);
    }

    const git_signature *author = git_commit_author(commit);
    const char *message = git_commit_message(commit);

    GitReference head_ref;
    res = git_repository_head(head_ref, repo);
    if (res)
    {
        throw QGitError("git_repository_head", res);
    }

    GitCommit parent;
    res = git_commit_lookup(parent, repo, git_reference_target(head_ref));
    if (res)
    {
        throw QGitError("git_commit_lookup (HEAD)", res);
    }

    git_oid new_commit_id;
    const git_commit *parents[] = { parent.value };
    res = git_commit_create(
        &new_commit_id,
        repo,
        "HEAD",
        author,
        committer,
        nullptr,
        message,
        tree,
        1,
        parents
    );
    if (res)
    {
        throw QGitError("git_commit_create", res);
    }

    git_repository_state_cleanup(repo);
}

void QGit::revert(const QString &commitId)
{
    GitRepository repo;
    int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
    if (res)
    {
        throw QGitError("git_repository_open", res);
    }

    git_repository_state_t state = (git_repository_state_t)git_repository_state(repo);
    if (state != GIT_REPOSITORY_STATE_NONE)
    {
        throw QGitError(tr("Repository is not in a clean state. Please resolve or abort the active merge, rebase, or cherry-pick first."), -1);
    }

    git_oid oid;
    res = resolveToCommitOid(oid, repo, commitId);
    if (res)
    {
        throw QGitError("resolveToCommitOid", res);
    }

    GitCommit commit;
    res = git_commit_lookup(commit, repo, &oid);
    if (res)
    {
        throw QGitError("git_commit_lookup", res);
    }

    git_revert_options opts = GIT_REVERT_OPTIONS_INIT;
    opts.checkout_opts.checkout_strategy = GIT_CHECKOUT_SAFE;

    res = git_revert(repo, commit, &opts);
    if (res)
    {
        throw QGitError("git_revert", res);
    }

    GitIndex index;
    res = git_repository_index(index, repo);
    if (res)
    {
        throw QGitError("git_repository_index", res);
    }

    if (git_index_has_conflicts(index))
    {
        throw QGitError(tr("Revert completed with conflicts. Please resolve conflicts and commit manually."), -1);
    }

    // No conflicts, auto-commit
    git_oid tree_id;
    res = git_index_write_tree(&tree_id, index);
    if (res)
    {
        throw QGitError("git_index_write_tree", res);
    }

    GitTree tree;
    res = git_tree_lookup(tree, repo, &tree_id);
    if (res)
    {
        throw QGitError("git_tree_lookup", res);
    }

    GitSignature me;
    res = git_signature_default(me, repo);
    if (res)
    {
        throw QGitError("git_signature_default", res);
    }

    // Extract summary of the reverted commit's message
    const char *orig_message = git_commit_message(commit);
    QString summary;
    if (orig_message)
    {
        QString fullMsg = QString::fromUtf8(orig_message);
        summary = fullMsg.split('\n').first().trimmed();
    }
    else
    {
        summary = "unknown commit";
    }

    QString revertMsg = QString("Revert \"%1\"\n\nThis reverts commit %2.\n").arg(summary, commitId);

    GitReference head_ref;
    res = git_repository_head(head_ref, repo);
    if (res)
    {
        throw QGitError("git_repository_head", res);
    }

    GitCommit parent;
    res = git_commit_lookup(parent, repo, git_reference_target(head_ref));
    if (res)
    {
        throw QGitError("git_commit_lookup (HEAD)", res);
    }

    git_oid new_commit_id;
    const git_commit *parents[] = { parent.value };
    res = git_commit_create(
        &new_commit_id,
        repo,
        "HEAD",
        me,
        me,
        nullptr,
        revertMsg.toUtf8().constData(),
        tree,
        1,
        parents
    );
    if (res)
    {
        throw QGitError("git_commit_create", res);
    }

    git_repository_state_cleanup(repo);
}

void QGit::reset(const QString &commitId, git_reset_t type)
{
    GitRepository repo;
    int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
    if (res)
    {
        throw QGitError("git_repository_open", res);
    }

    git_oid oid;
    res = resolveToCommitOid(oid, repo, commitId);
    if (res)
    {
        throw QGitError("resolveToCommitOid", res);
    }

    GitObject obj;
    res = git_object_lookup(obj, repo, &oid, GIT_OBJECT_COMMIT);
    if (res)
    {
        throw QGitError("git_object_lookup", res);
    }

    git_checkout_options opts = GIT_CHECKOUT_OPTIONS_INIT;
    opts.checkout_strategy = GIT_CHECKOUT_SAFE;

    res = git_reset(repo, obj, type, &opts);
    if (res)
    {
        throw QGitError("git_reset", res);
    }

    if (type == GIT_RESET_HARD || type == GIT_RESET_MIXED)
    {
        git_repository_state_cleanup(repo);
    }
}

QString QGit::configString(const QString &key) const
{
    GitRepository repo;
    GitConfig cfg;
    int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
    if (res == 0)
    {
        res = git_repository_config(cfg, repo);
    }
    else
    {
        res = git_config_open_default(cfg);
    }

    if (res)
    {
        throw QGitError("git_config_open", res);
    }

    GitBuf buf;
    res = git_config_get_string_buf(buf, cfg, key.toUtf8().constData());
    if (res == GIT_ENOTFOUND)
    {
        return QString();
    }
    if (res)
    {
        throw QGitError("git_config_get_string_buf", res);
    }

    return QString::fromUtf8(buf.value.ptr, buf.value.size);
}

void QGit::setConfigString(const QString &key, const QString &value, bool global)
{
    int res;
    GitConfig write_cfg;

    if (global)
    {
        GitConfig cascade_cfg;
        res = git_config_open_default(cascade_cfg);
        if (res) throw QGitError("git_config_open_default", res);

        res = git_config_open_global(write_cfg, cascade_cfg);
        if (res) throw QGitError("git_config_open_global", res);
    }
    else
    {
        GitRepository repo;
        res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if (res) throw QGitError("git_repository_open", res);

        GitConfig cascade_cfg;
        res = git_repository_config(cascade_cfg, repo);
        if (res) throw QGitError("git_repository_config", res);

        res = git_config_open_level(write_cfg, cascade_cfg, GIT_CONFIG_LEVEL_LOCAL);
        if (res) throw QGitError("git_config_open_level", res);
    }

    res = git_config_set_string(write_cfg, key.toUtf8().constData(), value.toUtf8().constData());
    if (res) throw QGitError("git_config_set_string", res);
}

void QGit::deleteConfigEntry(const QString &key, bool global)
{
    int res;
    GitConfig write_cfg;

    if (global)
    {
        GitConfig cascade_cfg;
        res = git_config_open_default(cascade_cfg);
        if (res) throw QGitError("git_config_open_default", res);

        res = git_config_open_global(write_cfg, cascade_cfg);
        if (res) throw QGitError("git_config_open_global", res);
    }
    else
    {
        GitRepository repo;
        res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if (res) throw QGitError("git_repository_open", res);

        GitConfig cascade_cfg;
        res = git_repository_config(cascade_cfg, repo);
        if (res) throw QGitError("git_repository_config", res);

        res = git_config_open_level(write_cfg, cascade_cfg, GIT_CONFIG_LEVEL_LOCAL);
        if (res) throw QGitError("git_config_open_level", res);
    }

    res = git_config_delete_entry(write_cfg, key.toUtf8().constData());
    if (res && res != GIT_ENOTFOUND)
    {
        throw QGitError("git_config_delete_entry", res);
    }
}

static int configForeachCallback(const git_config_entry *entry, void *payload)
{
    QMap<QString, QString> *map = static_cast<QMap<QString, QString>*>(payload);
    map->insert(QString::fromUtf8(entry->name), QString::fromUtf8(entry->value));
    return 0;
}

QMap<QString, QString> QGit::configEntries() const
{
    QMap<QString, QString> entries;
    GitRepository repo;
    GitConfig cfg;
    int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
    if (res == 0)
    {
        res = git_repository_config(cfg, repo);
    }
    else
    {
        res = git_config_open_default(cfg);
    }

    if (res == 0)
    {
        git_config_foreach(cfg, configForeachCallback, &entries);
    }
    return entries;
}

bool QGit::hasConflicts() const
{
    GitRepository repo;
    int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
    if (res) return false;

    GitIndex index;
    res = git_repository_index(index, repo);
    if (res) return false;

    return git_index_has_conflicts(index) != 0;
}

QList<QString> QGit::conflictedFiles() const
{
    QList<QString> files;
    GitRepository repo;
    int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
    if (res) return files;

    GitIndex index;
    res = git_repository_index(index, repo);
    if (res) return files;

    GitIndexConflictIterator iter;
    res = git_index_conflict_iterator_new(iter, index);
    if (res) return files;

    const git_index_entry *ancestor = nullptr;
    const git_index_entry *our = nullptr;
    const git_index_entry *their = nullptr;

    while (git_index_conflict_next(&ancestor, &our, &their, iter) == 0)
    {
        const git_index_entry *valid = ancestor ? ancestor : (our ? our : their);
        if (valid)
        {
            QString p = QString::fromUtf8(valid->path);
            if (!files.contains(p))
            {
                files.append(p);
            }
        }
    }

    return files;
}

bool QGit::conflictContents(const QString &path, QString &ancestor, QString &ours, QString &theirs) const
{
    ancestor.clear();
    ours.clear();
    theirs.clear();

    GitRepository repo;
    int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
    if (res) throw QGitError("git_repository_open", res);

    GitIndex index;
    res = git_repository_index(index, repo);
    if (res) throw QGitError("git_repository_index", res);

    const git_index_entry *ancestor_entry = nullptr;
    const git_index_entry *our_entry = nullptr;
    const git_index_entry *their_entry = nullptr;

    res = git_index_conflict_get(&ancestor_entry, &our_entry, &their_entry, index, path.toUtf8().constData());
    if (res == GIT_ENOTFOUND)
    {
        return false;
    }
    if (res)
    {
        throw QGitError("git_index_conflict_get", res);
    }

    auto readBlob = [&](const git_index_entry *entry, QString &outText) {
        if (!entry) return;
        git_blob *blob = nullptr;
        int r = git_blob_lookup(&blob, repo, &entry->id);
        if (r == 0)
        {
            outText = QString::fromUtf8((const char *)git_blob_rawcontent(blob), git_blob_rawsize(blob));
            git_blob_free(blob);
        }
    };

    readBlob(ancestor_entry, ancestor);
    readBlob(our_entry, ours);
    readBlob(their_entry, theirs);

    return true;
}

void QGit::resolveConflict(const QString &path, const QString &resolvedContent)
{
    // 1. Write the resolved content to working directory file
    QString fullPath = m_path.absoluteFilePath(path);
    QFile file(fullPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        throw QGitError("Could not open file to write resolved content: " + fullPath, -1);
    }
    QTextStream out(&file);
    out << resolvedContent;
    file.close();

    // 2. Clear conflict stages and stage resolved file in the Git index
    GitRepository repo;
    int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
    if (res) throw QGitError("git_repository_open", res);

    GitIndex index;
    res = git_repository_index(index, repo);
    if (res) throw QGitError("git_repository_index", res);

    res = git_index_conflict_remove(index, path.toUtf8().constData());
    if (res && res != GIT_ENOTFOUND) throw QGitError("git_index_conflict_remove", res);

    res = git_index_add_bypath(index, path.toUtf8().constData());
    if (res) throw QGitError("git_index_add_bypath", res);

    res = git_index_write(index);
    if (res) throw QGitError("git_index_write", res);
}

static int submoduleForeachCallback(git_submodule *sm, const char *name, void *payload)
{
    QList<QGitSubmodule> *list = static_cast<QList<QGitSubmodule>*>(payload);
    QGitSubmodule sub;
    sub.name = QString::fromUtf8(git_submodule_name(sm));
    sub.path = QString::fromUtf8(git_submodule_path(sm));

    const char *url = git_submodule_url(sm);
    if (url) sub.url = QString::fromUtf8(url);

    auto formatOid = [](const git_oid *oid) {
        if (!oid || git_oid_is_zero(oid)) return QString();
        char buf[GIT_OID_HEXSZ + 1];
        git_oid_tostr(buf, sizeof(buf), oid);
        return QString::fromLatin1(buf);
    };

    sub.headId = formatOid(git_submodule_head_id(sm));
    sub.indexId = formatOid(git_submodule_index_id(sm));
    sub.wdId = formatOid(git_submodule_wd_id(sm));

    git_repository *repo = git_submodule_owner(sm);
    unsigned int status = 0;
    if (git_submodule_status(&status, repo, name, GIT_SUBMODULE_IGNORE_UNSPECIFIED) == 0) {
        sub.status = status;
    }

    list->append(sub);
    return 0;
}

QList<QGitSubmodule> QGit::submodules() const
{
    QList<QGitSubmodule> list;
    GitRepository repo;
    int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
    if (res == 0)
    {
        git_submodule_foreach(repo, submoduleForeachCallback, &list);
    }
    return list;
}

void QGit::initSubmodule(const QString &name)
{
    GitRepository repo;
    int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
    if (res) throw QGitError("git_repository_open", res);

    GitSubmodule sm;
    res = git_submodule_lookup(sm, repo, name.toUtf8().constData());
    if (res) throw QGitError("git_submodule_lookup", res);

    res = git_submodule_init(sm, 0);
    if (res) throw QGitError("git_submodule_init", res);
}

void QGit::syncSubmodule(const QString &name)
{
    GitRepository repo;
    int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
    if (res) throw QGitError("git_repository_open", res);

    GitSubmodule sm;
    res = git_submodule_lookup(sm, repo, name.toUtf8().constData());
    if (res) throw QGitError("git_submodule_lookup", res);

    res = git_submodule_sync(sm);
    if (res) throw QGitError("git_submodule_sync", res);
}

void QGit::addRemote(const QString &name, const QString &url)
{
    GitRepository repo;
    int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
    if (res) throw QGitError("git_repository_open", res);

    GitRemote remote;
    res = git_remote_create(remote, repo, name.toUtf8().constData(), url.toUtf8().constData());
    if (res) throw QGitError("git_remote_create", res);
}

void QGit::deleteRemote(const QString &name)
{
    GitRepository repo;
    int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
    if (res) throw QGitError("git_repository_open", res);

    res = git_remote_delete(repo, name.toUtf8().constData());
    if (res) throw QGitError("git_remote_delete", res);
}

void QGit::renameRemote(const QString &oldName, const QString &newName)
{
    GitRepository repo;
    int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
    if (res) throw QGitError("git_repository_open", res);

    git_strarray problems = {nullptr, 0};
    res = git_remote_rename(&problems, repo, oldName.toUtf8().constData(), newName.toUtf8().constData());
    git_strarray_free(&problems);
    if (res) throw QGitError("git_remote_rename", res);
}

void QGit::setRemoteUrl(const QString &name, const QString &url, bool isPushUrl)
{
    GitRepository repo;
    int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
    if (res) throw QGitError("git_repository_open", res);

    if (isPushUrl) {
        res = git_remote_set_pushurl(repo, name.toUtf8().constData(), url.toUtf8().constData());
        if (res) throw QGitError("git_remote_set_pushurl", res);
    } else {
        res = git_remote_set_url(repo, name.toUtf8().constData(), url.toUtf8().constData());
        if (res) throw QGitError("git_remote_set_url", res);
    }
}

void QGit::updateSubmodule(QString name)
{
    QGitError error;
    try {
        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if (res) throw QGitError("git_repository_open", res);

        GitSubmodule sm;
        res = git_submodule_lookup(sm, repo, name.toUtf8().constData());
        if (res) throw QGitError("git_submodule_lookup", res);

        git_submodule_update_options opts = GIT_SUBMODULE_UPDATE_OPTIONS_INIT;
        opts.checkout_opts.checkout_strategy = GIT_CHECKOUT_SAFE;
        opts.fetch_opts.callbacks.credentials = sshKeyCredentialCallback;

        res = git_submodule_update(sm, 1, &opts);
        if (res) throw QGitError("git_submodule_update", res);

    } catch (const QGitError &ex) {
        error = ex;
    }
    emit updateSubmoduleReply(error);
}

void QGit::checkoutBranch(QString name)
{
    QGitError error;
    try {
        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if(res) throw QGitError("git_repository_open", res);

        GitObject treeish;
        git_checkout_options opts = GIT_CHECKOUT_OPTIONS_INIT;
        opts.checkout_strategy = GIT_CHECKOUT_SAFE;

        QString refName = "refs/heads/" + name;
        res = git_revparse_single(treeish, repo, refName.toUtf8().constData());
        if(res) throw QGitError("git_revparse_single", res);

        res = git_checkout_tree(repo, treeish, &opts);
        if(res) throw QGitError("git_checkout_tree", res);

        res = git_repository_set_head(repo, refName.toUtf8().constData());
        if(res) throw QGitError("git_repository_set_head", res);

    } catch(const QGitError &ex) {
        error = ex;
    }
    emit checkoutBranchReply(error);
}

void QGit::renameBranch(QString oldName, QString newName)
{
    QGitError error;
    try {
        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if(res) throw QGitError("git_repository_open", res);

        // 1. Check if name is already present
        GitReference existing_branch;
        res = git_branch_lookup(existing_branch, repo, newName.toUtf8().constData(), GIT_BRANCH_LOCAL);
        if (res == 0) {
            throw QGitError(tr("Branch '%1' already exists.").arg(newName), -1);
        }

        // 2. Lookup old branch
        GitReference branch;
        res = git_branch_lookup(branch, repo, oldName.toUtf8().constData(), GIT_BRANCH_LOCAL);
        if(res) throw QGitError("git_branch_lookup", res);

        int is_head = git_branch_is_head(branch);

        // 3. Create new branch on same commit
        git_object *target_obj = nullptr;
        res = git_reference_peel(&target_obj, branch, GIT_OBJ_COMMIT);
        if (res) throw QGitError("git_reference_peel", res);

        git_commit *commit = (git_commit *)target_obj;

        GitReference new_branch;
        res = git_branch_create(new_branch, repo, newName.toUtf8().constData(), commit, 0);
        git_object_free(target_obj);
        if (res) throw QGitError("git_branch_create", res);

        // 4. Update HEAD if we renamed current branch
        if (is_head == 1) {
            res = git_repository_set_head(repo, git_reference_name(new_branch));
            if (res) throw QGitError("git_repository_set_head", res);
        }

        // 5. Delete old branch
        res = git_branch_delete(branch);
        if (res) throw QGitError("git_branch_delete", res);

    } catch(const QGitError &ex) {
        error = ex;
    }
    emit renameBranchReply(error);
}

void QGit::renameTag(QString oldName, QString newName)
{
    QGitError error;
    try {
        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if(res) throw QGitError("git_repository_open", res);

        // 1. Check if tag name is already present
        GitReference existing_tag;
        QString newTagFullName = "refs/tags/" + newName;
        res = git_reference_lookup(existing_tag, repo, newTagFullName.toUtf8().constData());
        if (res == 0) {
            throw QGitError(tr("Tag '%1' already exists.").arg(newName), -1);
        }

        // 2. Lookup old tag reference
        GitReference tag_ref;
        QString oldTagFullName = "refs/tags/" + oldName;
        res = git_reference_lookup(tag_ref, repo, oldTagFullName.toUtf8().constData());
        if (res) throw QGitError("git_reference_lookup", res);

        // 3. Get target object of old tag
        git_object *target_obj = nullptr;
        res = git_reference_peel(&target_obj, tag_ref, GIT_OBJ_ANY);
        if (res) throw QGitError("git_reference_peel", res);

        // 4. Create new tag pointing to same target object
        git_oid new_tag_oid;
        res = git_tag_create_lightweight(&new_tag_oid, repo, newName.toUtf8().constData(), target_obj, 0);
        git_object_free(target_obj);
        if (res) throw QGitError("git_tag_create_lightweight", res);

        // 5. Delete old tag
        res = git_tag_delete(repo, oldName.toUtf8().constData());
        if (res) throw QGitError("git_tag_delete", res);

    } catch(const QGitError &ex) {
        error = ex;
    }
    emit renameTagReply(error);
}

void QGit::deleteTag(QString name)
{
    QGitError error;
    try {
        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if(res) throw QGitError("git_repository_open", res);

        res = git_tag_delete(repo, name.toUtf8().constData());
        if(res) throw QGitError("git_tag_delete", res);

    } catch(const QGitError &ex) {
        error = ex;
    }
    emit deleteTagReply(error);
}

void QGit::createTag(QString name, QString targetObjectId, QString message, bool force)
{
    QGitError error;
    try {
        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if (res) throw QGitError("git_repository_open", res);

        // Resolve the target OID from the commit hash string
        git_oid target_oid;
        res = resolveToCommitOid(target_oid, repo, targetObjectId);
        if (res) throw QGitError("resolveToCommitOid", res);

        // Look up the target git object (commit)
        GitObject target_obj;
        res = git_object_lookup(target_obj, repo, &target_oid, GIT_OBJECT_ANY);
        if (res) throw QGitError("git_object_lookup", res);

        git_oid tag_oid;
        if (message.isEmpty()) {
            // Create a lightweight tag
            res = git_tag_create_lightweight(&tag_oid, repo, name.toUtf8().constData(), target_obj, force ? 1 : 0);
            if (res) throw QGitError("git_tag_create_lightweight", res);
        } else {
            // Create an annotated tag (requires a tagger signature)
            GitSignature tagger;
            res = git_signature_default(tagger, repo);
            if (res) throw QGitError("git_signature_default", res);

            res = git_tag_create(&tag_oid, repo, name.toUtf8().constData(), target_obj, tagger, message.toUtf8().constData(), force ? 1 : 0);
            if (res) throw QGitError("git_tag_create", res);
        }

    } catch (const QGitError &ex) {
        error = ex;
    }
    emit createTagReply(error);
}

void QGit::clean(bool includeIgnored, bool removeDirectories)
{
    QGitError error;
    try {
        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if (res) throw QGitError("git_repository_open", res);

        git_status_options opts = GIT_STATUS_OPTIONS_INIT;
        opts.show = GIT_STATUS_SHOW_INDEX_AND_WORKDIR;
        opts.flags = GIT_STATUS_OPT_INCLUDE_UNTRACKED | GIT_STATUS_OPT_RECURSE_UNTRACKED_DIRS;

        if (includeIgnored) {
            opts.flags |= GIT_STATUS_OPT_INCLUDE_IGNORED | GIT_STATUS_OPT_RECURSE_IGNORED_DIRS;
        }

        GitStatusList list;
        res = git_status_list_new(list, repo, &opts);
        if (res) throw QGitError("git_status_list_new", res);

        size_t count = git_status_list_entrycount(list);
        for (size_t i = 0; i < count; ++i) {
            const git_status_entry *entry = git_status_byindex(list, i);
            if (!entry) continue;

            const git_diff_delta *delta = entry->index_to_workdir;
            if (!delta) continue;

            QString relPath = QString::fromUtf8(delta->new_file.path);
            QString absPath = m_path.absoluteFilePath(relPath);

            git_status_t status = entry->status;
            bool isUntracked = (status & GIT_STATUS_WT_NEW);
            bool isIgnored = (status & GIT_STATUS_IGNORED);

            if (isUntracked || isIgnored) {
                QFile::remove(absPath);
            }
        }

        if (removeDirectories) {
            std::function<void(const QString&)> pruneEmptyDirs = [&](const QString &dirPath) {
                QDir dir(dirPath);
                QStringList subDirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden);
                for (const QString &sub : subDirs) {
                    if (sub == ".git") continue;
                    pruneEmptyDirs(dir.absoluteFilePath(sub));
                }
                if (dirPath != m_path.absolutePath()) {
                    if (dir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden).isEmpty()) {
                        dir.rmdir(dirPath);
                    }
                }
            };
            pruneEmptyDirs(m_path.absolutePath());
        }

    } catch (const QGitError &ex) {
        error = ex;
    }
    emit cleanReply(error);
}

void QGit::applyPatch(QString patchPath)
{
    QGitError error;
    try {
        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if (res) throw QGitError("git_repository_open", res);

        QFile file(patchPath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            throw QGitError(QString("Could not open patch file: %1").arg(patchPath), -1);
        }

        QByteArray content = file.readAll();
        file.close();

        GitDiff diff;
        res = git_diff_from_buffer(diff, content.constData(), content.length());
        if (res) throw QGitError("git_diff_from_buffer", res);

        git_apply_options opts = GIT_APPLY_OPTIONS_INIT;
        res = git_apply(repo, diff, GIT_APPLY_LOCATION_WORKDIR, &opts);
        if (res) throw QGitError("git_apply", res);

    } catch (const QGitError &ex) {
        error = ex;
    }
    emit applyPatchReply(error);
}

void QGit::setUpstream(QString branchName, QString upstreamBranchName)
{
    QGitError error;
    try {
        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if(res) throw QGitError("git_repository_open", res);

        GitReference branch;
        res = git_branch_lookup(branch, repo, branchName.toUtf8().constData(), GIT_BRANCH_LOCAL);
        if(res) throw QGitError("git_branch_lookup", res);

        res = git_branch_set_upstream(branch, upstreamBranchName.toUtf8().constData());
        if(res) throw QGitError("git_branch_set_upstream", res);

    } catch(const QGitError &ex) {
        error = ex;
    }
    emit setUpstreamReply(error);
}

void QGit::deleteBranches(QList<QGitBranch> branches, bool force)
{
    QGitError error;

    try {
        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if(res)
        {
            throw QGitError("git_repository_open", res);
        }

        for(const auto &branch: branches)
        {
            if (branch.type() == GIT_BRANCH_LOCAL)
            {
                GitReference branch_ref;
                res = git_branch_lookup(branch_ref, repo, branch.name().toUtf8().constData(), GIT_BRANCH_LOCAL);
                if (res)
                {
                    throw QGitError("git_branch_lookup", res);
                }

                res = git_branch_delete(branch_ref);
                if (res)
                {
                    throw QGitError("git_branch_delete", res);
                }
            }
            else
            {
                QStringList parts = branch.name().split('/');
                if (parts.size() >= 4 && parts[0] == "refs" && parts[1] == "remotes")
                {
                    QString remoteName = parts[2];
                    QString branchName = parts.mid(3).join('/');
                    
                    GitRemote remote;
                    res = git_remote_lookup(remote, repo, remoteName.toUtf8().constData());
                    if (res)
                    {
                        throw QGitError("git_remote_lookup", res);
                    }
                    
                    GitStrArray refspecs;
                    refspecs.value.count = 1;
                    refspecs.value.strings = (char **)malloc(sizeof(char *) * 1);
                    QByteArray refspec = ":refs/heads/" + branchName.toUtf8();
                    refspecs.value.strings[0] = strdup(refspec.constData());
                    
                    git_push_options push_opts = GIT_PUSH_OPTIONS_INIT;
                    push_opts.callbacks.payload = this;
                    push_opts.callbacks.push_transfer_progress = [](unsigned int current, unsigned int total, size_t bytes, void *payload)->int
                    {
                        QGit *_this = static_cast<QGit *>(payload);
                        emit _this->pushProgress(current, total, bytes);
                        return 0;
                    };
                    
                    push_opts.callbacks.credentials = sshKeyCredentialCallback;
                    
                    res = git_remote_push(remote, &refspecs.value, &push_opts);
                    if (res)
                    {
                        throw QGitError("git_remote_push", res);
                    }
                }
            }
        }
    } catch(const QGitError & ex) {
        error = ex;
    }

    emit deleteBranchesReply(error);
}

void QGit::init()
{
    QGitError error;

    try
    {
        GitRepository repo;
        int res = git_repository_init(repo, m_path.absolutePath().toUtf8().constData(), 0);
        if(res)
        {
            throw QGitError("git_repository_init", res);
        }

    } catch(const QGitError & ex) {
        error = ex;
    }

    emit initReply(error);
}

void QGit::signature()
{
    QGitError error;
    QString email;
    QString name;    

    try
    {
        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        GitSignature me;
        res = git_signature_default(me, repo);
        if (res)
        {
            throw QGitError("git_signature_default", res);
        }

        name = QString::fromUtf8(me.value->name);
        email = QString::fromUtf8(me.value->email);

    } catch(const QGitError & ex) {
        error = ex;
    }

    emit signatureReply(name, email, error);
}

void QGit::status()
{
    QMap<git_status_t, int> items;
    QGitError error;

    try
    {
        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        GitStatusList list;
        res = git_status_list_new(list, repo, nullptr);
        if (res)
        {
            throw QGitError("git_status_list_new", res);
        }

        size_t index = 0;
        while(const git_status_entry *item = git_status_byindex(list, index))
        {
            items[item->status]++;
            index++;
        }

    } catch(const QGitError & ex) {
        error = ex;
    }

    emit statusReply(items, error);
}

void QGit::listBranchesAndTags()
{
    QList<QGitBranch> branches;
    QList<QGitTag> tags;
    QGitError error;

    try
    {
        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        GitBranchIterator it;
        res = git_branch_iterator_new(it, repo, GIT_BRANCH_ALL);
        if (res)
        {
            throw QGitError("git_branch_iterator_new", res);
        }

        while(1)
        {
            git_branch_t type = GIT_BRANCH_ALL;

            GitReference ref;
            if (git_branch_next(ref, &type, it) != 0)
            {
                break;
            }

            if (git_reference_type(ref) == GIT_REFERENCE_DIRECT)
            {
                const char *ref_name = git_reference_name(ref);

                GitObject obj;
                res = git_reference_peel(obj, ref, GIT_OBJ_COMMIT);
                if (res)
                {
                    throw QGitError("git_reference_peel", res);
                }

                git_time_t commit_time = git_commit_time(reinterpret_cast<const git_commit *>(obj.value));
                char oid_str[GIT_OID_HEXSZ + 1];
                git_oid_tostr(oid_str, sizeof(oid_str), git_object_id(obj));

                int ahead = 0;
                int behind = 0;

                if (type == GIT_BRANCH_LOCAL)
                {
                    GitReference upstream_ref;
                    int upstream_res = git_branch_upstream(upstream_ref, ref);
                    if (upstream_res == 0)
                    {
                        const git_oid *local_oid = git_reference_target(ref);
                        const git_oid *upstream_oid = git_reference_target(upstream_ref);
                        if (local_oid && upstream_oid)
                        {
                            size_t size_ahead = 0;
                            size_t size_behind = 0;
                            if (git_graph_ahead_behind(&size_ahead, &size_behind, repo, local_oid, upstream_oid) == 0)
                            {
                                ahead = static_cast<int>(size_ahead);
                                behind = static_cast<int>(size_behind);
                            }
                        }
                    }
                }

                QGitBranch branch = QGitBranch(ref_name, QString::fromLatin1(oid_str), commit_time, type, ahead, behind);
                branches.append(branch);
            }
        }

        GitStrArray tag_names;
        res = git_tag_list(tag_names, repo);
        if (res)
        {
            throw QGitError("git_tag_list", res);
        }

        for(size_t c = 0; c < tag_names.value.count; c++)
        {
            const char *tag_name = tag_names.value.strings[c];
            git_time_t tag_time = 0;
            GitReference tag_ref;
            GitObject git_obj;
            GitTag tag_obj;

            QByteArray tagFullName = QByteArray("refs/tags/") + tag_name;

            res = git_reference_lookup(tag_ref, repo, tagFullName.constData());
            if (res)
            {
                throw QGitError("git_reference_lookup", res);
            }

            //res = git_tag_lookup(tag_obj, repo, git_reference_target(tag_ref));
            res = git_reference_peel(git_obj, tag_ref, GIT_OBJ_COMMIT);
            if (res)
            {
                throw QGitError("git_reference_peel", res);
            }

            tag_time = git_commit_time(reinterpret_cast<const git_commit *>(git_obj.value));
            char oid_str[GIT_OID_HEXSZ + 1];
            git_oid_tostr(oid_str, sizeof(oid_str), git_object_id(git_obj));

            QGitTag tag(QString::fromUtf8(tag_name), QString::fromLatin1(oid_str), tag_time);
            tags.append(tag);
        }

    } catch(const QGitError &ex) {
        error = ex;
    }

    emit listBranchesAndTagsReply(branches, tags, error);
}

void QGit::stashSave(QString name, bool keepIndex, bool includeUntracked, bool includeIgnored)
{

    QGitError error;

    try
    {
        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        GitSignature me;
        res = git_signature_default(me, repo);
        if (res)
        {
            throw QGitError("git_signature_default", res);
        }

        uint32_t flags = GIT_STASH_DEFAULT;
        if (keepIndex) flags |= GIT_STASH_KEEP_INDEX;
        if (includeUntracked) flags |= GIT_STASH_INCLUDE_UNTRACKED;
        if (includeIgnored) flags |= GIT_STASH_INCLUDE_IGNORED;

        git_oid stashid;
        res = git_stash_save(&stashid, repo, me, name.toUtf8().constData(), flags);
        if (res)
        {
            throw QGitError("git_stash_save", res);
        }

    } catch(const QGitError &ex) {
        error = ex;
    }

    emit stashSaveReply(error);
}

void QGit::stashRemove(QString name)
{
    QGitError error;

    try
    {
        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        struct StashSearchContext {
            QString targetName;
            size_t foundIndex = 0;
            bool found = false;
        } ctx;
        ctx.targetName = name;

        res = git_stash_foreach(repo, [](size_t index, const char *message, const git_oid *stash_id, void *payload) -> int {
                Q_UNUSED(stash_id)

                auto ctx = static_cast<StashSearchContext *>(payload);

                if (QString::fromUtf8(message) == ctx->targetName)
                {
                    ctx->foundIndex = index;
                    ctx->found = true;
                    return 1; // Stop iterating.
                }

                return 0;
            }, &ctx);

        if (!ctx.found)
        {
            throw QGitError("stash not found", -1);
        }

        res = git_stash_drop(repo, ctx.foundIndex);
        if (res)
        {
            throw QGitError("git_stash_drop", res);
        }

    } catch(const QGitError &ex) {
        error = ex;
    }

    emit stashRemoveReply(error);
}

void QGit::listStashes()
{
    QStringList stashes;
    QGitError error;

    try
    {
        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        res = git_stash_foreach(repo, [](size_t index, const char* message, const git_oid *stash_id, void *payload) -> int {

                Q_UNUSED(index)
                Q_UNUSED(stash_id)

                auto list = static_cast<QStringList *>(payload);

                list->append(QString::fromUtf8(message));

                return 0;
            }, &stashes);
        if (res)
        {
            throw QGitError("git_stash_foreach", res);
        }

    } catch(const QGitError &ex) {
        error = ex;
    }

    emit listStashesReply(stashes, error);
}

void QGit::stashApply(QString name)
{
    QGitError error;

    try
    {
        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        struct StashSearchContext {
            QString targetName;
            size_t foundIndex = 0;
            bool found = false;
        } ctx;
        ctx.targetName = name;

        res = git_stash_foreach(repo, [](size_t index, const char *message, const git_oid *stash_id, void *payload) -> int {
            Q_UNUSED(stash_id)
            auto ctx = static_cast<StashSearchContext *>(payload);
            if (QString::fromUtf8(message) == ctx->targetName)
            {
                ctx->foundIndex = index;
                ctx->found = true;
                return 1;
            }
            return 0;
        }, &ctx);

        if (!ctx.found)
        {
            throw QGitError("Stash not found", -1);
        }

        git_stash_apply_options opts = GIT_STASH_APPLY_OPTIONS_INIT;
        res = git_stash_apply(repo, ctx.foundIndex, &opts);
        if (res)
        {
            throw QGitError("git_stash_apply", res);
        }

    } catch(const QGitError &ex) {
        error = ex;
    }

    emit stashApplyReply(error);
}

void QGit::stashPop(QString name)
{
    QGitError error;

    try
    {
        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        struct StashSearchContext {
            QString targetName;
            size_t foundIndex = 0;
            bool found = false;
        } ctx;
        ctx.targetName = name;

        res = git_stash_foreach(repo, [](size_t index, const char *message, const git_oid *stash_id, void *payload) -> int {
            Q_UNUSED(stash_id)
            auto ctx = static_cast<StashSearchContext *>(payload);
            if (QString::fromUtf8(message) == ctx->targetName)
            {
                ctx->foundIndex = index;
                ctx->found = true;
                return 1;
            }
            return 0;
        }, &ctx);

        if (!ctx.found)
        {
            throw QGitError("Stash not found", -1);
        }

        git_stash_apply_options opts = GIT_STASH_APPLY_OPTIONS_INIT;
        res = git_stash_pop(repo, ctx.foundIndex, &opts);
        if (res)
        {
            throw QGitError("git_stash_pop", res);
        }

    } catch(const QGitError &ex) {
        error = ex;
    }

    emit stashPopReply(error);
}

void QGit::listChangedFiles(int show, int sort, bool reversed)
{
    QList<QPair<QString, git_status_t>> items;
    QGitError error;

    try
    {
        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        git_status_options opts = {0, GIT_STATUS_SHOW_INDEX_AND_WORKDIR, 0, {}, nullptr, 0};
        opts.version = GIT_STATUS_OPTIONS_VERSION;
        opts.show = GIT_STATUS_SHOW_INDEX_AND_WORKDIR;
        opts.flags = GIT_STATUS_OPT_RENAMES_HEAD_TO_INDEX | GIT_STATUS_OPT_RENAMES_INDEX_TO_WORKDIR | GIT_STATUS_OPT_RECURSE_UNTRACKED_DIRS;

        if ((show == QGIT_STATUS_NONE)||(show & QGIT_STATUS_NEW))
            opts.flags |= GIT_STATUS_OPT_INCLUDE_UNTRACKED;

        if ((show == QGIT_STATUS_NONE)||(show & QGIT_STATUS_IGNORED))
            opts.flags |= GIT_STATUS_OPT_INCLUDE_IGNORED;

        if ((show == QGIT_STATUS_NONE)||(show == QGIT_STATUS_ALL))
            opts.flags |= GIT_STATUS_OPT_INCLUDE_UNMODIFIED | GIT_STATUS_OPT_INCLUDE_UNREADABLE;

        GitStatusList list;
        res = git_status_list_new(list, repo, &opts);
        if (res)
        {
            throw QGitError("git_status_list_new", res);
        }

        size_t index = 0;
        while(const git_status_entry *item = git_status_byindex(list, index))
        {
            git_status_t status = item->status;

            if ((item->index_to_workdir == nullptr)&&(item->head_to_index == nullptr))
            {
                throw QGitError("unknown scenario", res);
            }

            QString path1;
            if (
                    (item->index_to_workdir)
                    &&
                    (
                        ((show == QGIT_STATUS_NONE)&&(status == GIT_STATUS_CURRENT))
                        ||
                        ((show != QGIT_STATUS_NONE)&&(show & QGIT_STATUS_NEW)       &&(status != GIT_STATUS_CURRENT)&&(status & GIT_STATUS_WT_NEW))
                        ||
                        ((show != QGIT_STATUS_NONE)&&(show & QGIT_STATUS_MODIFIED)  &&(status != GIT_STATUS_CURRENT)&&(status & GIT_STATUS_WT_MODIFIED))
                        ||
                        ((show != QGIT_STATUS_NONE)&&(show & QGIT_STATUS_DELETED)   &&(status != GIT_STATUS_CURRENT)&&(status & GIT_STATUS_WT_DELETED))
                        ||
                        ((show != QGIT_STATUS_NONE)&&(show & QGIT_STATUS_RENAMED)   &&(status != GIT_STATUS_CURRENT)&&(status & GIT_STATUS_WT_RENAMED))
                        ||
                        ((show != QGIT_STATUS_NONE)&&(show & QGIT_STATUS_TYPECHANGE)&&(status != GIT_STATUS_CURRENT)&&(status & GIT_STATUS_WT_TYPECHANGE))
                        ||
                        ((show != QGIT_STATUS_NONE)&&(show & QGIT_STATUS_IGNORED)   &&(status != GIT_STATUS_CURRENT)&&(status & GIT_STATUS_IGNORED))
                        ||
                        ((show != QGIT_STATUS_NONE)&&(show & QGIT_STATUS_CONFLICTED)&&(status != GIT_STATUS_CURRENT)&&(status & GIT_STATUS_CONFLICTED))
                        ||
                        ((show == QGIT_STATUS_ALL))
                    )
                )
            {
                path1 = QString::fromUtf8(item->index_to_workdir->new_file.path);
                items.append({path1, status});
            }

            if ((item->head_to_index)&&(item->head_to_index->status != GIT_DELTA_UNMODIFIED)&&(status))
            {
                QString path2 = QString::fromUtf8(item->head_to_index->new_file.path);
                if (path1 != path2) {
                    items.append({path2, status});
                }
            }

            index++;
        }

        switch (sort)
        {
        case QGIT_SORT_FILEPATH:
            std::sort(items.begin(), items.end(), [](const auto &a, const auto &b) {
                return a.first < b.first;
            });
            break;
        case QGIT_SORT_FILENAME:
            std::sort(items.begin(), items.end(), [](const auto &a, const auto &b) {
                return QFileInfo(a.first).fileName() < QFileInfo(b.first).fileName();
            });
            break;
        case QGIT_SORT_STATUS:
            std::sort(items.begin(), items.end(), [](const auto &a, const auto &b) {
                return a.second < b.second;
            });
            break;
        default:
            break;
        }

        if (reversed)
        {
            std::reverse(items.begin(), items.end());
        }

    } catch(const QGitError &ex) {
        error = ex;
    }

    emit listChangedFilesReply(items, error);
}

void QGit::commitDiff(QString commitId, bool ignoreWhitespace)
{
    QList<QGitCommitDiffParent> parents;
    QGitSignature commitAuthor, commitCommiter;
    QDateTime commitTime;
    QString commitMessage;
    QString commitDescription;
    QString commitNote;
    QGitCommit commitDiff;
    QGitError error;

    try
    {
        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        if (commitId.isEmpty())
        {
            commitId = QStringLiteral("HEAD");
        }

        GitObject obj;
        res = git_revparse_single(obj, repo, commitId.toUtf8());
        if (res)
        {
            throw QGitError("git_revparse_single", res);
        }

        GitCommit commit;
        res = git_commit_lookup(commit, repo, git_object_id(obj));
        if (res)
        {
            throw QGitError("git_commit_lookup", res);
        }

        GitTree commit_tree;
        res = git_commit_tree(commit_tree, commit);
        if (res)
        {
            throw QGitError("git_commit_tree", res);
        }

        QString authorName = QString::fromUtf8(git_commit_author(commit)->name);
        QString authorEmail = QString::fromUtf8(git_commit_author(commit)->email);
        resolveMailmap(repo, authorName, authorEmail);
        commitAuthor = QGitSignature(authorName, authorEmail, QDateTime::fromMSecsSinceEpoch(git_commit_author(commit)->when.time * 1000));

        QString committerName = QString::fromUtf8(git_commit_committer(commit)->name);
        QString committerEmail = QString::fromUtf8(git_commit_committer(commit)->email);
        resolveMailmap(repo, committerName, committerEmail);
        commitCommiter = QGitSignature(committerName, committerEmail, QDateTime::fromMSecsSinceEpoch(git_commit_committer(commit)->when.time * 1000));

        auto time = git_commit_time(commit);
        auto timeOffset = git_commit_time_offset(commit);
        commitTime = QDateTime::fromMSecsSinceEpoch(time * 1000);
        commitTime.setTimeZone(QTimeZone(timeOffset * 60));

        commitMessage = QString::fromUtf8(git_commit_message(commit));

        {
            git_note *note = nullptr;
            if (git_note_read(&note, repo, nullptr, git_object_id(obj)) == 0)
            {
                commitNote = QString::fromUtf8(git_note_message(note));
                git_note_free(note);
            }
        }

        // Attempt git describe (best-effort: silently skip if no tags found)
        {
            git_describe_options descOpts = GIT_DESCRIBE_OPTIONS_INIT;
            descOpts.max_candidates_tags = 10;
            GitDescribeResult descResult;
            if (git_describe_commit(descResult, (git_object *)commit.value, &descOpts) == 0) {
                git_describe_format_options fmtOpts = GIT_DESCRIBE_FORMAT_OPTIONS_INIT;
                char buf[256] = {0};
                git_buf gbuf = GIT_BUF_INIT_CONST(buf, sizeof(buf));
                if (git_describe_format(&gbuf, descResult, &fmtOpts) == 0) {
                    commitDescription = QString::fromUtf8(gbuf.ptr);
                }
                git_buf_dispose(&gbuf);
            }
        }

        unsigned int parentCount = git_commit_parentcount(commit);
        if (parentCount == 0)
        {
            QGitCommitDiffParent item;

            git_diff_options options;
            memset(&options, 0, sizeof(options));
            options.version = GIT_DIFF_OPTIONS_VERSION;
            if (ignoreWhitespace) options.flags |= GIT_DIFF_IGNORE_WHITESPACE;

            GitDiff diff;
            res = git_diff_tree_to_tree(diff, repo, nullptr, commit_tree, &options);
            if (res)
            {
                throw QGitError("git_diff_tree_to_tree", res);
            }

            size_t _count = git_diff_num_deltas(diff);
            for(size_t c = 0; c < _count; c++)
            {
                const git_diff_delta *delta = nullptr;
                delta = git_diff_get_delta(diff, c);
                item.addFile(delta);
            }

            parents.append(item);
        }
        else
        {
            for(unsigned int c = 0; c < parentCount; c++)
            {
                GitCommit parent;
                res = git_commit_parent(parent, commit, c);
                if (res)
                {
                    throw QGitError("git_commit_parent", res);
                }

                char commit_id[GIT_OID_HEXSZ + 1] = {0, };
                const git_oid *oid = git_commit_id(parent);
                git_oid_tostr(commit_id, sizeof(commit_id), oid);
                QGitCommitDiffParent item(commit_id);

                GitTree parent_tree;
                res = git_commit_tree(parent_tree, parent);
                if (res)
                {
                    throw QGitError("git_commit_tree", res);
                }

                git_diff_options options;
                memset(&options, 0, sizeof(options));
                options.version = GIT_DIFF_OPTIONS_VERSION;
                if (ignoreWhitespace) options.flags |= GIT_DIFF_IGNORE_WHITESPACE;

                GitDiff diff;
                res = git_diff_tree_to_tree(diff, repo, parent_tree, commit_tree, &options);
                if (res)
                {
                    throw QGitError("git_diff_tree_to_tree", res);
                }

                size_t _count = git_diff_num_deltas(diff);
                for(size_t c2 = 0; c2 < _count; c2++)
                {
                    const git_diff_delta *delta = nullptr;
                    delta = git_diff_get_delta(diff, c2);
                    item.addFile(delta);
                }

                parents.append(item);
            }
        }
    } catch(const QGitError &ex) {
        error = ex;
    }

    commitDiff = QGitCommit(commitId, parents, commitTime, commitAuthor, commitCommiter, commitMessage, commitDescription, commitNote);

    emit commitDiffReply(commitId, commitDiff, error);
}

void QGit::commitDiffContent(QString first, QString second, QList<QString> files, uint32_t context_lines, bool ignoreWhitespace)
{
    QList<QGitDiffFile> items;
    QGitError error;

    try
    {
        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        GitStrArray pathspec;
        if (!files.isEmpty())
        {
            pathspec.value.count = static_cast<size_t>(files.count());
            pathspec.value.strings = static_cast<char **>(malloc(sizeof(char *) * pathspec.value.count));
            if (pathspec.value.strings == nullptr)
            {
                throw QGitError("malloc", 0);
            }

            int c = 0;
            for(const auto &file: files)
            {
                pathspec.value.strings[c] = strdup(file.toUtf8().constData());
                c++;
            }
        }

        GitDiff diff;
        if (second == "staged")
        {
            GitObject first_obj;
            res = git_revparse_single(first_obj, repo, "HEAD^{tree}");
            if (res)
            {
                throw QGitError("git_revparse_single(staged)", res);
            }

            GitTree first_tree;
            res = git_tree_lookup(first_tree, repo, git_object_id(first_obj));
            if (res)
            {
                throw QGitError("git_tree_lookup(staged)", res);
            }

            git_diff_options options;
            memset(&options, 0, sizeof(options));
            options.version = GIT_DIFF_OPTIONS_VERSION;
            options.flags = GIT_DIFF_INCLUDE_UNTRACKED | GIT_DIFF_SHOW_UNTRACKED_CONTENT;
            if (ignoreWhitespace) options.flags |= GIT_DIFF_IGNORE_WHITESPACE;
            options.context_lines = context_lines;
            options.pathspec = pathspec;

            res = git_diff_tree_to_index(diff, repo, first_tree, nullptr, &options);
            if (res)
            {
                throw QGitError("git_diff_tree_to_index(staged)", res);
            }
        }
        else if (second == "unstaged")
        {
            git_diff_options options;

            memset(&options, 0, sizeof(options));
            options.version = GIT_DIFF_OPTIONS_VERSION;
            options.flags = GIT_DIFF_INCLUDE_UNTRACKED | GIT_DIFF_RECURSE_UNTRACKED_DIRS | GIT_DIFF_SHOW_UNTRACKED_CONTENT;
            if (ignoreWhitespace) options.flags |= GIT_DIFF_IGNORE_WHITESPACE;
            options.context_lines = context_lines;
            options.pathspec = pathspec;

            res = git_diff_index_to_workdir(diff, repo, nullptr, &options);
            if (res)
            {
                throw QGitError("git_diff_index_to_workdir", res);
            }
        }
        else
        {
            GitObject first_obj;
            res = git_revparse_single(first_obj, repo, first.toUtf8());
            if (res)
            {
                throw QGitError("git_revparse_single(first)", res);
            }

            GitCommit first_commit;
            res = git_commit_lookup(first_commit, repo, git_object_id(first_obj));
            if (res)
            {
                throw QGitError("git_commit_lookup(first)", res);
            }

            GitTree first_tree;
            res = git_commit_tree(first_tree, first_commit);
            if (res)
            {
                throw QGitError("git_commit_tree(first)", res);
            }

            GitObject second_obj;
            res = git_revparse_single(second_obj, repo, second.toUtf8());
            if (res)
            {
                throw QGitError("git_revparse_single(second)", res);
            }

            GitCommit second_commit;
            res = git_commit_lookup(second_commit, repo, git_object_id(second_obj));
            if (res)
            {
                throw QGitError("git_commit_lookup(second)", res);
            }

            GitTree second_tree;
            res = git_commit_tree(second_tree, second_commit);
            if (res)
            {
                throw QGitError("git_commit_tree(second)", res);
            }

            res = git_diff_tree_to_tree(diff, repo, first_tree, second_tree, nullptr);
            if (res)
            {
                throw QGitError("git_diff_tree_to_tree", res);
            }
        }

        size_t _count = git_diff_num_deltas(diff);
        for(size_t c = 0; c < _count; c++)
        {
            const git_diff_delta *delta = git_diff_get_delta(diff, c);
            QGitDiffFile item(delta);

            if (files.contains(item.new_file().path()))
            {
                GitPatch patch;
                res = git_patch_from_diff(patch, diff, c);
                if (res)
                {
                    throw QGitError("git_patch_from_diff", res);
                }

                size_t hunks = git_patch_num_hunks(patch);
                for(size_t  hunk_cnt = 0; hunk_cnt < hunks; hunk_cnt++) {
                    const git_diff_hunk *hunk = nullptr;
                    res = git_patch_get_hunk(&hunk, nullptr, patch, hunk_cnt);
                    if (res)
                    {
                        throw QGitError("git_patch_get_hunk", res);
                    }

                    QGitDiffHunk hunkObj(hunk);
                    size_t lines = git_patch_num_lines_in_hunk(patch, hunk_cnt);
                    if (lines > 0)
                    {
                        for(int  line_cnt = 0; line_cnt < lines; line_cnt++) {
                            const git_diff_line *line = nullptr;
                            res = git_patch_get_line_in_hunk(&line, patch, hunk_cnt, line_cnt);
                            if (res)
                            {
                                throw QGitError("git_patch_get_line_in_hunk", res);
                            }

                            hunkObj.addLine(line);
                        }
                    }

                    item.addHunk(hunkObj);
                }

                items.append(item);
            }
        }
    } catch(const QGitError &ex) {
        error = ex;
    }

    emit commitDiffContentReply(first, second, items, error);
}

void QGit::stageFiles(QStringList items)
{
    QGitError error;

    try
    {
        if (items.count() == 0)
        {
            throw QGitError();
        }

        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        GitIndex index;
        res = git_repository_index(index, repo);
        if (res)
        {
            throw QGitError("git_repository_index", res);
        }

        for(const auto &item: items)
        {
            res = git_index_add_bypath(index, item.toUtf8().constData());
            if (res == GIT_ENOTFOUND) { // FIXME: Properly implement stage add/delete files.
                res = git_index_remove_bypath(index, item.toUtf8().constData());
                if (res)
                {
                    throw QGitError("git_index_remove_bypath", res);
                }
            }
            if (res)
            {
                throw QGitError("git_index_add_bypath", res);
            }
        }

        res = git_index_write(index);
        if (res)
        {
            throw QGitError("git_index_write", res);
        }

    } catch(const QGitError &ex) {
        error = ex;
    }

    emit stageFilesReply(error);
}

void QGit::unstageFiles(QStringList items)
{
    QGitError error;

    try
    {
        if (items.count() == 0)
        {
            throw QGitError();
        }

        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        GitStrArray paths;
        paths.value.count = static_cast<size_t>(items.count());
        paths.value.strings = static_cast<char **>(malloc(sizeof(char *) * paths.value.count));
        if (paths.value.strings == nullptr)
        {
            throw QGitError("malloc", 0);
        }

        for(int c = 0; c < items.count(); c++)
        {
            paths.value.strings[c] = strdup(items.at(c).toUtf8().constData());
        }

        GitReference head;
        GitObject head_commit;
        res = git_repository_head(head, repo);
        if (res == 0)
        {
            res = git_reference_peel(head_commit, head, GIT_OBJ_COMMIT);
            if (res)
            {
                throw QGitError("git_reference_peel", res);
            }
        }

        res = git_reset_default(repo, head_commit, &paths.value);
        if (res)
        {
            throw QGitError("git_reset_default", res);
        }

    } catch(const QGitError &ex) {
        error = ex;
    }

    emit unstageFilesReply(error);
}

void QGit::stageFileLines(QString filename, QVector<QGitDiffWidgetLine> lines)
{
    QGitError error;

    try
    {
        if (lines.count() == 0)
        {
            throw QGitError();
        }

        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        GitIndex index;
        res = git_repository_index(index, repo);
        if (res)
        {
            throw QGitError("git_repository_index", res);
        }

        const git_index_entry *entry = git_index_get_bypath(index, filename.toUtf8().constData(), GIT_INDEX_STAGE_NORMAL);
        if (entry == nullptr)
        {
            throw QGitError("git_index_get_bypath", res);
        }

        GitBlob blob;
        res = git_blob_lookup(blob, repo, &entry->id);
        if (res)
        {
            throw QGitError("git_blob_lookup", res);
        }

        const char *blob_content = static_cast<const char *>(git_blob_rawcontent(blob));
        git_off_t blob_size = git_blob_rawsize(blob);
        QByteArray buffer = QByteArray(blob_content, blob_size);
        auto bufferLines = buffer.split(LINE_END);
        int added = 0;

        for(const auto &line: lines)
        {
            QByteArray content;

            switch (line.origin)
            {
            case ' ':
            case '=':
            case '>':
            case '<':
                continue;
            case '-':
                {
                    int pos = line.old_lineno + added - 1;
                    bool wasLast = (pos == bufferLines.size() - 1);
                    bufferLines.removeAt(pos);
                    if (wasLast && !bufferLines.isEmpty()) {
                        bufferLines.append("");
                    }
                    added--;
                }
                break;
            case '+':
                {
                    content = line.content;
                    bool endsWithNL = content.endsWith(LINE_END);
                    if (endsWithNL) content.chop(1);

                    int pos = line.old_lineno + added - 1;
                    bufferLines.insert(pos, content);
                    if (endsWithNL)
                    {
                        if (pos == bufferLines.size() - 1)
                        {
                            bufferLines.append("");
                        }
                    }
                    else
                    {
                        if (pos == bufferLines.size() - 2 && bufferLines.last() == "")
                        {
                            bufferLines.removeLast();
                        }
                    }
                    added++;
                }
                break;
            default:
                throw QGitError("Unknown operation", 0);
            }
        }

        buffer = bufferLines.join(LINE_END);

        res = git_index_add_from_buffer(index, entry, buffer.constData(), buffer.length());
        if (res)
        {
            throw QGitError("git_index_add_frombuffer", res);
        }

        res = git_index_write(index);
        if (res)
        {
            throw QGitError("git_index_write", res);
        }
    } catch(const QGitError &ex) {
        error = ex;
    }

    emit stageFilesReply(error);
}

void QGit::unstageFileLines(QString filename, QVector<QGitDiffWidgetLine> lines)
{
    QGitError error;

    try
    {
        if (lines.count() == 0)
        {
            throw QGitError();
        }

        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        GitIndex index;
        res = git_repository_index(index, repo);
        if (res)
        {
            throw QGitError("git_repository_index", res);
        }

        const git_index_entry *entry = git_index_get_bypath(index, filename.toUtf8().constData(), GIT_INDEX_STAGE_NORMAL);
        if (entry == nullptr)
        {
            throw QGitError("git_index_get_bypath", res);
        }

        GitBlob blob;
        res = git_blob_lookup(blob, repo, &entry->id);
        if (res)
        {
            throw QGitError("git_blob_lookup", res);
        }

        const char *blob_content = static_cast<const char *>(git_blob_rawcontent(blob));
        git_off_t blob_size = git_blob_rawsize(blob);
        QByteArray buffer = QByteArray(blob_content, blob_size);
        auto bufferLines = buffer.split(LINE_END);
        int added = 0;

        for(const auto &line: lines)
        {
            QByteArray content;

            switch(line.origin)
            {
            case ' ':
            case '=':
            case '>':
            case '<':
                continue;
            case '+':
                {
                    int pos = line.new_lineno + added - 1;
                    bool wasLast = (pos == bufferLines.size() - 1);
                    bufferLines.removeAt(pos);
                    if (wasLast && !bufferLines.isEmpty()) {
                        bufferLines.append("");
                    }
                    added--;
                }
                break;
            case '-':
                {
                    content = line.content;
                    bool endsWithNL = content.endsWith(LINE_END);
                    if (endsWithNL) content.chop(1);

                    int pos = line.new_lineno + added - 1;
                    bufferLines.insert(pos, content);
                    if (endsWithNL)
                    {
                        if (pos == bufferLines.size() - 1)
                        {
                            bufferLines.append("");
                        }
                    }
                    else
                    {
                        if (pos == bufferLines.size() - 2 && bufferLines.last() == "")
                        {
                            bufferLines.removeLast();
                        }
                    }
                    added++;
                }
                break;
            default:
                throw QGitError("Unknown operation", 0);
            }
        }
        buffer = bufferLines.join(LINE_END);

        res = git_index_add_from_buffer(index, entry, buffer.constData(), buffer.length());
        if (res)
        {
            throw QGitError("git_index_add_frombuffer", res);
        }

        res = git_index_write(index);
        if (res)
        {
            throw QGitError("git_index_write", res);
        }
    } catch(const QGitError &ex) {
        error = ex;
    }

    emit unstageFilesReply(error);
}

void QGit::discardFiles(QStringList items)
{
    QGitError error;

    try
    {
        if (items.count() == 0)
        {
            throw QGitError();
        }

        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        GitIndex index;
        res = git_repository_index(index, repo);
        if (res)
        {
            throw QGitError("git_repository_index", res);
        }

        QStringList filesToCheckout;
        bool indexChanged = false;

        for (const QString &item : items)
        {
            unsigned int status_flags = 0;
            int status_res = git_status_file(&status_flags, repo, item.toUtf8().constData());

            if (status_res == 0 && ((status_flags & GIT_STATUS_WT_NEW) || (status_flags & GIT_STATUS_INDEX_NEW)))
            {
                QString absolutePath = m_path.absoluteFilePath(item);
                QFile::remove(absolutePath);

                if (status_flags & GIT_STATUS_INDEX_NEW)
                {
                    res = git_index_remove_bypath(index, item.toUtf8().constData());
                    if (res == 0)
                    {
                        indexChanged = true;
                    }
                }
            }
            else
            {
                filesToCheckout.append(item);
            }
        }

        if (indexChanged)
        {
            res = git_index_write(index);
            if (res)
            {
                throw QGitError("git_index_write", res);
            }
        }

        if (!filesToCheckout.isEmpty())
        {
            GitStrArray paths;
            paths.value.count = static_cast<size_t>(filesToCheckout.count());
            paths.value.strings = static_cast<char **>(malloc(sizeof(char *) * paths.value.count));
            if (paths.value.strings == nullptr)
            {
                throw QGitError("malloc", 0);
            }

            for(int c = 0; c < filesToCheckout.count(); c++)
            {
                paths.value.strings[c] = strdup(filesToCheckout.at(c).toUtf8().constData());
            }

            git_checkout_options opts = GIT_CHECKOUT_OPTIONS_INIT;
            opts.checkout_strategy = GIT_CHECKOUT_FORCE;
            opts.paths = paths.value;

            res = git_checkout_index(repo, nullptr, &opts);
            if (res)
            {
                throw QGitError("git_checkout_index", res);
            }
        }

    } catch(const QGitError &ex) {
        error = ex;
    }

    emit discardFilesReply(error);
}

void QGit::discardFileLines(QString filename, QVector<QGitDiffWidgetLine> lines)
{
    QGitError error;

    try
    {
        if (lines.count() == 0)
        {
            throw QGitError();
        }

        QString filePath = m_path.absoluteFilePath(filename);
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly))
        {
            throw QGitError("QFile::open(ReadOnly)", 0);
        }

        QByteArray buffer = file.readAll();
        file.close();

        auto bufferLines = buffer.split(LINE_END);
        int added = 0;

        for (const auto &line: lines)
        {
            QByteArray content;

            switch(line.origin)
            {
            case ' ':
            case '=':
            case '>':
            case '<':
                continue;
            case '+':
                {
                    int pos = line.new_lineno + added - 1;
                    bool wasLast = (pos == bufferLines.size() - 1);
                    bufferLines.removeAt(pos);
                    if (wasLast && !bufferLines.isEmpty()) {
                        bufferLines.append("");
                    }
                    added--;
                }
                break;
            case '-':
                {
                    content = line.content;
                    bool endsWithNL = content.endsWith(LINE_END);
                    if (endsWithNL) content.chop(1);

                    int pos = line.new_lineno + added - 1;
                    bufferLines.insert(pos, content);
                    if (endsWithNL)
                    {
                        if (pos == bufferLines.size() - 1)
                        {
                            bufferLines.append("");
                        }
                    }
                    else
                    {
                        if (pos == bufferLines.size() - 2 && bufferLines.last() == "")
                        {
                            bufferLines.removeLast();
                        }
                    }
                    added++;
                }
                break;
            default:
                throw QGitError("Unknown operation", 0);
            }
        }

        buffer = bufferLines.join(LINE_END);

        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        {
            throw QGitError("QFile::open(WriteOnly)", 0);
        }

        file.write(buffer);
        file.close();

    } catch(const QGitError &ex) {
        error = ex;
    }

    emit discardFilesReply(error);
}

void QGit::commit(QString message, bool withPush, bool amend)
{
    git_oid new_commit_id = { {0} };
    QGitError error;

    try
    {
        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        /// Check if something is staged
        GitTree tree;
        GitObject git_obj;
        res = git_revparse_single(git_obj, repo, "HEAD^{tree}");
        if (res == 0)
        {
            res = git_tree_lookup(tree, repo, git_object_id(git_obj));
            if (res)
            {
                throw QGitError("git_tree_lookup(staged)", res);
            }
        }

        GitDiff diff;
        res = git_diff_tree_to_index(diff, repo, tree, nullptr, nullptr);
        if (res)
        {
            throw QGitError("git_diff_tree_to_index(staged)", res);
        }

        int _count = git_diff_num_deltas(diff);
        if (_count == 0 && !amend)
        {
            throw QGitError("Nothing staged.", res);
        }

        GitSignature me;
        res = git_signature_default(me, repo);
        if (res)
        {
            throw QGitError("git_signature_default", res);
        }

        GitIndex index;
        res = git_repository_index(index, repo);
        if (res)
        {
            throw QGitError("git_repository_index", res);
        }

        git_oid tree_id = { {0} };
        res = git_index_write_tree(&tree_id, index);
        if (res)
        {
            throw QGitError("git_index_write_tree", res);
        }

        res = git_tree_lookup(tree, repo, &tree_id);
        if (res)
        {
            throw QGitError("git_tree_lookup", res);
        }

        int unborn = git_repository_head_unborn(repo);
        if (unborn)
        {
            res = git_commit_create_v(
              &new_commit_id,
              repo,
              "HEAD",                        /* name of ref to update */
              me,                            /* author */
              me,                            /* committer */
              nullptr,                       /* nullptr = UTF-8 message encoding */
              message.toUtf8().constData(),  /* message */
              tree,                          /* root tree */
              0);                            /* parent count */
            if (res)
            {
                throw QGitError("git_commit_create_v", res);
            }
        }
        else
        {
            git_oid parent_id = { {0} };
            res = git_reference_name_to_id(&parent_id, repo, "HEAD");
            if (res)
            {
                throw QGitError("git_reference_name_to_id", res);
            }

            GitCommit parent;
            res = git_commit_lookup(parent, repo, &parent_id);
            if (res)
            {
                throw QGitError("git_commit_lookup", res);
            }

            if (amend)
            {
                res = git_commit_amend(
                  &new_commit_id,
                  parent,
                  "HEAD",                        /* name of ref to update */
                  nullptr,                       /* nullptr = keep original author */
                  me,                            /* committer */
                  nullptr,                       /* nullptr = UTF-8 message encoding */
                  message.toUtf8().constData(),  /* message */
                  tree);                         /* new tree */
                if (res)
                {
                    throw QGitError("git_commit_amend", res);
                }
            }
            else
            {
                GitCommit parent2;
                int parent_count = 1;
                const git_commit *commit_parents[2] = { parent.value, nullptr };

                if (git_repository_state(repo) == GIT_REPOSITORY_STATE_MERGE) {
                    git_oid merge_head_id;
                    if (git_reference_name_to_id(&merge_head_id, repo, "MERGE_HEAD") == 0) {
                        if (git_commit_lookup(parent2, repo, &merge_head_id) == 0) {
                            commit_parents[1] = parent2.value;
                            parent_count = 2;
                        }
                    }
                }

                res = git_commit_create(
                  &new_commit_id,
                  repo,
                  "HEAD",                        /* name of ref to update */
                  me,                            /* author */
                  me,                            /* committer */
                  nullptr,                       /* nullptr = UTF-8 message encoding */
                  message.toUtf8().constData(),  /* message */
                  tree,                          /* root tree */
                  parent_count,                  /* parent count */
                  commit_parents);               /* parents */
                if (res)
                {
                    throw QGitError("git_commit_create", res);
                }

                if (parent_count > 1 || 
                    git_repository_state(repo) == GIT_REPOSITORY_STATE_CHERRYPICK ||
                    git_repository_state(repo) == GIT_REPOSITORY_STATE_REVERT) {
                    git_repository_state_cleanup(repo);
                }
            }
        }

        if (withPush)
        {

        }

    } catch(const QGitError &ex) {
        error = ex;
    }

    emit commitReply(QString::fromUtf8(git_oid_tostr_s(&new_commit_id)), error);
}

void QGit::clone(QUrl url)
{
    QGitError error;

    try
    {
        git_clone_options opts;
        int res = git_clone_init_options(&opts, GIT_CLONE_OPTIONS_VERSION);
        if (res)
        {
            throw QGitError("git_clone_init_options", res);
        }

        opts.fetch_opts.callbacks.payload = this;

        opts.fetch_opts.callbacks.transfer_progress = [](const git_transfer_progress *stats, void *payload) -> int {

            auto _this = static_cast<QGit *>(payload);

            emit _this->cloneTransferReply(stats->total_objects, stats->indexed_objects, stats->received_objects, stats->local_objects, stats->total_deltas, stats->indexed_deltas, stats->received_bytes);

            return QThread::currentThread()->isInterruptionRequested();
        };

        opts.checkout_opts.progress_payload = this;

        opts.checkout_opts.progress_cb = [](
                const char *path,
                size_t completed_steps,
                size_t total_steps,
                void *payload)
        {
            auto _this = static_cast<QGit *>(payload);

            emit _this->cloneProgressReply(QString::fromUtf8(path), completed_steps, total_steps);

            _this = nullptr;
        };

        GitRepository repo;
        res = git_clone(repo, url.toString().toUtf8().constData(), m_path.absolutePath().toUtf8().constData(), &opts);
        if (res)
        {
            throw QGitError("git_clone", res);
        }

    } catch(const QGitError &ex) {
        error = ex;
    }

    emit cloneReply(error);
}

void QGit::pull(QString remote, QString branch, bool rebase)
{
    QGitError error;

    try
    {
        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        GitRemote libgit2_remote;
        bool is_anonymous = false;
        if (remote.contains(QStringLiteral("://")) || remote.contains(QStringLiteral("@")))
        {
            res = git_remote_create_anonymous(libgit2_remote, repo, remote.toUtf8().constData());
            if (res)
            {
                throw QGitError("git_remote_create_anonymous", res);
            }
            is_anonymous = true;
        }
        else
        {
            res = git_remote_lookup(libgit2_remote, repo, remote.toUtf8().constData());
            if (res)
            {
                throw QGitError("git_remote_lookup", res);
            }
        }

        git_fetch_options fetch_opts = GIT_FETCH_OPTIONS_INIT;
        fetch_opts.callbacks.credentials = sshKeyCredentialCallback;

        res = git_remote_fetch(libgit2_remote, nullptr, &fetch_opts, "pull");
        if (res)
        {
            throw QGitError("git_remote_fetch", res);
        }

        QString targetRef = is_anonymous ? QStringLiteral("FETCH_HEAD") : (remote + "/" + branch);

        if (rebase) {
            this->rebase(targetRef);
        } else {
            merge(targetRef);
        }

    } catch(const QGitError &ex) {
        error = ex;
    }

    emit pullReply(error);
}

void QGit::rebase(QString upstream, QString branch, QString onto)
{
    QGitError error;
    git_rebase *rebase_ptr = nullptr;

    try
    {
        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        GitAnnotatedCommit upstream_annotated;
        res = git_annotated_commit_from_revspec(upstream_annotated, repo, upstream.toUtf8().constData());
        if (res)
        {
            throw QGitError("git_annotated_commit_from_revspec", res);
        }

        GitAnnotatedCommit branch_annotated;
        const git_annotated_commit *branch_ptr = nullptr;
        if (!branch.isEmpty())
        {
            res = git_annotated_commit_from_revspec(branch_annotated, repo, branch.toUtf8().constData());
            if (res)
            {
                throw QGitError("git_annotated_commit_from_revspec", res);
            }
            branch_ptr = branch_annotated.value;
        }

        GitAnnotatedCommit onto_annotated;
        const git_annotated_commit *onto_ptr = nullptr;
        if (!onto.isEmpty())
        {
            res = git_annotated_commit_from_revspec(onto_annotated, repo, onto.toUtf8().constData());
            if (res)
            {
                throw QGitError("git_annotated_commit_from_revspec", res);
            }
            onto_ptr = onto_annotated.value;
        }

        git_rebase_options opts = GIT_REBASE_OPTIONS_INIT;
        res = git_rebase_init(&rebase_ptr, repo, branch_ptr, upstream_annotated.value, onto_ptr, &opts);
        if (res)
        {
            throw QGitError("git_rebase_init", res);
        }

        GitSignature committer;
        res = git_signature_default(committer, repo);
        if (res)
        {
            git_rebase_abort(rebase_ptr);
            git_rebase_free(rebase_ptr);
            rebase_ptr = nullptr;
            throw QGitError("git_signature_default", res);
        }

        git_rebase_operation *op = nullptr;
        bool has_conflict = false;

        while ((res = git_rebase_next(&op, rebase_ptr)) == 0)
        {
            git_oid commit_id;
            res = git_rebase_commit(&commit_id, rebase_ptr, nullptr, committer, nullptr, nullptr);
            if (res == GIT_EUNMERGED)
            {
                has_conflict = true;
                break;
            }
            else if (res < 0)
            {
                git_rebase_abort(rebase_ptr);
                git_rebase_free(rebase_ptr);
                rebase_ptr = nullptr;
                throw QGitError("git_rebase_commit", res);
            }
        }

        if (has_conflict)
        {
            git_rebase_free(rebase_ptr);
            rebase_ptr = nullptr;
            throw QGitError("Rebase conflicts detected. Please resolve conflicts or abort rebase.", -1);
        }

        if (res != GIT_ITEROVER && res != 0)
        {
            git_rebase_abort(rebase_ptr);
            git_rebase_free(rebase_ptr);
            rebase_ptr = nullptr;
            throw QGitError("git_rebase_next", res);
        }

        res = git_rebase_finish(rebase_ptr, committer);
        git_rebase_free(rebase_ptr);
        rebase_ptr = nullptr;
        if (res)
        {
            throw QGitError("git_rebase_finish", res);
        }
    }
    catch (const QGitError &ex)
    {
        error = ex;
    }

    emit rebaseReply(error);
}

void QGit::merge(QString branchName)
{
    QGitError error;
    try {
        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if(res) throw QGitError("git_repository_open", res);

        GitAnnotatedCommit annotated;
        res = git_annotated_commit_from_revspec(annotated, repo, branchName.toUtf8().constData());
        if(res) throw QGitError("git_annotated_commit_from_revspec", res);

        git_merge_analysis_t analysis;
        git_merge_preference_t preference;
        const git_annotated_commit *sources[] = { annotated.value };

        res = git_merge_analysis(&analysis, &preference, repo, sources, 1);
        if(res) throw QGitError("git_merge_analysis", res);

        if (analysis & GIT_MERGE_ANALYSIS_UP_TO_DATE) {
            // Already up to date
        } else if (analysis & GIT_MERGE_ANALYSIS_FASTFORWARD) {
            // Fast-forward
            GitObject target_obj;
            res = git_object_lookup(target_obj, repo, git_annotated_commit_id(annotated), GIT_OBJECT_COMMIT);
            if(res) throw QGitError("git_object_lookup", res);

            git_checkout_options opts = GIT_CHECKOUT_OPTIONS_INIT;
            opts.checkout_strategy = GIT_CHECKOUT_SAFE;
            res = git_checkout_tree(repo, target_obj, &opts);
            if(res) throw QGitError("git_checkout_tree", res);

            GitReference head_ref;
            res = git_repository_head(head_ref, repo);
            if(res) throw QGitError("git_repository_head", res);

            GitReference new_ref;
            res = git_reference_set_target(new_ref, head_ref, git_object_id(target_obj), "Fast-forward merge");
            if(res) throw QGitError("git_reference_set_target", res);

        } else if (analysis & GIT_MERGE_ANALYSIS_NORMAL) {
            // Normal merge
            git_merge_options merge_opts = GIT_MERGE_OPTIONS_INIT;
            git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;
            checkout_opts.checkout_strategy = GIT_CHECKOUT_SAFE;

            res = git_merge(repo, sources, 1, &merge_opts, &checkout_opts);
            if(res) throw QGitError("git_merge", res);

            // Check for conflicts
            GitIndex index;
            res = git_repository_index(index, repo);
            if(res) throw QGitError("git_repository_index", res);

            if (git_index_has_conflicts(index)) {
                throw QGitError("Merge conflicts detected. Please resolve them manually.", -1);
            }

            // Create merge commit
            git_oid tree_id;
            res = git_index_write_tree(&tree_id, index);
            if (res) throw QGitError("git_index_write_tree", res);

            GitTree tree;
            res = git_tree_lookup(tree, repo, &tree_id);
            if (res) throw QGitError("git_tree_lookup", res);

            GitSignature sig;
            res = git_signature_default(sig, repo);
            if (res) throw QGitError("git_signature_default", res);

            GitReference head_ref;
            res = git_repository_head(head_ref, repo);
            if (res) throw QGitError("git_repository_head", res);

            GitCommit parent1; // HEAD
            res = git_commit_lookup(parent1, repo, git_reference_target(head_ref));
            if (res) throw QGitError("git_commit_lookup (HEAD)", res);

            GitCommit parent2; // MERGED branch
            res = git_commit_lookup(parent2, repo, git_annotated_commit_id(annotated));
            if (res) throw QGitError("git_commit_lookup (MERGE)", res);

            QString msg = QString("Merge branch '%1'").arg(branchName);

            git_oid new_commit_id;
            const git_commit *parents[] = { parent1.value, parent2.value };
            res = git_commit_create(
                &new_commit_id,
                repo,
                "HEAD",
                sig,
                sig,
                nullptr,
                msg.toUtf8().constData(),
                tree,
                2,
                parents
            );
            if (res) throw QGitError("git_commit_create", res);

            git_repository_state_cleanup(repo);
        }

    } catch(const QGitError &ex) {
        error = ex;
    }
    emit mergeReply(error);
}

void QGit::fetch(bool fetchFromAllRemotes, bool purgeDeletedBranches, bool fetchAllTags)
{
    QGitError error;

    try
    {
        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        GitStrArray remotes;
        res = git_remote_list(remotes, repo);
        if (res)
        {
            throw QGitError("git_remote_list", res);
        }

        QVector<QByteArray> remotesToFetch;

        if (fetchFromAllRemotes)
        {
            remotesToFetch.reserve(static_cast<int>(remotes.value.count));
            for(size_t idx = 0; idx < remotes.value.count; idx++)
            {
                remotesToFetch.append(QByteArray(remotes.value.strings[idx]));
            }
        }
        else
        {
            QByteArray selected;
            for(size_t idx = 0; idx < remotes.value.count; idx++)
            {
                if (QString::fromUtf8(remotes.value.strings[idx]) == QStringLiteral("origin"))
                {
                    selected = QByteArray(remotes.value.strings[idx]);
                    break;
                }
            }

            if ((selected.isEmpty())&&(remotes.value.count > 0))
            {
                selected = QByteArray(remotes.value.strings[0]);
            }

            if (!selected.isEmpty())
            {
                remotesToFetch.append(selected);
            }
        }

        if (remotesToFetch.isEmpty())
        {
            throw QGitError("git_remote_lookup", GIT_ENOTFOUND);
        }

        git_fetch_options fetch_opts = GIT_FETCH_OPTIONS_INIT;

        fetch_opts.callbacks.credentials = sshKeyCredentialCallback;

        fetch_opts.prune = purgeDeletedBranches ? GIT_FETCH_PRUNE : GIT_FETCH_NO_PRUNE;
        fetch_opts.download_tags = fetchAllTags ? GIT_REMOTE_DOWNLOAD_TAGS_ALL : GIT_REMOTE_DOWNLOAD_TAGS_AUTO;

        for(const auto &remoteName: remotesToFetch)
        {
            GitRemote remote;
            res = git_remote_lookup(remote, repo, remoteName.constData());
            if (res)
            {
                throw QGitError("git_remote_lookup", res);
            }

            res = git_remote_fetch(remote, nullptr, &fetch_opts, "fetch");
            if (res)
            {
                throw QGitError("git_remote_fetch", res);
            }
        }

    } catch(const QGitError &ex) {
        fprintf(stderr, "error doing git fetch!\n");
        error = ex;
    }

    emit fetchReply(error);
}

void QGit::push(QString remote, QStringList branches, bool tags, bool force)
{
    QGitError error;

    try
    {
        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        GitRemote libgit2_remote;
        if (remote.contains(QStringLiteral("://")) || remote.contains(QStringLiteral("@")))
        {
            res = git_remote_create_anonymous(libgit2_remote, repo, remote.toUtf8().constData());
            if (res)
            {
                throw QGitError("git_remote_create_anonymous", res);
            }
        }
        else
        {
            res = git_remote_lookup(libgit2_remote, repo, remote.toUtf8().constData());
            if (res)
            {
                throw QGitError("git_remote_lookup", res);
            }
        }

        for(const auto &branch: branches)
        {
            GitReference ref;
            res = git_branch_lookup(ref, repo, branch.toUtf8().constData(), GIT_BRANCH_LOCAL);
            if (res)
            {
                throw QGitError("Local branch not found!", res);
            }

            GitReference upstream_branch_ref;
            res = git_branch_upstream(upstream_branch_ref, ref);
            if (res)
            {
                throw QGitError("Local branch is not linked with remote branch!", res);
            }
        }

        GitStrArray refspecs;
        refspecs.value.count = branches.size();
        refspecs.value.strings = static_cast<char **>(malloc(sizeof(char *) * refspecs.value.count));
        for(int i = 0; i < branches.size(); i++)
        {
            QByteArray refspec = "refs/heads/" + branches[i].toUtf8() + ":refs/heads/" + branches[i].toUtf8();
            if (force) refspec.prepend("+");
            refspecs.value.strings[i] = strdup(refspec.constData());
        }

        git_push_options push_opts = GIT_PUSH_OPTIONS_INIT;
        push_opts.callbacks.payload = this;
        push_opts.callbacks.push_transfer_progress = [](unsigned int current, unsigned int total, size_t bytes,void *payload)->int
        {
            QGit *_this = static_cast<QGit *>(payload);
            emit _this->pushProgress(current, total, bytes);
            return 0;
        };

        push_opts.callbacks.credentials = sshKeyCredentialCallback;

        push_opts.callbacks.push_update_reference = [](const char *refname, const char *status, void *payload) -> int
        {
            Q_UNUSED(payload);
            if (status) {
                fprintf(stderr, "Push rejected for %s: %s\n", refname, status);
                return -1; // Returning non-zero will fail the push operation locally as well
            }
            return 0;
        };

        res = git_remote_push(libgit2_remote, &refspecs.value, &push_opts);
        if (res)
        {
            throw QGitError("git_remote_push", res);
        }

    } catch(const QGitError &ex) {
        error = ex;
    }

    emit pushReply(error);
}

void QGit::listCommits(QString object, int length)
{
    QList<QGitCommit> commits;
    QGitError error;

    try
    {
        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        GitRevWalk walker;
        res = git_revwalk_new(walker, repo);
        if (res)
        {
            throw QGitError("git_revwalk_new", res);
        }

        git_oid oid;
        if (object.isEmpty())
        {
            res = git_revwalk_push_head(walker);
            if (res)
            {
                length = 0;
            }
        }
        else
        {
            res = resolveToCommitOid(oid, repo, object);
            if (res)
            {
                throw QGitError("resolveToCommitOid", res);
            }

            res = git_revwalk_push(walker, &oid);
            if (res)
            {
                throw QGitError("git_revwalk_push", res);
            }

            memset(&oid, 0, sizeof(oid));

            git_revwalk_next(&oid, walker);

            memset(&oid, 0, sizeof(oid));
        }

        int count = 0;
        while ((!git_revwalk_next(&oid, walker))&&(count < length)) {

            QGitCommit item;
            unsigned int parents = 0;

            QString commit_id;
            QList<QGitCommitDiffParent> commit_parents;
            QDateTime commit_time;
            QGitSignature commit_author;
            QGitSignature commit_commiter;
            QString commit_message;

            GitCommit commit;
            res = git_commit_lookup(commit, repo, &oid);
            if (res)
            {
                throw QGitError("git_commit_lookup", res);
            }

            commit_id = QString::fromUtf8(git_oid_tostr_s(&oid));

            parents = git_commit_parentcount(commit);
            for(unsigned int index = 0; index < parents; index++)
            {
                GitCommit parent;
                QByteArray parentStr;

                res = git_commit_parent(parent, commit, index);
                if (res)
                {
                    throw QGitError("git_commit_parent", res);
                }

                const git_oid *parent_iod =  git_commit_id(parent);
                parentStr = QByteArray(git_oid_tostr_s(parent_iod));

                commit_parents.append(QGitCommitDiffParent(parentStr));
            }

            auto time = git_commit_time(commit);
            auto timeOffset = git_commit_time_offset(commit);
            commit_time = QDateTime::fromMSecsSinceEpoch(time * 1000);
            commit_time.setTimeZone(QTimeZone(timeOffset * 60));

            QString author_name = QString::fromUtf8(git_commit_author(commit)->name);
            QString author_email = QString::fromUtf8(git_commit_author(commit)->email);
            resolveMailmap(repo, author_name, author_email);
            QDateTime author_when = QDateTime::fromMSecsSinceEpoch(git_commit_author(commit)->when.time * 1000);
            author_when.setTimeZone(QTimeZone(git_commit_author(commit)->when.offset * 60));
            commit_author = QGitSignature(author_name, author_email, author_when);

            QString commiter_name =  QString::fromUtf8(git_commit_committer(commit)->name);
            QString commiter_email = QString::fromUtf8(git_commit_committer(commit)->email);
            resolveMailmap(repo, commiter_name, commiter_email);
            QDateTime commiter_when = QDateTime::fromMSecsSinceEpoch(git_commit_committer(commit)->when.time * 1000);
            commiter_when.setTimeZone(QTimeZone(git_commit_committer(commit)->when.offset * 60));
            commit_commiter = QGitSignature(commiter_name, commiter_email, commiter_when);

            commit_message = QString::fromUtf8(git_commit_message(commit));

            item = QGitCommit(commit_id, commit_parents, commit_time, commit_author, commit_commiter, commit_message);

            commits.push_back(item);

            count++;
        }

    } catch(const QGitError &ex) {
        error = ex;
    }

    emit listCommitsReply(commits, error);
}

void QGit::abortSearch()
{
    m_abortSearch = true;
}

void QGit::searchCommits(QString text, QString type)
{
    m_abortSearch = false;
    QGitError error;

    try
    {
        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        GitRevWalk walker;
        res = git_revwalk_new(walker, repo);
        if (res)
        {
            throw QGitError("git_revwalk_new", res);
        }

        res = git_revwalk_push_head(walker);
        if (res)
        {
            emit searchFinished();
            return;
        }

        git_revwalk_sorting(walker, GIT_SORT_TIME);

        git_oid oid;
        while (!git_revwalk_next(&oid, walker)) {
            if (m_abortSearch)
            {
                break;
            }
            GitCommit commit;
            res = git_commit_lookup(commit, repo, &oid);
            if (res)
            {
                continue;
            }

            bool matched = false;
            if (type == "message")
            {
                QString commit_message = QString::fromUtf8(git_commit_message(commit));
                if (commit_message.contains(text, Qt::CaseInsensitive))
                {
                    matched = true;
                }
            }
            else if (type == "author")
            {
                const git_signature *author = git_commit_author(commit);
                if (author)
                {
                    QString author_name = QString::fromUtf8(author->name);
                    QString author_email = QString::fromUtf8(author->email);
                    if (author_name.contains(text, Qt::CaseInsensitive) ||
                        author_email.contains(text, Qt::CaseInsensitive))
                    {
                        matched = true;
                    }
                }
            }
            else if (type == "files")
            {
                GitTree commit_tree;
                res = git_commit_tree(commit_tree, commit);
                if (res == 0)
                {
                    unsigned int parentCount = git_commit_parentcount(commit);
                    if (parentCount == 0)
                    {
                        git_diff_options options;
                        memset(&options, 0, sizeof(options));
                        options.version = GIT_DIFF_OPTIONS_VERSION;

                        GitDiff diff;
                        res = git_diff_tree_to_tree(diff, repo, nullptr, commit_tree, &options);
                        if (res == 0)
                        {
                            size_t _count = git_diff_num_deltas(diff);
                            for (size_t c = 0; c < _count; c++)
                            {
                                const git_diff_delta *delta = git_diff_get_delta(diff, c);
                                if (delta)
                                {
                                    if ((delta->old_file.path && QString::fromUtf8(delta->old_file.path).contains(text, Qt::CaseInsensitive)) ||
                                        (delta->new_file.path && QString::fromUtf8(delta->new_file.path).contains(text, Qt::CaseInsensitive)))
                                    {
                                        matched = true;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        for (unsigned int c = 0; c < parentCount; c++)
                        {
                            GitCommit parent;
                            res = git_commit_parent(parent, commit, c);
                            if (res == 0)
                            {
                                GitTree parent_tree;
                                res = git_commit_tree(parent_tree, parent);
                                if (res == 0)
                                {
                                    git_diff_options options;
                                    memset(&options, 0, sizeof(options));
                                    options.version = GIT_DIFF_OPTIONS_VERSION;

                                    GitDiff diff;
                                    res = git_diff_tree_to_tree(diff, repo, parent_tree, commit_tree, &options);
                                    if (res == 0)
                                    {
                                        size_t _count = git_diff_num_deltas(diff);
                                        for (size_t c2 = 0; c2 < _count; c2++)
                                        {
                                            const git_diff_delta *delta = git_diff_get_delta(diff, c2);
                                            if (delta)
                                            {
                                                if ((delta->old_file.path && QString::fromUtf8(delta->old_file.path).contains(text, Qt::CaseInsensitive)) ||
                                                    (delta->new_file.path && QString::fromUtf8(delta->new_file.path).contains(text, Qt::CaseInsensitive)))
                                                {
                                                    matched = true;
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            if (matched)
                            {
                                break;
                            }
                        }
                    }
                }
            }

            if (matched)
            {
                QString commit_id = QString::fromUtf8(git_oid_tostr_s(&oid));
                QList<QGitCommitDiffParent> commit_parents;
                unsigned int parents = git_commit_parentcount(commit);
                for (unsigned int index = 0; index < parents; index++)
                {
                    GitCommit parent;
                    res = git_commit_parent(parent, commit, index);
                    if (res == 0)
                    {
                        const git_oid *parent_oid = git_commit_id(parent);
                        QByteArray parentStr = QByteArray(git_oid_tostr_s(parent_oid));
                        commit_parents.append(QGitCommitDiffParent(parentStr));
                    }
                }

                auto time = git_commit_time(commit);
                auto timeOffset = git_commit_time_offset(commit);
                QDateTime commit_time = QDateTime::fromMSecsSinceEpoch(time * 1000);
                commit_time.setTimeZone(QTimeZone(timeOffset * 60));

                QString author_name = QString::fromUtf8(git_commit_author(commit)->name);
                QString author_email = QString::fromUtf8(git_commit_author(commit)->email);
                resolveMailmap(repo, author_name, author_email);
                QDateTime author_when = QDateTime::fromMSecsSinceEpoch(git_commit_author(commit)->when.time * 1000);
                author_when.setTimeZone(QTimeZone(git_commit_author(commit)->when.offset * 60));
                QGitSignature commit_author(author_name, author_email, author_when);

                QString commiter_name = QString::fromUtf8(git_commit_committer(commit)->name);
                QString commiter_email = QString::fromUtf8(git_commit_committer(commit)->email);
                resolveMailmap(repo, commiter_name, commiter_email);
                QDateTime commiter_when = QDateTime::fromMSecsSinceEpoch(git_commit_committer(commit)->when.time * 1000);
                commiter_when.setTimeZone(QTimeZone(git_commit_committer(commit)->when.offset * 60));
                QGitSignature commit_commiter(commiter_name, commiter_email, commiter_when);

                QString commit_message = QString::fromUtf8(git_commit_message(commit));

                QGitCommit item(commit_id, commit_parents, commit_time, commit_author, commit_commiter, commit_message);

                emit commitFound(item);
            }
        }

    }
    catch (const QGitError &ex)
    {
        error = ex;
    }

    emit searchFinished();
}

QString QGit::getNote(const QString &commitHash) const
{
    QString noteText;
    try {
        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if (res) throw QGitError("git_repository_open", res);

        git_oid oid;
        res = git_oid_fromstr(&oid, commitHash.toUtf8().constData());
        if (res) throw QGitError("git_oid_fromstr", res);

        git_note *note = nullptr;
        if (git_note_read(&note, repo, nullptr, &oid) == 0)
        {
            noteText = QString::fromUtf8(git_note_message(note));
            git_note_free(note);
        }
    } catch (...) {
        // Silently catch errors during synchronous retrieval
    }
    return noteText;
}

struct GitReflog {
    GitReflog() = default;
    GitReflog(const GitReflog&) = delete;
    GitReflog& operator=(const GitReflog&) = delete;
    GitReflog(GitReflog&&) = delete;
    GitReflog& operator=(GitReflog&&) = delete;
    operator git_reflog*() { return value; }
    operator git_reflog**() { return &value; }
    ~GitReflog() { if (value) { git_reflog_free(value); value = nullptr; }}
    git_reflog *value = nullptr;
};

QList<QGitReflogEntry> QGit::getReflog(const QString &refName) const
{
    QList<QGitReflogEntry> entries;
    try {
        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if (res) throw QGitError("git_repository_open", res);

        GitReflog reflog;
        QString fullRefName = refName;
        if (refName != QStringLiteral("HEAD") && !refName.startsWith(QStringLiteral("refs/"))) {
            fullRefName = QStringLiteral("refs/heads/") + refName;
        }

        res = git_reflog_read(reflog, repo, fullRefName.toUtf8().constData());
        if (res) {
            res = git_reflog_read(reflog, repo, refName.toUtf8().constData());
        }

        if (res == 0) {
            size_t count = git_reflog_entrycount(reflog);
            for (size_t i = 0; i < count; i++) {
                const git_reflog_entry *entry = git_reflog_entry_byindex(reflog, i);
                if (entry) {
                    QGitReflogEntry qEntry;
                    
                    const git_oid *oid = git_reflog_entry_id_new(entry);
                    if (oid) {
                        char oid_str[GIT_OID_HEXSZ + 1];
                        git_oid_tostr(oid_str, sizeof(oid_str), oid);
                        qEntry.commitHash = QString::fromLatin1(oid_str);
                    }
                    
                    const git_signature *committer = git_reflog_entry_committer(entry);
                    if (committer) {
                        qEntry.committerName = QString::fromUtf8(committer->name);
                        qEntry.committerEmail = QString::fromUtf8(committer->email);
                        qEntry.time = committer->when.time;
                    }
                    
                    const char *msg = git_reflog_entry_message(entry);
                    if (msg) {
                        qEntry.message = QString::fromUtf8(msg);
                    }
                    
                    entries.append(qEntry);
                }
            }
        }
    } catch (...) {
    }
    return entries;
}

void QGit::setNote(QString commitHash, QString noteText)
{
    QGitError error;
    try {
        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if (res) throw QGitError("git_repository_open", res);

        git_oid oid;
        res = git_oid_fromstr(&oid, commitHash.toUtf8().constData());
        if (res) throw QGitError("git_oid_fromstr", res);

        GitSignature me;
        res = git_signature_default(me, repo);
        if (res) throw QGitError("git_signature_default", res);

        git_oid note_oid;
        res = git_note_create(&note_oid, repo, nullptr, me, me, &oid, noteText.toUtf8().constData(), 1);
        if (res) throw QGitError("git_note_create", res);

    } catch (const QGitError &ex) {
        error = ex;
    }
    emit setNoteReply(error);
}

void QGit::removeNote(QString commitHash)
{
    QGitError error;
    try {
        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if (res) throw QGitError("git_repository_open", res);

        git_oid oid;
        res = git_oid_fromstr(&oid, commitHash.toUtf8().constData());
        if (res) throw QGitError("git_oid_fromstr", res);

        GitSignature me;
        res = git_signature_default(me, repo);
        if (res) throw QGitError("git_signature_default", res);

        res = git_note_remove(repo, nullptr, me, me, &oid);
        if (res) throw QGitError("git_note_remove", res);

    } catch (const QGitError &ex) {
        error = ex;
    }
    emit removeNoteReply(error);
}
