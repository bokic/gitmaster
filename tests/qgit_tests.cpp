// SPDX-License-Identifier: LGPL-3.0-or-later
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>

#include <git2.h>
#include <cstring>
#include <exception>
#include <iostream>

#include "qgit.h"
#include "qgiterror.h"

namespace {

int g_failures = 0;

#define CHECK(cond)                                                                                \
    do {                                                                                           \
        if (!(cond)) {                                                                             \
            std::cerr << "FAIL [" << __FILE__ << ":" << __LINE__ << "]: " #cond << std::endl;      \
            ++g_failures;                                                                          \
        }                                                                                          \
    } while (false)

void writeFile(const QString &filePath, const QByteArray &content)
{
    QFileInfo fi(filePath);
    QDir().mkpath(fi.absolutePath());
    QFile f(filePath);
    CHECK(f.open(QIODevice::WriteOnly | QIODevice::Truncate));
    CHECK(f.write(content) == content.size());
    f.close();
}

void initRepoWithCommit(const QString &repoPath)
{
    git_repository *repo = nullptr;
    CHECK(git_repository_init(&repo, repoPath.toUtf8().constData(), 0) == 0);

    git_config *cfg = nullptr;
    CHECK(git_repository_config(&cfg, repo) == 0);
    CHECK(git_config_set_string(cfg, "user.name", "Test User") == 0);
    CHECK(git_config_set_string(cfg, "user.email", "test@example.com") == 0);
    CHECK(git_config_set_bool(cfg, "core.autocrlf", 0) == 0);
    git_config_free(cfg);

    writeFile(repoPath + "/README.md", "hello\n");

    git_index *idx = nullptr;
    CHECK(git_repository_index(&idx, repo) == 0);
    CHECK(git_index_add_bypath(idx, "README.md") == 0);
    CHECK(git_index_write(idx) == 0);

    git_oid treeId;
    CHECK(git_index_write_tree(&treeId, idx) == 0);
    git_index_free(idx);

    git_tree *tree = nullptr;
    CHECK(git_tree_lookup(&tree, repo, &treeId) == 0);

    git_signature *sig = nullptr;
    CHECK(git_signature_now(&sig, "Test User", "test@example.com") == 0);

    git_oid commitId;
    CHECK(git_commit_create_v(&commitId, repo, "HEAD", sig, sig, nullptr, "Initial commit", tree, 0) == 0);

    git_signature_free(sig);
    git_tree_free(tree);
    git_repository_free(repo);
}

void testQGitErrorStdException()
{
    bool caughtAsStdException = false;
    try {
        throw QGitError("testFunc", -1, "boom");
    } catch (const std::exception &ex) {
        caughtAsStdException = true;
        CHECK(std::string(ex.what()) == "boom");
    }
    CHECK(caughtAsStdException);
}

void testRemoveSubmodulePathTraversal()
{
    QTemporaryDir tmpRoot;
    CHECK(tmpRoot.isValid());

    const QString repoPath = tmpRoot.filePath("repo");
    const QString victimPath = tmpRoot.filePath("victim-dir");
    const QString victimFile = victimPath + "/secret.txt";
    initRepoWithCommit(repoPath);
    writeFile(victimFile, "do not delete");

    // Register submodule "evil" in repo, then rewrite .gitmodules with a path traversal
    git_repository *repo = nullptr;
    CHECK(git_repository_open(&repo, repoPath.toUtf8().constData()) == 0);
    git_submodule *sm = nullptr;
    CHECK(git_submodule_add_setup(&sm, repo, "https://example.com/evil.git", "evil", 1) == 0);
    if (sm) git_submodule_free(sm);
    git_repository_free(repo);

    writeFile(repoPath + "/.gitmodules",
              "[submodule \"evil\"]\n\tpath = ../victim-dir\n\turl = https://example.com/evil.git\n");

    QGit git;
    git.setPath(QDir(repoPath));

    QGitError lastErr;
    QObject::connect(&git, &QGit::removeSubmoduleReply, [&](const QGitError &err) { lastErr = err; });
    git.removeSubmodule("../victim-dir", QGit::SubmoduleRemoveOption::RemoveWorkingDirectory);

    CHECK(lastErr.hasError());
    CHECK(QDir(victimPath).exists());
    CHECK(QFile::exists(victimFile));

    // Also test absolute path rejection
    lastErr = QGitError();
    git.removeSubmodule(victimPath, QGit::SubmoduleRemoveOption::RemoveWorkingDirectory);
    CHECK(lastErr.hasError());
    CHECK(QDir(victimPath).exists());
    CHECK(QFile::exists(victimFile));
}

void testStashAddressingByIndex()
{
    QTemporaryDir tmpRoot;
    CHECK(tmpRoot.isValid());
    const QString repoPath = tmpRoot.filePath("repo");
    initRepoWithCommit(repoPath);

    git_repository *repo = nullptr;
    CHECK(git_repository_open(&repo, repoPath.toUtf8().constData()) == 0);
    git_signature *sig = nullptr;
    CHECK(git_signature_now(&sig, "Test User", "test@example.com") == 0);

    // Stash 1 (will become stash@{1})
    writeFile(repoPath + "/README.md", "stash-older\n");
    git_oid oid1;
    CHECK(git_stash_save(&oid1, repo, sig, "dup-msg", GIT_STASH_DEFAULT) == 0);

    // Stash 2 (will become stash@{0})
    writeFile(repoPath + "/README.md", "stash-newer\n");
    git_oid oid2;
    CHECK(git_stash_save(&oid2, repo, sig, "dup-msg", GIT_STASH_DEFAULT) == 0);

    git_signature_free(sig);
    git_repository_free(repo);

    QGit git;
    git.setPath(QDir(repoPath));

    QGitError removeErr;
    QObject::connect(&git, &QGit::stashRemoveReply, [&](const QGitError &err) { removeErr = err; });
    // Remove stash@{1} (the older one) specifically
    git.stashRemove("stash@{1}");
    CHECK(!removeErr.hasError());

    // Remaining stash@{0} should be the newer one ("stash-newer")
    QGitError applyErr;
    QObject::connect(&git, &QGit::stashApplyReply, [&](const QGitError &err) { applyErr = err; });
    git.stashApply("stash@{0}");
    CHECK(!applyErr.hasError());

    QFile f(repoPath + "/README.md");
    CHECK(f.open(QIODevice::ReadOnly));
    CHECK(f.readAll() == "stash-newer\n");
}

void testArchiveExportZipAndTar()
{
    QTemporaryDir tmpRoot;
    CHECK(tmpRoot.isValid());
    const QString repoPath = tmpRoot.filePath("repo");
    initRepoWithCommit(repoPath);

    QGit git;
    git.setPath(QDir(repoPath));

    // 1. Valid ZIP export
    const QString zipOut = tmpRoot.filePath("out.zip");
    QString created;
    QGitError err;
    QObject::connect(&git, &QGit::exportArchiveReply, [&](const QString &path, const QGitError &e) {
        created = path;
        err = e;
    });
    git.exportArchive("HEAD", zipOut, "proj", "zip");
    CHECK(!err.hasError());
    CHECK(created == zipOut);
    CHECK(QFileInfo(zipOut).size() > 0);

    // 2. Valid TAR export
    const QString tarOut = tmpRoot.filePath("out.tar");
    git.exportArchive("HEAD", tarOut, "proj", "tar");
    CHECK(!err.hasError());
    CHECK(created == tarOut);
    CHECK(QFileInfo(tarOut).size() >= 1024);

    // 3. Unsplittable >100-byte filename in TAR should error, not silently truncate
    const QString longName = QString(120, QLatin1Char('a')) + ".txt";
    writeFile(repoPath + "/" + longName, "long");
    git_repository *repo = nullptr;
    CHECK(git_repository_open(&repo, repoPath.toUtf8().constData()) == 0);
    git_index *idx = nullptr;
    CHECK(git_repository_index(&idx, repo) == 0);
    CHECK(git_index_add_bypath(idx, longName.toUtf8().constData()) == 0);
    CHECK(git_index_write(idx) == 0);
    git_oid treeId;
    CHECK(git_index_write_tree(&treeId, idx) == 0);
    git_index_free(idx);
    git_tree *tree = nullptr;
    CHECK(git_tree_lookup(&tree, repo, &treeId) == 0);
    git_signature *sig = nullptr;
    CHECK(git_signature_now(&sig, "Test User", "test@example.com") == 0);
    git_oid headOid;
    CHECK(git_reference_name_to_id(&headOid, repo, "HEAD") == 0);
    git_commit *parent = nullptr;
    CHECK(git_commit_lookup(&parent, repo, &headOid) == 0);
    git_oid c2;
    CHECK(git_commit_create_v(&c2, repo, "HEAD", sig, sig, nullptr, "Long filename", tree, 1, parent) == 0);
    git_commit_free(parent);
    git_signature_free(sig);
    git_tree_free(tree);
    git_repository_free(repo);

    const QString badTarOut = tmpRoot.filePath("bad.tar");
    git.exportArchive("HEAD", badTarOut, "proj", "tar");
    CHECK(err.hasError());
    CHECK(!QFile::exists(badTarOut));
}

void testCleanSkipsSymlinksAndNestedRepos()
{
    QTemporaryDir tmpRoot;
    CHECK(tmpRoot.isValid());
    const QString repoPath = tmpRoot.filePath("repo");
    const QString outsideDir = tmpRoot.filePath("outside-empty");
    const QString nestedRepo = repoPath + "/nested-repo";
    initRepoWithCommit(repoPath);
    initRepoWithCommit(nestedRepo);
    CHECK(QDir().mkpath(outsideDir));

#ifndef Q_OS_WIN
    CHECK(QFile::link(outsideDir, repoPath + "/link-to-outside"));
#endif

    QGit git;
    git.setPath(QDir(repoPath));
    QGitError cleanErr;
    QObject::connect(&git, &QGit::cleanReply, [&](const QGitError &e) { cleanErr = e; });
    git.clean(QGit::CleanOption::IncludeIgnored | QGit::CleanOption::RemoveDirectories);

    CHECK(!cleanErr.hasError());
    CHECK(QDir(outsideDir).exists());
    CHECK(QDir(nestedRepo + "/.git").exists());
}

} // namespace

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    git_libgit2_init();

    auto runTest = [](const char *name, void (*fn)()) {
        std::cout << "[RUN ] " << name << std::endl;
        fn();
        std::cout << "[DONE] " << name << std::endl;
    };

    runTest("testQGitErrorStdException", testQGitErrorStdException);
    runTest("testRemoveSubmodulePathTraversal", testRemoveSubmodulePathTraversal);
    runTest("testStashAddressingByIndex", testStashAddressingByIndex);
    runTest("testArchiveExportZipAndTar", testArchiveExportZipAndTar);
    runTest("testCleanSkipsSymlinksAndNestedRepos", testCleanSkipsSymlinksAndNestedRepos);

    git_libgit2_shutdown();
    return g_failures == 0 ? 0 : 1;
}
