#include "qgitrepository.h"
#include "ui_qgitrepository.h"
#include "qloghistoryitemdelegate.h"
#include "qgitmastermainwindow.h"
#include "qgitfetchdialog.h"
#include "qgitpulldialog.h"
#include "qgitpushdialog.h"
#include "qgitbranchdialog.h"
#include "qgitmergedialog.h"
#include "qgitconflictresolverdialog.h"
#include "qgitcreatetagdialog.h"
#include "qgitcleandialog.h"
#include "qgitreflogdialog.h"
#include "qgitremotesdialog.h"
#include "qgitblamedialog.h"
#include "qgitworktreedialog.h"
#include "qgitflowdialog.h"
#include <qgitbranch.h>

#include <QDebug>
#include <QCryptographicHash>
#include <QTableWidgetItem>
#include <QTreeWidgetItem>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QKeyEvent>
#include <QScrollBar>
#include <QWindow>
#include <QMenu>
#include <QAction>
#include <QList>
#include <QFile>
#include <QTextStream>

#define COMMIT_COUNT_TO_LOAD 100


#include <QLineEdit>
#include <QStyledItemDelegate>

#include <QHBoxLayout>
#include <QCheckBox>
#include <QLabel>
#include <QToolButton>
#include <QMouseEvent>
#include <QFileInfo>
#include "qgitbranchtreewidget.h"

QGitRepository::QGitRepository(const QString &path, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::QGitRepository)
    , m_path(path)
    , m_allCommitsLoaded(false)
    , m_iconFileNew(":/small/added")
    , m_iconFileClean(":/small/clean")
    , m_iconFileModified(":/small/modified")
    , m_iconFileRemoved(":/small/deleted")
    , m_iconFileRenamed(":/small/images/small/renamed.png")
    , m_iconFileConflict(":/small/images/small/conflict.png")
    , m_iconFileIgnored(":/small/ignored")
    , m_iconFileUnknown(":/small/unknown")
    , m_iconTag(":/small/tag")
    , m_iconWhiteCheckbox(":/small/white_checkbox")
    , m_iconCurrentBranch(":/small/current_branch")
    , m_iconBranch(":/small/branch")
    , m_iconRemote(":/small/remote")
    , m_iconStash(":/small/stash")
    , m_iconRemoteBranch(":/small/remote_branch")
    , m_iconSubmodule(":/small/submodule")
    , m_iconWorktree(":/small/worktree")
    , m_currentLogBranchFilter("")
    , m_logCommitsOffset(0)
    , m_git(new QGit())
    , m_hasRemotes(false)
    , m_hasCommitsToPush(false)
{
    QString name;
    QString email;

    ui->setupUi(this);

    ui->treeWidget_staged->setHeaderHidden(true);
    ui->treeWidget_staged->setColumnCount(1);
    ui->treeWidget_unstaged->setHeaderHidden(true);
    ui->treeWidget_unstaged->setColumnCount(1);
    ui->treeWidget_unstaged->setContextMenuPolicy(Qt::CustomContextMenu);

    ui->treeWidget_pending->setHeaderHidden(true);
    ui->treeWidget_pending->setColumnCount(1);
    ui->treeWidget_pending->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->comboBox_logBranchFilter, &QComboBox::currentTextChanged,
            this, &QGitRepository::on_comboBox_logBranchFilter_currentTextChanged);

    ui->commit_diff->setContentsMargins(10, 10, 10, 10);

    ui->logHistory_commits->setItemDelegate(new QLogHistoryItemDelegate(ui->logHistory_commits));

    m_git->moveToThread(&m_thread);

    ui->splitter_3->setStretchFactor(0, 0);
    ui->splitter_3->setStretchFactor(1, 1);

    ui->logHistory_splitter_1->setStretchFactor(0, 0);
    ui->logHistory_splitter_1->setStretchFactor(1, 1);

    ui->logHistory_splitter_2->setStretchFactor(0, 0);
    ui->logHistory_splitter_2->setStretchFactor(1, 1);

    ui->splitter_5->setSizes(QList<int>() << 100 << 100);


    if (QGit::gitRepositoryDefaultSignature(m_path, name, email))
    {
        QByteArray hash;

        hash = QCryptographicHash::hash(email.trimmed().toUtf8(), QCryptographicHash::Md5).toHex();

        QString urlStr = QString("https://www.gravatar.com/avatar/%1?s=24").arg(QString::fromLatin1(hash));
        auto url = QUrl(urlStr);
        connect(&m_networkManager, &QNetworkAccessManager::finished, this, &QGitRepository::gravatarImageDownloadFinished);

        m_networkManager.get(QNetworkRequest(url));

        ui->label_signatureGravatarImage->setToolTip(tr("%1 <%2>").arg(name, email));
        ui->label_signatureEmail->setText(tr("%1 <%2>").arg(name, email));
    }


    ui->logHistory_files->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->search_files->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->logHistory_commits->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->search_commits->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

    ui->logHistory_files->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->logHistory_files, &QTableWidget::customContextMenuRequested,
            this, &QGitRepository::on_logHistory_files_customContextMenuRequested);

    ui->search_files->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->search_files, &QTableWidget::customContextMenuRequested,
            this, &QGitRepository::on_search_files_customContextMenuRequested);

    activateCommitOperation(false);

    connect(this, &QGitRepository::localStash, m_git, &QGit::stashSave);
    connect(m_git, &QGit::stashSaveReply, this, &QGitRepository::localStashSaveReply);
    connect(m_git, &QGit::stashRemoveReply, this, &QGitRepository::localStashRemoveReply);

    connect(this, &QGitRepository::repositoryFetch, m_git, &QGit::fetch);
    connect(m_git, &QGit::fetchReply, this, &QGitRepository::repositoryFetchReply);

    connect(this, &QGitRepository::repositoryPull, m_git, &QGit::pull);
    connect(m_git, &QGit::pullReply, this, &QGitRepository::repositoryPullReply);

    connect(this, &QGitRepository::repositoryUpdateSubmodule, m_git, &QGit::updateSubmodule);
    connect(m_git, &QGit::updateSubmoduleReply, this, &QGitRepository::repositoryUpdateSubmoduleReply);

    connect(this, &QGitRepository::repositoryPush, m_git, &QGit::push);
    connect(m_git, &QGit::pushReply, this, &QGitRepository::repositoryPushReply);

    connect(this, &QGitRepository::repositoryBranches, m_git, &QGit::listBranchesAndTags);
    connect(m_git, &QGit::listBranchesAndTagsReply, this, &QGitRepository::repositoryBranchesAndTagsReply);

    connect(this, &QGitRepository::repositoryStashes, m_git, &QGit::listStashes);
    connect(m_git, &QGit::listStashesReply, this, &QGitRepository::repositoryStashesReply);

    connect(this, &QGitRepository::repositoryChangedFiles, m_git, &QGit::listChangedFiles);
    connect(m_git, &QGit::listChangedFilesReply, this, &QGitRepository::repositoryChangedFilesReply);

    connect(this, &QGitRepository::repositoryStageFiles, m_git, &QGit::stageFiles);
    connect(m_git, &QGit::stageFilesReply, this, &QGitRepository::repositoryStageFilesReply);

    connect(this, &QGitRepository::repositoryUnstageFiles, m_git, &QGit::unstageFiles);
    connect(m_git, &QGit::unstageFilesReply, this, &QGitRepository::repositoryUnstageFilesReply);

    connect(this, &QGitRepository::repositoryDiscardFiles, m_git, &QGit::discardFiles);
    connect(m_git, &QGit::discardFilesReply, this, &QGitRepository::repositoryDiscardFilesReply);

    connect(this, &QGitRepository::repositoryCommit, m_git, &QGit::commit);
    connect(m_git, &QGit::commitReply, this, &QGitRepository::repositoryCommitReply);

    connect(this, &QGitRepository::repositoryGetCommits, m_git, &QGit::listCommits);
    connect(m_git, &QGit::listCommitsReply, this, &QGitRepository::repositoryGetCommitsReply);

    connect(this, &QGitRepository::repositorySearchCommits, m_git, &QGit::searchCommits);
    connect(this, &QGitRepository::repositoryAbortSearch, m_git, &QGit::abortSearch);
    connect(m_git, &QGit::commitFound, this, &QGitRepository::onCommitFound);
    connect(m_git, &QGit::searchFinished, this, &QGitRepository::onSearchFinished);

    connect(this, &QGitRepository::repositoryGetCommitDiff, m_git, &QGit::commitDiff);
    connect(m_git, &QGit::commitDiffReply, this, &QGitRepository::repositoryGetCommitDiffReply);

    connect(ui->commit_diff, &QGitDiffWidget::select, this, &QGitRepository::selectedLines);

    ui->treeWidget_unstaged->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->commit_diff->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, &QGitRepository::stageFileLines, m_git, &QGit::stageFileLines);
    connect(this, &QGitRepository::unstageFileLines, m_git, &QGit::unstageFileLines);
    connect(this, &QGitRepository::repositoryDiscardFileLines, m_git, &QGit::discardFileLines);

    connect(this, &QGitRepository::deleteBranches, m_git, &QGit::deleteBranches);
    connect(m_git, &QGit::deleteBranchesReply, this, &QGitRepository::deleteBranchesReply);


    connect(ui->logHistory_commits->verticalScrollBar(), &QScrollBar::valueChanged, this, &QGitRepository::historyTableSliderMoved);
    connect(ui->plainTextEdit_commitMessage, &QAdvPlainTextEdit::abort, ui->pushButton_commitCancel, &QPushButton::click);

    connect(m_git, &QGit::stashApplyReply, this, &QGitRepository::stashApplyReply);
    connect(m_git, &QGit::stashPopReply, this, &QGitRepository::stashPopReply);

    connect(m_git, &QGit::checkoutBranchReply, this, &QGitRepository::checkoutBranchReply);
    connect(m_git, &QGit::renameBranchReply, this, &QGitRepository::renameBranchReply);
    connect(m_git, &QGit::setUpstreamReply, this, &QGitRepository::setUpstreamReply);
    connect(m_git, &QGit::deleteTagReply, this, &QGitRepository::deleteTagReply);
    connect(this, &QGitRepository::repositoryMerge, m_git, &QGit::merge);
    connect(m_git, &QGit::mergeReply, this, &QGitRepository::repositoryMergeReply);
    connect(this, &QGitRepository::repositoryRebase, m_git, &QGit::rebase);
    connect(m_git, &QGit::rebaseReply, this, &QGitRepository::repositoryRebaseReply);
    connect(this, &QGitRepository::repositoryRenameBranch, m_git, &QGit::renameBranch);
    connect(this, &QGitRepository::repositoryRenameTag, m_git, &QGit::renameTag);
    connect(m_git, &QGit::renameTagReply, this, &QGitRepository::renameTagReply);
    connect(this, &QGitRepository::repositoryCreateTag, m_git, &QGit::createTag);
    connect(m_git, &QGit::createTagReply, this, &QGitRepository::createTagReply);
    connect(this, &QGitRepository::repositoryClean, m_git, &QGit::clean);
    connect(m_git, &QGit::cleanReply, this, &QGitRepository::repositoryCleanReply);
    connect(this, &QGitRepository::repositoryApplyPatch, m_git, &QGit::applyPatch);
    connect(m_git, &QGit::applyPatchReply, this, &QGitRepository::repositoryApplyPatchReply);
    connect(this, &QGitRepository::repositorySetNote, m_git, &QGit::setNote);
    connect(m_git, &QGit::setNoteReply, this, &QGitRepository::repositorySetNoteReply);
    connect(this, &QGitRepository::repositoryRemoveNote, m_git, &QGit::removeNote);
    connect(m_git, &QGit::removeNoteReply, this, &QGitRepository::repositoryRemoveNoteReply);

    ui->branchesTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->branchesTreeView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    ui->logHistory_commits->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->logHistory_commits, &QLogHistoryTableWidget::customContextMenuRequested, this, &QGitRepository::on_logHistory_commits_customContextMenuRequested);

    m_git->setPath(QDir(m_path));

    m_thread.setObjectName("QGit(repo)");
    m_thread.start();

    connect(ui->comboBox_gitDiffOptions, &QComboBoxGitDiffOptions::optionsChanged, this, &QGitRepository::on_comboBox_gitDiffOptions_optionsChanged);
}

QGitRepository::~QGitRepository()
{
    m_git->disconnect(this);
    this->disconnect(m_git);

    m_thread.requestInterruption();
    m_thread.quit();
    m_thread.wait();

    delete m_git;
    m_git = nullptr;

    delete ui;
}

void QGitRepository::refreshData()
{
    if (m_refreshing)
        return;

    struct RefreshGuard {
        bool &flag;
        RefreshGuard(bool &f) : flag(f) { flag = true; }
        ~RefreshGuard() { flag = false; }
    } guard(m_refreshing);

    on_repositoryDetail_currentChanged(ui->repositoryDetail->currentIndex());

    if (ui->repositoryDetail->currentIndex() != 0)
    {
        fetchRepositoryChangedFiles();
    }

    emit repositoryBranches();
    emit repositoryStashes();

    emit updateRemoteActionsRequested();
}

void QGitRepository::stash(const QString &name, bool keepIndex, bool includeUntracked, bool includeIgnored)
{
    emit localStash(name, keepIndex, includeUntracked, includeIgnored);
}

void QGitRepository::branchDialog()
{
    QGitBranchDialog dlg(this);

    if (dlg.exec() == QDialog::Accepted)
    {
        if (dlg.operation() == QGitBranchDialog::NewBranchOperation)
        {
            auto branchName = dlg.newBranchName();
            auto commitId = dlg.newBranchCommitId();
            auto checkout = dlg.newBranchCheckout();
            try {
                m_git->createLocalBranch(branchName, commitId, checkout);
                emit repositoryBranches();
            } catch (const QGitError &error) {
                QMessageBox::critical(this, tr("Error creating branch"), error.errorString());
            }
        }
        else if (dlg.operation() == QGitBranchDialog::DeleteBranchesOperation)
        {
            auto branches = dlg.deleteBranches();
            auto forced = dlg.forceDelete();

            emit deleteBranches(branches, forced);
        }
    }
}

void QGitRepository::worktreeDialog()
{
    QGitWorktreeDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        try {
            m_git->addWorktree(dlg.worktreeName(), dlg.worktreePath(),
                               dlg.worktreeBranch(), dlg.createNewBranch());
            refreshData();
        } catch (const QGitError &error) {
            QMessageBox::critical(this, tr("Error adding worktree"), error.errorString());
        }
    }
}

void QGitRepository::gitFlowDialog()
{
    QGitFlowDialog dlg(this);
    dlg.exec();
}

void QGitRepository::commit()
{
    ui->repositoryDetail->setCurrentIndex(0);
    activateCommitOperation(true);
    ui->plainTextEdit_commitMessage->setFocus();
}

void QGitRepository::fetch()
{
    QGitFetchDialog dlg(this);

    if (dlg.exec() == QDialog::Accepted)
    {
        bool fetchFromAllRemotes = dlg.fetchFromAllRemotes();
        bool purgeDeletedBranches = dlg.purgeDeletedBranches();
        bool fetchAllTags = dlg.fetchAllTags();

        emit statusMessage("Fetching...");
        emit repositoryFetch(fetchFromAllRemotes, purgeDeletedBranches, fetchAllTags);
    }
}

void QGitRepository::pull()
{
    QGitPullDialog dlg(this);

    if (dlg.exec() == QDialog::Accepted)
    {
        QString remoteName = dlg.remote().name;
        if (remoteName == tr("custom"))
        {
            remoteName = dlg.remote().url;
        }
        QString branchName = dlg.branch();
        bool rebase = dlg.rebase();

        emit statusMessage(tr("Pulling..."));
        emit repositoryPull(remoteName, branchName, rebase);
    }
}

void QGitRepository::push()
{
    QGitPushDialog dlg(this);

    if (dlg.exec() == QDialog::Accepted)
    {
        QString remote = dlg.remote();
        QStringList branches = dlg.branches();
        bool tags = dlg.tags();
        bool force = dlg.force();

        m_lastRemote = remote;
        m_lastBranches = branches;
        m_lastTags = tags;

        emit statusMessage(tr("Pushing..."));
        emit repositoryPush(remote, branches, tags, force);
    }
}

void QGitRepository::merge()
{
    QGitMergeDialog dlg(m_git->path(), this);

    if (dlg.exec() == QDialog::Accepted)
    {
        QString branchName = dlg.selectedBranch();
        if (!branchName.isEmpty()) {
            if (dlg.rebaseInsteadOfMerge()) {
                emit statusMessage(tr("Rebasing current branch onto %1...").arg(branchName));
                emit repositoryRebase(branchName);
            } else {
                emit statusMessage(tr("Merging branch %1...").arg(branchName));
                emit repositoryMerge(branchName);
            }
        }
    }
}

void QGitRepository::rebase()
{
    QGitMergeDialog dlg(m_git->path(), this);
    dlg.setRebaseMode(true);

    if (dlg.exec() == QDialog::Accepted)
    {
        QString branchName = dlg.selectedBranch();
        if (!branchName.isEmpty()) {
            emit statusMessage(tr("Rebasing current branch onto %1...").arg(branchName));
            emit repositoryRebase(branchName);
        }
    }
}

void QGitRepository::repositoryMergeReply(const QGitError &error)
{
    emit clearStatusMessage();

    if (error.errorCode()) {
        QMessageBox::critical(this, tr("Merge Error"), 
                              tr("Could not merge the selected branch. Details: %1").arg(error.errorString()));
    } else {
        refreshData();
    }
}

void QGitRepository::repositoryRebaseReply(const QGitError &error)
{
    emit clearStatusMessage();

    if (error.errorCode()) {
        QMessageBox::critical(this, tr("Rebase Error"), 
                              tr("Could not rebase onto the specified branch. Details: %1").arg(error.errorString()));
    } else {
        refreshData();
    }
}

QGit *QGitRepository::git() const
{
    return m_git;
}

void QGitRepository::navigateToCommit(const QString &hash)
{
    if (hash.isEmpty()) return;
    ui->repositoryDetail->setCurrentWidget(ui->tabLogHistory);
    if (!ui->logHistory_commits->selectCommit(hash))
    {
        if (m_allCommitsLoaded) {
            QMessageBox::information(this, tr("Commit not found"),
                                     tr("The commit [%1] was not found in the repository history.").arg(hash.left(10)));
        } else {
            m_searchingCommitHash = hash;
            fetchCommits();
            emit statusMessage(tr("Searching for commit %1...").arg(hash.left(10)));
        }
    }
}

bool QGitRepository::event(QEvent *event)
{
    if (event->type() == QEvent::Show)
    {
        emit repositoryBranches();
        emit repositoryStashes();
    }

    return QWidget::event(event);
}

void QGitRepository::gravatarImageDownloadFinished(QNetworkReply *reply)
{
    if (reply) {
        QByteArray data = reply->readAll();

        QPixmap pixmap;

        pixmap.loadFromData(data);

        ui->label_signatureGravatarImage->setPixmap(pixmap);

        reply->deleteLater();
    }
}

void QGitRepository::localStashSaveReply(const QGitError &error)
{
    if (error.errorCode()) {
        QMessageBox::critical(this, tr("Error saving stash"), error.errorString());
    } else {
        refreshData();
    }
}

void QGitRepository::localStashRemoveReply(const QGitError &error)
{
    if (error.errorCode()) {
        QMessageBox::critical(this, tr("Error dropping stash"), error.errorString());
    } else {
        refreshData();
    }
}

void QGitRepository::repositoryFetchReply(const QGitError &error)
{
    if (error.errorCode())
    {
        emit statusMessage(error.functionName());
    }
    else
    {
        emit clearStatusMessage();
    }
}

void QGitRepository::repositoryPullReply(const QGitError &error)
{
    if (error.errorCode())
    {
        emit statusMessage(error.errorString());
        QMessageBox::critical(this, tr("Pull Error"), error.errorString());
    }
    else
    {
        emit clearStatusMessage();
        refreshData();
    }
}

void QGitRepository::repositoryUpdateSubmoduleReply(const QGitError &error)
{
    if (error.errorCode())
    {
        emit statusMessage(error.errorString());
        QMessageBox::critical(this, tr("Submodule Update Error"), error.errorString());
    }
    else
    {
        emit clearStatusMessage();
        refreshData();
        QMessageBox::information(this, tr("Submodule Updated"), tr("Submodule updated successfully."));
    }
}

void QGitRepository::repositoryPushReply(const QGitError &error)
{
    if (error.errorCode())
    {
        QString errStr = error.errorString();
        emit statusMessage(errStr);

        if (errStr.contains(QStringLiteral("rejected"), Qt::CaseInsensitive) || errStr.contains(QStringLiteral("non-fast-forward"), Qt::CaseInsensitive))
        {
            auto res = QMessageBox::question(this, tr("Push Rejected"), 
                                             tr("Push was rejected by the remote (likely non-fast-forward). Do you want to force push?"),
                                             QMessageBox::Yes | QMessageBox::No);
            if (res == QMessageBox::Yes)
            {
                emit statusMessage(tr("Force Pushing..."));
                emit repositoryPush(m_lastRemote, m_lastBranches, m_lastTags, true);
                return;
            }
        }
    }
    else
    {
        emit clearStatusMessage();
    }
}

void QGitRepository::repositoryBranchesAndTagsReply(const QList<QGitBranch> &branches, const QList<QGitTag> &tags, const QList<QGitSubmodule> &submodules, const QList<QGitWorktree> &worktrees, bool hasRemotes, bool hasCommitsToPush, const QGitError &error)
{
    if (error.errorCode())
    {
        emit statusMessage(tr("Error loading branches/tags: %1").arg(error.errorString()));
    }

    m_hasRemotes = hasRemotes;
    m_hasCommitsToPush = hasCommitsToPush;

    emit updateRemoteActionsRequested();

    auto current_branch = QGit::getBranchNameFromPath(m_git->path().path());

    ui->branchesTreeView->populateBranchesAndTags(branches, tags, submodules, worktrees, current_branch);

    // Populate the branch filter dropdown
    ui->comboBox_logBranchFilter->blockSignals(true);
    QString currentSavedText = ui->comboBox_logBranchFilter->currentText();
    ui->comboBox_logBranchFilter->clear();

    ui->comboBox_logBranchFilter->addItem(tr("All Branches"), QStringLiteral("all"));
    ui->comboBox_logBranchFilter->addItem(tr("Current Branch (HEAD)"), QStringLiteral(""));

    // Add local branches
    ui->comboBox_logBranchFilter->addItem(QStringLiteral("--- Local Branches ---"), QVariant());
    for (const auto &branch : branches) {
        if (branch.type() & GIT_BRANCH_LOCAL) {
            QString name = branch.name();
            if (name.startsWith(QStringLiteral("refs/heads/")))
                name = name.mid(11);
            ui->comboBox_logBranchFilter->addItem(name, branch.name());
        }
    }

    // Add remote branches
    ui->comboBox_logBranchFilter->addItem(QStringLiteral("--- Remote Branches ---"), QVariant());
    for (const auto &branch : branches) {
        if (branch.type() & GIT_BRANCH_REMOTE) {
            QString name = branch.name();
            if (name.startsWith(QStringLiteral("refs/remotes/")))
                name = name.mid(13);
            ui->comboBox_logBranchFilter->addItem(name, branch.name());
        }
    }

    // Restore selection
    int savedIdx = ui->comboBox_logBranchFilter->findText(currentSavedText);
    if (savedIdx >= 0) {
        ui->comboBox_logBranchFilter->setCurrentIndex(savedIdx);
    } else {
        ui->comboBox_logBranchFilter->setCurrentIndex(1); // default to Current Branch (HEAD)
    }
    ui->comboBox_logBranchFilter->blockSignals(false);

    ui->logHistory_commits->setReferences(branches, tags, current_branch);
}

void QGitRepository::repositoryStashesReply(const QStringList &stashes, const QGitError &error)
{
    Q_UNUSED(error)
    ui->branchesTreeView->populateStashes(stashes);
}

void QGitRepository::repositoryChangedFilesReply(const QList<QPair<QString, git_status_t>> &files, const QGitError &error)
{
    Q_UNUSED(error)
    m_changedFiles = files;
    updateStatusViews();

    // Update stash button state: check if there are any staged or unstaged changes
    bool hasStashableChanges = false;
    for (const auto &filePair : files)
    {
        git_status_t status = filePair.second;
        if (status & (GIT_STATUS_INDEX_NEW | GIT_STATUS_INDEX_MODIFIED | GIT_STATUS_INDEX_DELETED | 
                      GIT_STATUS_INDEX_RENAMED | GIT_STATUS_INDEX_TYPECHANGE | 
                      GIT_STATUS_WT_MODIFIED | GIT_STATUS_WT_DELETED | 
                      GIT_STATUS_WT_RENAMED | GIT_STATUS_WT_TYPECHANGE | 
                      GIT_STATUS_CONFLICTED))
        {
            hasStashableChanges = true;
            break;
        }
    }

    emit setStashEnabledRequested(hasStashableChanges);
    emit refreshRepositoryTreeRequested();
}

void QGitRepository::updateStatusViews()
{
    bool noStaging = (ui->stackedWidget_staging->currentIndex() == 1);

    auto layoutMode = static_cast<QGitFileStatusTreeWidget::ViewLayoutMode>(m_layoutOption);
    ui->treeWidget_staged->setViewLayoutMode(layoutMode);
    ui->treeWidget_unstaged->setViewLayoutMode(layoutMode);
    ui->treeWidget_pending->setViewLayoutMode(layoutMode);

    int stagedCnt = 0;
    for (const auto &filePair : m_changedFiles) {
        if (filePair.second & (GIT_STATUS_INDEX_NEW | GIT_STATUS_INDEX_MODIFIED | GIT_STATUS_INDEX_DELETED | GIT_STATUS_INDEX_RENAMED | GIT_STATUS_INDEX_TYPECHANGE)) {
            stagedCnt++;
        }
    }

    if (noStaging)
    {
        ui->treeWidget_pending->populateFiles(m_changedFiles, false, true);

        if (layoutMode == QGitFileStatusTreeWidget::TreeView)
        {
            for (int i = 0; i < ui->treeWidget_pending->topLevelItemCount(); ++i)
            {
                ui->treeWidget_pending->updateFolderCheckStates(ui->treeWidget_pending->topLevelItem(i));
            }
        }

        ui->checkBox_PendingFiles->blockSignals(true);
        ui->checkBox_PendingFiles->setText(tr("Pending files (%1/%2 files)").arg(stagedCnt).arg(m_changedFiles.count()));
        if (stagedCnt == m_changedFiles.count() && m_changedFiles.count() > 0)
        {
            ui->checkBox_PendingFiles->setCheckState(Qt::Checked);
        }
        else if (stagedCnt == 0)
        {
            ui->checkBox_PendingFiles->setCheckState(Qt::Unchecked);
        }
        else
        {
            ui->checkBox_PendingFiles->setCheckState(Qt::PartiallyChecked);
        }
        ui->checkBox_PendingFiles->blockSignals(false);
    }
    else
    {
        ui->treeWidget_staged->populateFiles(m_changedFiles, true, false);
        ui->treeWidget_unstaged->populateFiles(m_changedFiles, false, false);
    }

    ui->commit_diff->refresh();

    if (stagedCnt > 0 || ui->checkBox_amendCommit->isChecked())
    {
        ui->pushButton_commit->setEnabled(true);
    }
    else
    {
        ui->pushButton_commit->setEnabled(false);
    }
}

void QGitRepository::repositoryStageFilesReply(const QGitError &error)
{
    Q_UNUSED(error)

    fetchRepositoryChangedFiles();
}

void QGitRepository::repositoryUnstageFilesReply(const QGitError &error)
{
    Q_UNUSED(error)

    fetchRepositoryChangedFiles();
}

void QGitRepository::repositoryDiscardFilesReply(const QGitError &error)
{
    if (error.errorCode() != 0) {
        QMessageBox::critical(this, tr("Discard Error"), error.errorString());
    }
    fetchRepositoryChangedFiles();
}

void QGitRepository::repositoryCommitReply(const QString &commit_id, const QGitError &error)
{
    Q_UNUSED(commit_id)

    ui->plainTextEdit_commitMessage->setEnabled(true);
    ui->pushButton_commit->setEnabled(true);

    if (error.errorCode() != 0) {
        QMessageBox::critical(this, tr("Commit Error"), error.errorString());
        // A post-commit push can fail after the local commit has succeeded.
        refreshData();
    } else {
        ui->plainTextEdit_commitMessage->clear();
        ui->checkBox_amendCommit->setChecked(false);
        m_draftCommitMessage.clear();
        refreshData();
    }
}

void QGitRepository::repositoryGetCommitsReply(const QList<QGitCommit> &commits, const QGitError &error)
{
    Q_UNUSED(error)

    for(const auto &commit: commits)
    {
        ui->logHistory_commits->addCommit(commit);
    }

    m_logCommitsOffset += commits.count();

    if (commits.count() < COMMIT_COUNT_TO_LOAD)
    {
        m_allCommitsLoaded = true;
    }

    if (!m_searchingCommitHash.isEmpty())
    {
        if (ui->logHistory_commits->selectCommit(m_searchingCommitHash)) {
            m_searchingCommitHash = "";
            emit clearStatusMessage();
        } else {
            if (m_allCommitsLoaded) {
                QMessageBox::information(this, tr("Commit not found"), 
                                         tr("The commit [%1] was not found in the repository history.").arg(m_searchingCommitHash.left(10)));
                m_searchingCommitHash = "";
                emit clearStatusMessage();
            } else {
                fetchCommits();
                emit statusMessage(tr("Searching for commit %1...").arg(m_searchingCommitHash.left(10)));
            }
        }
    }
    else
    {
        emit clearStatusMessage();
    }
}

void QGitRepository::repositoryGetCommitDiffReply(const QString &commitId, const QGitCommit &diff, const QGitError &error)
{
    if (error.errorCode())
    {
        return;
    }

    if (commitId == "HEAD")
    {
        // do nothing
    }
    if (commitId == "staged")
    {
        m_stagedDiff = diff;
    }
    else if (commitId == "unstaged")
    {
        m_unstagedDiff = diff;
    }
    else
    {
        m_commitDiff = diff;

        bool ignoreWhitespace = ui->comboBox_gitDiffOptions->ignoreWhitespace();

        if (ui->repositoryDetail->currentIndex() == 2)
        {
            int currentRow = ui->search_commits->currentRow();
            if (currentRow < 0) return;

            const QString commit_id = ui->search_commits->item(currentRow, 3)->data(Qt::UserRole).toString();
            const QString email = ui->search_commits->item(currentRow, 2)->data(Qt::UserRole + 1).toString();

            QString html;
            QStringList parentsHtml;
            QStringList labelsHtml;

            for (int i = 0; i < m_commitDiff.parents().count(); i++)
            {
                parentsHtml << QStringLiteral("<a href = \"") + m_commitDiff.parents().at(i).commitHash() + QStringLiteral("\">") + m_commitDiff.parents().at(i).commitHash().left(10) + QStringLiteral("</a>");
            }

            html += QStringLiteral("<div>");
            html += QStringLiteral("<img src=\"https://www.gravatar.com/avatar/") + QCryptographicHash::hash(email.trimmed().toUtf8(), QCryptographicHash::Md5).toHex() + QStringLiteral("?s=32\" width=\"32\" height=\"32\" style=\"float: right\" />");
            html += QStringLiteral("</div>");
            html += QStringLiteral("<b>Commit:</b> ") + commit_id + QStringLiteral("<br />");
            html += (parentsHtml.count() > 1? QStringLiteral("<b>Parents:</b>"): QStringLiteral("<b>Parent:</b>")) + parentsHtml.join(", ") + QStringLiteral("<br />");
            html += QStringLiteral("<b>Date:</b> ") + m_commitDiff.time().toString() + QStringLiteral("<br />");
            if (!m_commitDiff.description().isEmpty()) {
                html += QStringLiteral("<b>Describe:</b> ") + m_commitDiff.description() + QStringLiteral("<br />");
            }
            html += QStringLiteral("<b>Labels:</b> ") + labelsHtml.join(", ") + QStringLiteral("<br />");
            html += QStringLiteral("<br />");
            html += m_commitDiff.message().toHtmlEscaped().replace(QLatin1Char('\n'), QStringLiteral("<br />"));
            if (!m_commitDiff.note().isEmpty()) {
                html += QStringLiteral("<br /><br /><b>Notes:</b><br />") + m_commitDiff.note().toHtmlEscaped().replace(QLatin1Char('\n'), QStringLiteral("<br />"));
            }

            ui->search_info->setHtml(html);

            ui->search_files->clearContents();

            QMap<QString, QGitDiffFile> uniqueFiles;
            for (int i = 0; i < m_commitDiff.parents().count(); i++) {
                auto parentFiles = m_commitDiff.parents().at(i).files();
                for (const auto &f : parentFiles) {
                    QString path = f.new_file().path();
                    if (!uniqueFiles.contains(path)) {
                        uniqueFiles.insert(path, f);
                    }
                }
            }

            QList<QGitDiffFile> files = uniqueFiles.values();
            ui->search_files->setRowCount(files.count());

            for(int c = 0; c < files.count(); c++)
            {
                QString path = files.at(c).new_file().path();

                QString filename = QFileInfo(path).fileName();
                QString pathname = path.left(path.length() - filename.length());
                if ((pathname.endsWith('/'))||(pathname.endsWith('\\')))
                {
                    pathname.resize(pathname.length() - 1);
                }

                QIcon item_icon;

                switch(files.at(c).status()) {
                case GIT_DELTA_ADDED:
                    item_icon = m_iconFileNew;
                    break;
                case GIT_DELTA_DELETED:
                    item_icon = m_iconFileRemoved;
                    break;
                case GIT_DELTA_MODIFIED:
                    item_icon = m_iconFileModified;
                    break;
                default:
                    item_icon = m_iconFileUnknown;
                    break;
                }

                QTableWidgetItem *item = new QTableWidgetItem(item_icon, filename);
                item->setData(Qt::UserRole, path);
                ui->search_files->setItem(c, 0, item);

                item = new QTableWidgetItem(pathname);
                ui->search_files->setItem(c, 1, item);
            }

            if (ui->search_files->rowCount() > 0)
            {
                ui->search_files->setCurrentCell(0, 0);

                ui->search_diff->setIgnoreWhitespace(ignoreWhitespace);
                ui->search_diff->setGitDiff(m_commitDiff.parents().at(0).commitHash(), m_commitDiff.id(), {ui->search_files->item(0, 0)->data(Qt::UserRole).toString()});
            }
        }
        else
        {
            int currentRow = ui->logHistory_commits->currentRow();
            if (currentRow < 0) return;

            const QString commit_id = ui->logHistory_commits->item(currentRow, 4)->data(Qt::UserRole).toString();
            const QString email = ui->logHistory_commits->item(currentRow, 3)->data(Qt::UserRole + 1).toString();

            QString html;
            QStringList parentsHtml;
            QStringList labelsHtml;

            for (int i = 0; i < m_commitDiff.parents().count(); i++)
            {
                parentsHtml << QStringLiteral("<a href = \"") + m_commitDiff.parents().at(i).commitHash() + QStringLiteral("\">") + m_commitDiff.parents().at(i).commitHash().left(10) + QStringLiteral("</a>");
            }

            html += QStringLiteral("<div>");
            html += QStringLiteral("<img src=\"https://www.gravatar.com/avatar/") + QCryptographicHash::hash(email.trimmed().toUtf8(), QCryptographicHash::Md5).toHex() + QStringLiteral("?s=32\" width=\"32\" height=\"32\" style=\"float: right\" />");
            html += QStringLiteral("</div>");
            html += QStringLiteral("<b>Commit:</b> ") + commit_id + QStringLiteral("<br />");
            html += (parentsHtml.count() > 1? QStringLiteral("<b>Parents:</b>"): QStringLiteral("<b>Parent:</b>")) + parentsHtml.join(", ") + QStringLiteral("<br />");
            html += QStringLiteral("<b>Date:</b> ") + m_commitDiff.time().toString() + QStringLiteral("<br />");
            if (!m_commitDiff.description().isEmpty()) {
                html += QStringLiteral("<b>Describe:</b> ") + m_commitDiff.description() + QStringLiteral("<br />");
            }
            html += QStringLiteral("<b>Labels:</b> ") + labelsHtml.join(", ") + QStringLiteral("<br />");
            html += QStringLiteral("<br />");
            html += m_commitDiff.message().toHtmlEscaped().replace(QLatin1Char('\n'), QStringLiteral("<br />"));
            if (!m_commitDiff.note().isEmpty()) {
                html += QStringLiteral("<br /><br /><b>Notes:</b><br />") + m_commitDiff.note().toHtmlEscaped().replace(QLatin1Char('\n'), QStringLiteral("<br />"));
            }

            ui->logHistory_info->setHtml(html);

            ui->logHistory_files->clearContents();

            QMap<QString, QGitDiffFile> uniqueFiles;
            for (int i = 0; i < m_commitDiff.parents().count(); i++) {
                auto parentFiles = m_commitDiff.parents().at(i).files();
                for (const auto &f : parentFiles) {
                    QString path = f.new_file().path();
                    if (!uniqueFiles.contains(path)) {
                        uniqueFiles.insert(path, f);
                    }
                }
            }

            QList<QGitDiffFile> files = uniqueFiles.values();
            ui->logHistory_files->setRowCount(files.count());

            for(int c = 0; c < files.count(); c++)
            {
                QString path = files.at(c).new_file().path();

                QString filename = QFileInfo(path).fileName();
                QString pathname = path.left(path.length() - filename.length());
                if ((pathname.endsWith('/'))||(pathname.endsWith('\\')))
                {
                    pathname.resize(pathname.length() - 1);
                }

                QIcon item_icon;

                switch(files.at(c).status()) {
                case GIT_DELTA_ADDED:
                    item_icon = m_iconFileNew;
                    break;
                case GIT_DELTA_DELETED:
                    item_icon = m_iconFileRemoved;
                    break;
                case GIT_DELTA_MODIFIED:
                    item_icon = m_iconFileModified;
                    break;
                default:
                    item_icon = m_iconFileUnknown;
                    break;
                }

                QTableWidgetItem *item = new QTableWidgetItem(item_icon, filename);
                item->setData(Qt::UserRole, path);
                ui->logHistory_files->setItem(c, 0, item);

                item = new QTableWidgetItem(pathname);
                ui->logHistory_files->setItem(c, 1, item);
            }

            if (ui->logHistory_files->rowCount() > 0)
            {
                ui->logHistory_files->setCurrentCell(0, 0);

                ui->logHistory_diff->setIgnoreWhitespace(ignoreWhitespace);
                ui->logHistory_diff->setGitDiff(m_commitDiff.parents().at(0).commitHash(), m_commitDiff.id(), {ui->logHistory_files->item(0, 0)->data(Qt::UserRole).toString()});
            }
        }
    }
}

void QGitRepository::deleteBranchesReply(const QGitError &error)
{
    if (error.errorCode())
    {
        QMessageBox::critical(this, tr("Error deleting branches"), error.errorString());
    }
    emit repositoryBranches();
}

void QGitRepository::on_branchesTreeView_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column)
    if (!item) return;

    QString type = item->data(0, Qt::UserRole + 2).toString();
    if (type == "Submodule")
    {
        QString subPath = item->data(0, Qt::UserRole + 1).toString();
        QString fullSubPath = m_git->path().absoluteFilePath(subPath);
        emit openRepositoryRequested(fullSubPath);
        return;
    }

    QString hash = item->data(0, Qt::UserRole).toString();
    if (!hash.isEmpty())
    {
        ui->repositoryDetail->setCurrentWidget(ui->tabLogHistory);
        if (!ui->logHistory_commits->selectCommit(hash))
        {
            if (m_allCommitsLoaded) {
                 QMessageBox::information(this, tr("Commit not found"), 
                                          tr("The commit [%1] was not found in the repository history.").arg(hash.left(10)));
            } else {
                m_searchingCommitHash = hash;
                fetchCommits();
                emit statusMessage(tr("Searching for commit %1...").arg(hash.left(10)));
            }
        }
    }
}

void QGitRepository::on_branchesTreeView_customContextMenuRequested(const QPoint &pos)
{
    QTreeWidgetItem *item = ui->branchesTreeView->itemAt(pos);
    if (!item)
        return;

    QString type = item->data(0, Qt::UserRole + 2).toString();
    QString fullName = item->data(0, Qt::UserRole + 1).toString();

    if (type == "WorkingCopy")
    {
        QMenu menu(this);
        QAction *applyPatchAction = menu.addAction(tr("Apply Patch..."));
        QAction *cleanAction = menu.addAction(tr("Clean Working Directory..."));
        menu.addSeparator();
        QAction *showReflogAction = menu.addAction(tr("Show Reference Log (Reflog)..."));
        QAction *showBlameAction  = menu.addAction(tr("Browse File Blame..."));

        QAction *selectedAction = menu.exec(ui->branchesTreeView->viewport()->mapToGlobal(pos));
        if (selectedAction == applyPatchAction)
        {
            QString patchPath = QFileDialog::getOpenFileName(
                this,
                tr("Select Patch File"),
                QString(),
                tr("Patch/Diff Files (*.patch *.diff);;All Files (*)")
            );
            if (!patchPath.isEmpty())
            {
                emit statusMessage(tr("Applying patch..."));
                emit repositoryApplyPatch(patchPath);
            }
        }
        else if (selectedAction == cleanAction)
        {
            QGitCleanDialog dlg(this);
            if (dlg.exec() == QDialog::Accepted) {
                emit statusMessage(tr("Cleaning working directory..."));
                emit repositoryClean(dlg.removeIgnored(), dlg.removeDirectories());
            }
        }
        else if (selectedAction == showReflogAction)
        {
            QGitReflogDialog dlg(QStringLiteral("HEAD"), this, this);
            dlg.exec();
        }
        else if (selectedAction == showBlameAction)
        {
            QGitBlameDialog dlg(m_git, this);
            connect(&dlg, &QGitBlameDialog::commitNavigationRequested,
                    this, &QGitRepository::navigateToCommit);
            dlg.exec();
        }
        return;
    }

    if (type == "RemotesHeader")
    {
        QMenu menu(this);
        QAction *manageAction = menu.addAction(tr("Manage Remotes..."));

        QAction *selectedAction = menu.exec(ui->branchesTreeView->viewport()->mapToGlobal(pos));
        if (selectedAction == manageAction)
        {
            QGitRemotesDialog dlg(this, this);
            dlg.exec();
        }
        return;
    }

    if (type == "WorktreesHeader")
    {
        QMenu menu(this);
        QAction *addAction = menu.addAction(tr("Add Worktree..."));

        QAction *selectedAction = menu.exec(ui->branchesTreeView->viewport()->mapToGlobal(pos));
        if (selectedAction == addAction)
        {
            worktreeDialog();
        }
        return;
    }

    if (type == "Worktree")
    {
        QString wtName   = item->data(0, Qt::UserRole + 3).toString();
        QString wtPath   = item->data(0, Qt::UserRole).toString();
        bool    wtLocked = item->data(0, Qt::UserRole + 4).toBool();

        QMenu menu(this);
        QAction *openAction   = menu.addAction(tr("Open in New Tab"));
        menu.addSeparator();
        QAction *lockAction   = menu.addAction(wtLocked ? tr("Unlock Worktree") : tr("Lock Worktree"));
        menu.addSeparator();
        QAction *removeAction = menu.addAction(tr("Remove Worktree (Prune)"));

        QAction *selectedAction = menu.exec(ui->branchesTreeView->viewport()->mapToGlobal(pos));
        if (selectedAction == openAction)
        {
            emit openRepositoryRequested(wtPath);
        }
        else if (selectedAction == lockAction)
        {
            try {
                m_git->lockWorktree(wtName, !wtLocked);
                refreshData();
            } catch (const QGitError &error) {
                QMessageBox::critical(this, tr("Worktree Error"), error.errorString());
            }
        }
        else if (selectedAction == removeAction)
        {
            auto res = QMessageBox::question(
                this, tr("Remove Worktree"),
                tr("Remove worktree '%1'? This will prune its Git metadata. "
                   "The directory itself will not be deleted.").arg(wtName),
                QMessageBox::Yes | QMessageBox::No);
            if (res == QMessageBox::Yes) {
                try {
                    m_git->removeWorktree(wtName);
                    refreshData();
                } catch (const QGitError &error) {
                    QMessageBox::critical(this, tr("Worktree Error"), error.errorString());
                }
            }
        }
        return;
    }

    if (type == "Submodule")
    {
        QString subName = item->data(0, Qt::UserRole).toString();
        QString subPath = item->data(0, Qt::UserRole + 1).toString();

        QMenu menu(this);
        QAction *openAction = menu.addAction(tr("Open Submodule Repository"));
        menu.addSeparator();
        QAction *updateAction = menu.addAction(tr("Update Submodule"));
        QAction *initAction = menu.addAction(tr("Initialize Submodule"));
        QAction *syncAction = menu.addAction(tr("Sync URL"));

        QAction *selectedAction = menu.exec(ui->branchesTreeView->viewport()->mapToGlobal(pos));
        if (selectedAction == openAction)
        {
            QString fullSubPath = m_git->path().absoluteFilePath(subPath);
            emit openRepositoryRequested(fullSubPath);
        }
        else if (selectedAction == updateAction)
        {
            emit statusMessage(tr("Updating submodule %1...").arg(subName));
            emit repositoryUpdateSubmodule(subName);
        }
        else if (selectedAction == initAction)
        {
            try {
                m_git->initSubmodule(subName);
                refreshData();
                QMessageBox::information(this, tr("Submodule Initialized"), tr("Submodule '%1' initialized successfully.").arg(subName));
            } catch (const QGitError &error) {
                QMessageBox::critical(this, tr("Error"), error.errorString());
            }
        }
        else if (selectedAction == syncAction)
        {
            try {
                m_git->syncSubmodule(subName);
                QMessageBox::information(this, tr("Submodule Synced"), tr("Submodule '%1' URL synchronized successfully.").arg(subName));
            } catch (const QGitError &error) {
                QMessageBox::critical(this, tr("Error"), error.errorString());
            }
        }
        return;
    }

    if (type == "Stash")
    {
        QMenu menu(this);
        QAction *applyAction = menu.addAction(tr("Apply Stash"));
        QAction *popAction = menu.addAction(tr("Pop Stash"));
        menu.addSeparator();
        QAction *dropAction = menu.addAction(tr("Drop Stash"));

        QAction *selectedAction = menu.exec(ui->branchesTreeView->viewport()->mapToGlobal(pos));
        if (selectedAction == applyAction)
        {
            emit statusMessage(tr("Applying stash %1...").arg(fullName));
            m_git->stashApply(fullName);
        }
        else if (selectedAction == popAction)
        {
            emit statusMessage(tr("Popping stash %1...").arg(fullName));
            m_git->stashPop(fullName);
        }
        else if (selectedAction == dropAction)
        {
            auto res = QMessageBox::question(this, tr("Drop Stash"), 
                                             tr("Are you sure you want to drop stash '%1'?").arg(fullName),
                                             QMessageBox::Yes | QMessageBox::No);
            if (res == QMessageBox::Yes)
            {
                emit statusMessage(tr("Dropping stash %1...").arg(fullName));
                m_git->stashRemove(fullName);
            }
        }
    }
    else if (type == "LocalBranch")
    {
        QMenu menu(this);
        QAction *checkoutAction = menu.addAction(tr("Checkout Branch"));
        QAction *renameAction = menu.addAction(tr("Rename Branch..."));
        QAction *setUpstreamAction = menu.addAction(tr("Set Upstream..."));
        menu.addSeparator();
        QAction *showReflogAction = menu.addAction(tr("Show Reference Log (Reflog)..."));
        menu.addSeparator();
        QAction *deleteAction = menu.addAction(tr("Delete Branch"));

        if (item->font(0).bold()) {
            checkoutAction->setEnabled(false);
            deleteAction->setEnabled(false);
        }

        QAction *selectedAction = menu.exec(ui->branchesTreeView->viewport()->mapToGlobal(pos));
        if (selectedAction == checkoutAction)
        {
            emit statusMessage(tr("Checking out branch %1...").arg(fullName));
            m_git->checkoutBranch(fullName);
        }
        else if (selectedAction == renameAction)
        {
            ui->branchesTreeView->editItem(item, 0);
        }
        else if (selectedAction == setUpstreamAction)
        {
            QString upstream = QInputDialog::getText(this, tr("Set Upstream"), tr("Enter upstream for branch %1:").arg(fullName));
            if (!upstream.isEmpty()) {
                m_git->setUpstream(fullName, upstream);
            }
        }
        else if (selectedAction == showReflogAction)
        {
            QGitReflogDialog dlg(fullName, this, this);
            dlg.exec();
        }
        else if (selectedAction == deleteAction)
        {
            auto res = QMessageBox::question(this, tr("Delete Branch"), 
                                             tr("Are you sure you want to delete branch '%1'?").arg(fullName),
                                             QMessageBox::Yes | QMessageBox::No);
            if (res == QMessageBox::Yes) {
                QList<QGitBranch> toDelete;
                toDelete << QGitBranch(fullName, "", 0, GIT_BRANCH_LOCAL);
                m_git->deleteBranches(toDelete, false);
            }
        }
    }
    else if (type == "RemoteBranch")
    {
        QMenu menu(this);
        QAction *checkoutAction = menu.addAction(tr("Checkout as Local Branch..."));
        menu.addSeparator();
        QAction *showReflogAction = menu.addAction(tr("Show Reference Log (Reflog)..."));
        
        QAction *selectedAction = menu.exec(ui->branchesTreeView->viewport()->mapToGlobal(pos));
        if (selectedAction == checkoutAction)
        {
            QString localName = fullName;
            if (localName.contains('/'))
                localName = localName.section('/', 1); // Remove origin/

            bool ok;
            QString name = QInputDialog::getText(this, tr("Checkout Remote Branch"), 
                                                 tr("Enter local branch name:"), QLineEdit::Normal, 
                                                  localName, &ok);
            if (ok && !name.isEmpty()) {
                m_git->checkoutBranch(fullName); // Simplified for now
            }
        }
        else if (selectedAction == showReflogAction)
        {
            QGitReflogDialog dlg(fullName, this, this);
            dlg.exec();
        }
    }
    else if (type == "Tag")
    {
        QMenu menu(this);
        QAction *renameAction = menu.addAction(tr("Rename Tag"));
        QAction *deleteAction = menu.addAction(tr("Delete Tag"));
        
        QAction *selectedAction = menu.exec(ui->branchesTreeView->viewport()->mapToGlobal(pos));
        if (selectedAction == renameAction)
        {
            ui->branchesTreeView->editItem(item, 0);
        }
        else if (selectedAction == deleteAction)
        {
            auto res = QMessageBox::question(this, tr("Delete Tag"), 
                                             tr("Are you sure you want to delete tag '%1'?").arg(fullName),
                                             QMessageBox::Yes | QMessageBox::No);
            if (res == QMessageBox::Yes) {
                m_git->deleteTag(fullName);
            }
        }
    }
}

void QGitRepository::checkoutBranchReply(const QGitError &error)
{
    if (error.errorCode())
    {
        QMessageBox::critical(this, tr("Checkout Error"), error.errorString());
    }
    refreshData();
}

void QGitRepository::renameBranchReply(const QGitError &error)
{
    if (error.errorCode())
    {
        QMessageBox::critical(this, tr("Rename Error"), error.errorString());
    }
    refreshData();
}

void QGitRepository::renameTagReply(const QGitError &error)
{
    if (error.errorCode())
    {
        QMessageBox::critical(this, tr("Rename Tag Error"), error.errorString());
    }
    refreshData();
}

void QGitRepository::setUpstreamReply(const QGitError &error)
{
    if (error.errorCode())
    {
        QMessageBox::critical(this, tr("Set Upstream Error"), error.errorString());
    }
    refreshData();
}

void QGitRepository::stashApplyReply(const QGitError &error)
{
    if (error.errorCode())
    {
        QMessageBox::critical(this, tr("Stash Apply Error"), 
                              tr("Could not apply stash. This might be due to merge conflicts or local changes.\n\nDetails: %1").arg(error.errorString()));
        emit statusMessage(tr("Stash apply failed."));
    }
    else
    {
        emit statusMessage(tr("Stash applied successfully."));
        refreshData();
    }
}

void QGitRepository::stashPopReply(const QGitError &error)
{
    if (error.errorCode())
    {
        QMessageBox::critical(this, tr("Stash Pop Error"), 
                              tr("Could not pop stash. Changes might have conflicts with your working copy.\n\nDetails: %1").arg(error.errorString()));
        emit statusMessage(tr("Stash pop failed."));
    }
    else
    {
        emit statusMessage(tr("Stash popped successfully."));
        refreshData();
    }
}

void QGitRepository::on_repositoryDetail_currentChanged(int index)
{
    disconnect(ui->commit_diff, &QGitDiffWidget::requestGitDiff, m_git, &QGit::commitDiffContent);
    disconnect(m_git, &QGit::commitDiffContentReply, ui->commit_diff, &QGitDiffWidget::responseGitDiff);

    disconnect(ui->logHistory_diff, &QGitDiffWidget::requestGitDiff, m_git, &QGit::commitDiffContent);
    disconnect(m_git, &QGit::commitDiffContentReply, ui->logHistory_diff, &QGitDiffWidget::responseGitDiff);

    disconnect(ui->search_diff, &QGitDiffWidget::requestGitDiff, m_git, &QGit::commitDiffContent);
    disconnect(m_git, &QGit::commitDiffContentReply, ui->search_diff, &QGitDiffWidget::responseGitDiff);

    switch(index) {
    case 0:
        connect(ui->commit_diff, &QGitDiffWidget::requestGitDiff, m_git, &QGit::commitDiffContent);
        connect(m_git, &QGit::commitDiffContentReply, ui->commit_diff, &QGitDiffWidget::responseGitDiff);

        fetchRepositoryChangedFiles();
        break;
    case 1:
        connect(ui->logHistory_diff, &QGitDiffWidget::requestGitDiff, m_git, &QGit::commitDiffContent);
        connect(m_git, &QGit::commitDiffContentReply, ui->logHistory_diff, &QGitDiffWidget::responseGitDiff);

        fetchCommits();
        break;
    case 2:
        connect(ui->search_diff, &QGitDiffWidget::requestGitDiff, m_git, &QGit::commitDiffContent);
        connect(m_git, &QGit::commitDiffContentReply, ui->search_diff, &QGitDiffWidget::responseGitDiff);
        break;
    default:
        Q_UNIMPLEMENTED();
        break;
    }
}

void QGitRepository::on_checkBox_StagedFiles_clicked()
{
    QStringList items;

    ui->checkBox_StagedFiles->setChecked(true);

    for (const auto &pair : m_changedFiles)
    {
        git_status_t status = pair.second;
        if (status & (GIT_STATUS_INDEX_NEW | GIT_STATUS_INDEX_MODIFIED | GIT_STATUS_INDEX_DELETED | GIT_STATUS_INDEX_RENAMED | GIT_STATUS_INDEX_TYPECHANGE))
        {
            items.append(pair.first);
        }
    }

    if (items.count() > 0)
    {
        emit repositoryUnstageFiles(items);
    }
}

void QGitRepository::on_checkBox_UnstagedFiles_clicked()
{
    QStringList items;

    ui->checkBox_UnstagedFiles->setChecked(false);

    for (const auto &pair : m_changedFiles)
    {
        git_status_t status = pair.second;
        if ((status == GIT_STATUS_CURRENT) || (status & (GIT_STATUS_WT_NEW | GIT_STATUS_WT_MODIFIED | GIT_STATUS_WT_DELETED | GIT_STATUS_WT_TYPECHANGE | GIT_STATUS_WT_RENAMED | GIT_STATUS_WT_UNREADABLE | GIT_STATUS_IGNORED | GIT_STATUS_CONFLICTED)))
        {
            items.append(pair.first);
        }
    }

    if (items.count() > 0)
    {
        emit repositoryStageFiles(items);
    }
}

void QGitRepository::on_checkBox_PendingFiles_clicked()
{
    QStringList items;
    bool shouldStage = (ui->checkBox_PendingFiles->checkState() != Qt::Unchecked);

    for (const auto &pair : m_changedFiles)
    {
        items.append(pair.first);
    }

    if (!items.isEmpty())
    {
        if (shouldStage)
        {
            emit repositoryStageFiles(items);
        }
        else
        {
            emit repositoryUnstageFiles(items);
        }
    }
}

void QGitRepository::on_treeWidget_staged_itemChanged(QTreeWidgetItem *item, int column)
{
    if (column != 0) return;
    QString file = item->data(0, Qt::UserRole).toString();
    if (file.isEmpty()) return;

    if (item->checkState(0) == Qt::Unchecked)
    {
        emit repositoryUnstageFiles({file});
    }
}

void QGitRepository::on_treeWidget_unstaged_itemChanged(QTreeWidgetItem *item, int column)
{
    if (column != 0) return;
    QString file = item->data(0, Qt::UserRole).toString();
    if (file.isEmpty()) return;

    if (item->checkState(0) == Qt::Checked)
    {
        emit repositoryStageFiles({file});
    }
}

void QGitRepository::on_treeWidget_pending_itemChanged(QTreeWidgetItem *item, int column)
{
    if (column != 0) return;

    QString file = item->data(0, Qt::UserRole).toString();
    if (file.isEmpty())
    {
        ui->treeWidget_pending->blockSignals(true);
        QStringList paths;
        collectFilePaths(item, paths);

        Qt::CheckState state = item->checkState(0);
        auto propagate = [&](auto self, QTreeWidgetItem *it, Qt::CheckState st) -> void {
            for (int i = 0; i < it->childCount(); ++i) {
                QTreeWidgetItem *child = it->child(i);
                child->setCheckState(0, st);
                self(self, child, st);
            }
        };
        propagate(propagate, item, state);

        ui->treeWidget_pending->blockSignals(false);

        if (state == Qt::Checked)
        {
            emit repositoryStageFiles(paths);
        }
        else if (state == Qt::Unchecked)
        {
            emit repositoryUnstageFiles(paths);
        }
        return;
    }

    if (item->checkState(0) == Qt::Checked)
    {
        emit repositoryStageFiles({file});
    }
    else if (item->checkState(0) == Qt::Unchecked)
    {
        emit repositoryUnstageFiles({file});
    }
}

void QGitRepository::on_pushButton_commit_clicked()
{
    ui->plainTextEdit_commitMessage->setEnabled(false);
    ui->pushButton_commit->setEnabled(false);

    bool withPush = ui->checkBox_pushChangesImmidietely->isChecked();
    bool amend = ui->checkBox_amendCommit->isChecked();

    emit repositoryCommit(ui->plainTextEdit_commitMessage->toPlainText(), withPush, amend);
}

void QGitRepository::on_pushButton_commitCancel_clicked()
{
    activateCommitOperation(false);
}

void QGitRepository::on_checkBox_amendCommit_clicked(bool checked)
{
    if (checked)
    {
        m_draftCommitMessage = ui->plainTextEdit_commitMessage->toPlainText();
        QString headMessage = m_git->headCommitMessage();
        ui->plainTextEdit_commitMessage->setPlainText(headMessage);
        ui->pushButton_commit->setEnabled(true);
    }
    else
    {
        ui->plainTextEdit_commitMessage->setPlainText(m_draftCommitMessage);
        m_draftCommitMessage.clear();
        if (ui->treeWidget_staged->topLevelItemCount() > 0)
        {
            ui->pushButton_commit->setEnabled(true);
        }
        else
        {
            ui->pushButton_commit->setEnabled(false);
        }
    }
}

void QGitRepository::historyTableSliderMoved(int pos)
{
    if (pos == ui->logHistory_commits->verticalScrollBar()->maximum())
    {
        int rows = ui->logHistory_commits->rowCount();

        if (rows > 0)
        {
            fetchCommits();
        }
    }
}

void QGitRepository::selectedLines(const QString &filename, const QVector<QGitDiffWidgetLine> &lines)
{
    if (lines.isEmpty())
    {
        return;
    }

    if (m_stageingFiles)
    {
        emit stageFileLines(filename, lines);
    }
    else
    {
        emit unstageFileLines(filename, lines);
    }
}

void QGitRepository::on_logHistory_commits_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    Q_UNUSED(currentColumn)
    Q_UNUSED(previousRow)
    Q_UNUSED(previousColumn)

    QString commit_id;
    int rows = 0;

    if (currentRow < 0)
    {
        return;
    }

    commit_id = ui->logHistory_commits->item(currentRow, 4)->data(Qt::UserRole).toString();

    if (!commit_id.isEmpty())
    {
        emit repositoryGetCommitDiff(commit_id, ui->comboBox_gitDiffOptions->ignoreWhitespace());
    }

    rows = ui->logHistory_commits->rowCount();

    if (currentRow == rows - 1)
    {
        fetchCommits();
    }
}

void QGitRepository::on_plainTextEdit_commitMessage_focus()
{
    activateCommitOperation(true);
}

void QGitRepository::fetchCommits()
{
    if (!m_allCommitsLoaded)
    {
        emit repositoryGetCommits(m_currentLogBranchFilter, m_logCommitsOffset, COMMIT_COUNT_TO_LOAD);
        emit statusMessage(tr("Fetching commits..."));
    }
}

void QGitRepository::activateCommitOperation(bool activate)
{
    if (activate)
    {
        ui->horizontalWidgetLayout_1->show();
        ui->horizontalWidgetLayout_2->show();
        ui->plainTextEdit_commitMessage->setPlaceholderText("");
        ui->plainTextEdit_commitMessage->setFixedHeight(26 * 4);
    }
    else
    {
        ui->horizontalWidgetLayout_1->hide();
        ui->horizontalWidgetLayout_2->hide();
        ui->plainTextEdit_commitMessage->setPlainText("");
        ui->plainTextEdit_commitMessage->setPlaceholderText(tr("Commit message"));
        ui->plainTextEdit_commitMessage->setMinimumHeight(0);
        ui->plainTextEdit_commitMessage->setMaximumHeight(26);
        ui->label_signatureGravatarImage->setFocus();
    }
}

void QGitRepository::on_logHistory_files_itemSelectionChanged()
{
    bool ignoreWhitespace = ui->comboBox_gitDiffOptions->ignoreWhitespace();
    QTableWidgetItem *item = ui->logHistory_files->currentItem();

    if (item && item->column() == 0)
    {
        ui->logHistory_diff->setIgnoreWhitespace(ignoreWhitespace);
        ui->logHistory_diff->setGitDiff(m_commitDiff.parents().at(0).commitHash(), m_commitDiff.id(), {item->data(Qt::UserRole).toString()});
    }
}

static void openBlameForFileItem(QTableWidgetItem *item, QGit *git,
                                  QGitRepository *repo, const QString &commitId,
                                  QWidget *parent)
{
    if (!item) return;
    // Column 0 holds the relative file path in UserRole
    QTableWidgetItem *col0 = item->tableWidget()->item(item->row(), 0);
    if (!col0) return;
    QString relPath = col0->data(Qt::UserRole).toString();
    if (relPath.isEmpty()) return;

    QGitBlameDialog *dlg = new QGitBlameDialog(git, parent);
    dlg->setAttribute(Qt::WA_DeleteOnClose);

    // Pre-select the file automatically
    dlg->preSelectFile(relPath, commitId);

    QObject::connect(dlg, &QGitBlameDialog::commitNavigationRequested,
                     repo, &QGitRepository::navigateToCommit);
    dlg->show();
}

void QGitRepository::on_logHistory_files_customContextMenuRequested(const QPoint &pos)
{
    QTableWidgetItem *item = ui->logHistory_files->itemAt(pos);
    if (!item) return;

    QMenu menu(this);
    QAction *blameAction = menu.addAction(tr("Show Blame at this Commit..."));
    QAction *selected = menu.exec(ui->logHistory_files->viewport()->mapToGlobal(pos));
    if (selected == blameAction) {
        openBlameForFileItem(item, m_git, this, m_commitDiff.id(), this);
    }
}

void QGitRepository::on_search_files_customContextMenuRequested(const QPoint &pos)
{
    QTableWidgetItem *item = ui->search_files->itemAt(pos);
    if (!item) return;

    QMenu menu(this);
    QAction *blameAction = menu.addAction(tr("Show Blame at this Commit..."));
    QAction *selected = menu.exec(ui->search_files->viewport()->mapToGlobal(pos));
    if (selected == blameAction) {
        openBlameForFileItem(item, m_git, this, m_commitDiff.id(), this);
    }
}

void QGitRepository::on_comboBox_gitDiffOptions_optionsChanged()
{
    bool ignoreWhitespace = ui->comboBox_gitDiffOptions->ignoreWhitespace();
    int lines = ui->comboBox_gitDiffOptions->linesOfContent();

    ui->logHistory_diff->setIgnoreWhitespace(ignoreWhitespace);
    ui->logHistory_diff->setLinesOfContent(lines);

    ui->commit_diff->setIgnoreWhitespace(ignoreWhitespace);
    ui->commit_diff->setLinesOfContent(lines);

    if (ui->repositoryDetail->currentWidget() == ui->tabLogHistory) {
        ui->logHistory_diff->refresh();
    } else if (ui->repositoryDetail->currentWidget() == ui->tabFileStatus) {
        ui->commit_diff->refresh();
    }
}

void QGitRepository::on_treeWidget_staged_itemSelectionChanged()
{
    QList<QString> files;
    QMap<QString, git_status_t> statuses;

    if (ui->treeWidget_staged->isActiveWindow())
    {
        const auto &staged = ui->treeWidget_staged->selectedItems();
        for(auto item: staged) {
            QString file = item->data(0, Qt::UserRole).toString();
            if (file.isEmpty()) continue;
            files << file;
            statuses.insert(file, (git_status_t)item->data(0, Qt::UserRole + 1).toInt());
        }

        ui->commit_diff->setGitDiff("", "staged", files, statuses);

        ui->treeWidget_unstaged->blockSignals(true);
        for(auto item: ui->treeWidget_unstaged->selectedItems())
        {
            item->setSelected(false);
        }
        ui->treeWidget_unstaged->blockSignals(false);
    }

    m_stageingFiles = false;
}

void QGitRepository::on_treeWidget_unstaged_itemSelectionChanged()
{
    QList<QString> files;
    QMap<QString, git_status_t> statuses;

    if (ui->treeWidget_unstaged->isActiveWindow())
    {
        const auto &unstaged = ui->treeWidget_unstaged->selectedItems();
        for(const auto &item: unstaged) {
            QString file = item->data(0, Qt::UserRole).toString();
            if (file.isEmpty()) continue;
            files << file;
            statuses.insert(file, (git_status_t)item->data(0, Qt::UserRole + 1).toInt());
        }

        ui->commit_diff->setGitDiff("", "unstaged", files, statuses);

        ui->treeWidget_staged->blockSignals(true);
        for(auto item: ui->treeWidget_staged->selectedItems())
        {
            item->setSelected(false);
        }
        ui->treeWidget_staged->blockSignals(false);
    }

    m_stageingFiles = true;
}

void QGitRepository::on_treeWidget_pending_itemSelectionChanged()
{
    QList<QString> files;
    QMap<QString, git_status_t> statuses;

    if (ui->treeWidget_pending->isActiveWindow())
    {
        const auto &pending = ui->treeWidget_pending->selectedItems();
        for (auto item : pending) {
            QString file = item->data(0, Qt::UserRole).toString();
            if (file.isEmpty()) continue;
            files << file;
            statuses.insert(file, (git_status_t)item->data(0, Qt::UserRole + 1).toInt());
        }

        QString mode = "unstaged";
        if (!pending.isEmpty())
        {
            QTreeWidgetItem *first = pending.first();
            if (first->checkState(0) == Qt::Checked)
            {
                mode = "staged";
            }
        }

        ui->commit_diff->setGitDiff("", mode, files, statuses);
        m_stageingFiles = (mode == "unstaged");
    }
}

void QGitRepository::on_treeWidget_unstaged_customContextMenuRequested(const QPoint &pos)
{
    const auto &selected = ui->treeWidget_unstaged->selectedItems();
    QList<QTreeWidgetItem*> selectedFiles;
    for (auto item : selected) {
        if (!item->data(0, Qt::UserRole).toString().isEmpty()) {
            selectedFiles << item;
        }
    }

    QMenu menu(this);
    QAction *resolveAction = nullptr;
    QAction *discardAction = nullptr;
    QAction *ignoreAction = nullptr;

    if (!selectedFiles.isEmpty()) {
        uint32_t status = selectedFiles.first()->data(0, Qt::UserRole + 1).toUInt();
        bool hasConflict = (status & GIT_STATUS_CONFLICTED);
        bool isUntracked = (status & GIT_STATUS_WT_NEW);

        if (hasConflict) {
            resolveAction = menu.addAction(tr("Resolve Conflicts..."));
        }
        discardAction = menu.addAction(tr("Discard changes"));

        if (isUntracked && selectedFiles.size() == 1) {
            menu.addSeparator();
            ignoreAction = menu.addAction(tr("Add to .gitignore..."));
        }
        menu.addSeparator();
    }

    QAction *cleanAction = menu.addAction(tr("Clean Working Directory..."));

    QAction *res = menu.exec(ui->treeWidget_unstaged->mapToGlobal(pos));
    if (resolveAction && res == resolveAction) {
        QGitConflictResolverDialog dlg(m_git, selectedFiles.first()->data(0, Qt::UserRole).toString(), this);
        if (dlg.exec() == QDialog::Accepted) {
            refreshData();
        }
    } else if (discardAction && res == discardAction) {
        auto confirm = QMessageBox::question(this, tr("Discard changes"),
                                             tr("Are you sure you want to discard changes in the selected files? This operation cannot be undone."),
                                             QMessageBox::Yes | QMessageBox::No);
        if (confirm == QMessageBox::Yes) {
            QStringList files;
            for (auto item : selectedFiles) {
                files << item->data(0, Qt::UserRole).toString();
            }
            emit repositoryDiscardFiles(files);
        }
    } else if (ignoreAction && res == ignoreAction) {
        QString filePath = selectedFiles.first()->data(0, Qt::UserRole).toString();
        bool ok = false;
        QString pattern = QInputDialog::getText(
            this,
            tr("Add to .gitignore"),
            tr("Pattern to ignore:"),
            QLineEdit::Normal,
            filePath,
            &ok);
        if (ok && !pattern.trimmed().isEmpty()) {
            QString gitignorePath = m_path + "/.gitignore";
            QFile file(gitignorePath);
            if (file.open(QIODevice::ReadWrite | QIODevice::Text)) {
                if (file.size() > 0) {
                    file.seek(file.size() - 1);
                    char lastChar;
                    file.getChar(&lastChar);
                    if (lastChar != '\n') {
                        file.seek(file.size());
                        QTextStream out(&file);
                        out << "\n";
                      }
                  }
                  file.seek(file.size());
                  QTextStream out(&file);
                  out << pattern.trimmed() << "\n";
                  file.close();
                  refreshData();
              } else {
                  QMessageBox::critical(this, tr(".gitignore Error"),
                                        tr("Could not open .gitignore for writing:\n%1").arg(gitignorePath));
              }
          }
      } else if (res == cleanAction) {
          QGitCleanDialog dlg(this);
          if (dlg.exec() == QDialog::Accepted) {
              emit statusMessage(tr("Cleaning working directory..."));
              emit repositoryClean(dlg.removeIgnored(), dlg.removeDirectories());
          }
      }
}

void QGitRepository::on_treeWidget_pending_customContextMenuRequested(const QPoint &pos)
{
    const auto &selected = ui->treeWidget_pending->selectedItems();
    QList<QTreeWidgetItem*> selectedFiles;
    for (auto item : selected) {
        if (!item->data(0, Qt::UserRole).toString().isEmpty()) {
            selectedFiles << item;
        }
    }

    QMenu menu(this);
    QAction *resolveAction = nullptr;
    QAction *discardAction = nullptr;
    QAction *ignoreAction = nullptr;

    if (!selectedFiles.isEmpty()) {
        uint32_t status = selectedFiles.first()->data(0, Qt::UserRole + 1).toUInt();
        bool hasConflict = (status & GIT_STATUS_CONFLICTED);
        bool isUntracked = (status & GIT_STATUS_WT_NEW);

        if (hasConflict) {
            resolveAction = menu.addAction(tr("Resolve Conflicts..."));
        }
        discardAction = menu.addAction(tr("Discard changes"));

        if (isUntracked && selectedFiles.size() == 1) {
            menu.addSeparator();
            ignoreAction = menu.addAction(tr("Add to .gitignore..."));
        }
        menu.addSeparator();
    }

    QAction *cleanAction = menu.addAction(tr("Clean Working Directory..."));

    QAction *res = menu.exec(ui->treeWidget_pending->mapToGlobal(pos));
    if (resolveAction && res == resolveAction) {
        QGitConflictResolverDialog dlg(m_git, selectedFiles.first()->data(0, Qt::UserRole).toString(), this);
        if (dlg.exec() == QDialog::Accepted) {
            refreshData();
        }
    } else if (discardAction && res == discardAction) {
        auto confirm = QMessageBox::question(this, tr("Discard changes"),
                                             tr("Are you sure you want to discard changes in the selected files? This operation cannot be undone."),
                                             QMessageBox::Yes | QMessageBox::No);
        if (confirm == QMessageBox::Yes) {
            QStringList files;
            for (auto item : selectedFiles) {
                files << item->data(0, Qt::UserRole).toString();
            }
            emit repositoryDiscardFiles(files);
        }
    } else if (ignoreAction && res == ignoreAction) {
        QString filePath = selectedFiles.first()->data(0, Qt::UserRole).toString();
        bool ok = false;
        QString pattern = QInputDialog::getText(
            this,
            tr("Add to .gitignore"),
            tr("Pattern to ignore:"),
            QLineEdit::Normal,
            filePath,
            &ok);
        if (ok && !pattern.trimmed().isEmpty()) {
            QString gitignorePath = m_path + "/.gitignore";
            QFile file(gitignorePath);
            if (file.open(QIODevice::ReadWrite | QIODevice::Text)) {
                if (file.size() > 0) {
                    file.seek(file.size() - 1);
                    char lastChar;
                    file.getChar(&lastChar);
                    if (lastChar != '\n') {
                        file.seek(file.size());
                        QTextStream out(&file);
                        out << "\n";
                      }
                  }
                  file.seek(file.size());
                  QTextStream out(&file);
                  out << pattern.trimmed() << "\n";
                  file.close();
                  refreshData();
              } else {
                  QMessageBox::critical(this, tr(".gitignore Error"),
                                        tr("Could not open .gitignore for writing:\n%1").arg(gitignorePath));
              }
          }
      } else if (res == cleanAction) {
          QGitCleanDialog dlg(this);
          if (dlg.exec() == QDialog::Accepted) {
              emit statusMessage(tr("Cleaning working directory..."));
              emit repositoryClean(dlg.removeIgnored(), dlg.removeDirectories());
          }
      }
}

void QGitRepository::on_commit_diff_customContextMenuRequested(const QPoint &pos)
{
    if (!m_stageingFiles) return;

    QString fileName;
    int hoverFile = ui->commit_diff->hoverFile();
    int hoverHunk = ui->commit_diff->hoverHunk();
    int hoverLine = ui->commit_diff->hoverLine();

    QVector<QGitDiffWidgetLine> lines = ui->commit_diff->linesAt(hoverFile, hoverHunk, hoverLine, fileName);
    if (lines.isEmpty()) return;

    QMenu menu(this);
    QString actionText = (hoverLine >= 0) ? tr("Discard line") : tr("Discard hunk");
    QAction *discardAction = menu.addAction(actionText);

    QAction *res = menu.exec(ui->commit_diff->mapToGlobal(pos));
    if (res == discardAction) {
        auto confirm = QMessageBox::question(this, tr("Discard changes"),
                                             tr("Are you sure you want to discard this %1? This operation cannot be undone.")
                                             .arg((hoverLine >= 0) ? tr("line") : tr("hunk")),
                                             QMessageBox::Yes | QMessageBox::No);
        if (confirm == QMessageBox::Yes) {
            emit repositoryDiscardFileLines(fileName, lines);
        }
    }
}

void QGitRepository::on_logHistory_commits_customContextMenuRequested(const QPoint &pos)
{
    QModelIndex index = ui->logHistory_commits->indexAt(pos);
    if (!index.isValid()) return;

    int row = index.row();
    int col = (ui->logHistory_commits->columnCount() == 4) ? 3 : 4;
    QTableWidgetItem *item = ui->logHistory_commits->item(row, col);
    if (!item) return;

    QString selectedHash = item->data(Qt::UserRole).toString();
    if (selectedHash.isEmpty()) return;

    QString headHash = m_git->headCommitId();

    bool canRebase = true;
    if (selectedHash.compare(headHash, Qt::CaseInsensitive) == 0) {
        canRebase = false;
    } else if (!headHash.isEmpty() && m_git->isAncestor(selectedHash, headHash)) {
        canRebase = false;
    }

    QList<QGitRef> refs = ui->logHistory_commits->getReferences(selectedHash);
    QString shortHash = selectedHash.left(7);
    QString targetName = shortHash;
    for (const auto &ref : refs) {
        if (ref.type == QGitRef::CurrentBranch) {
            canRebase = false;
            targetName = ref.name;
            break;
        } else if (ref.type == QGitRef::LocalBranch || ref.type == QGitRef::RemoteBranch || ref.type == QGitRef::Tag) {
            targetName = ref.name;
        }
    }

    QMenu menu(this);
    QAction *rebaseAction = nullptr;
    QAction *cherrypickAction = nullptr;
    QAction *revertAction = nullptr;

    if (canRebase) {
        rebaseAction = menu.addAction(tr("Rebase current branch onto '%1'").arg(targetName));
    }

    bool canCherrypick = true;
    if (selectedHash.compare(headHash, Qt::CaseInsensitive) == 0) {
        canCherrypick = false;
    } else if (!headHash.isEmpty() && m_git->isAncestor(selectedHash, headHash)) {
        canCherrypick = false;
    }

    if (canCherrypick) {
        cherrypickAction = menu.addAction(tr("Cherry-pick commit '%1'").arg(shortHash));
    }

    revertAction = menu.addAction(tr("Revert commit '%1'").arg(shortHash));

    QMenu *resetMenu = new QMenu(tr("Reset current branch to '%1'").arg(shortHash), &menu);
    QAction *softResetAction = resetMenu->addAction(tr("Soft (keep changes staged)"));
    QAction *mixedResetAction = resetMenu->addAction(tr("Mixed (keep changes unstaged)"));
    QAction *hardResetAction = resetMenu->addAction(tr("Hard (discard all changes)"));
    menu.addMenu(resetMenu);

    menu.addSeparator();
    QAction *createTagAction = menu.addAction(tr("Create Tag at '%1'...").arg(shortHash));
    menu.addSeparator();
    QAction *editNoteAction = menu.addAction(tr("Edit Git Note..."));
    QAction *deleteNoteAction = menu.addAction(tr("Delete Git Note"));

    QAction *res = menu.exec(ui->logHistory_commits->viewport()->mapToGlobal(pos));
    if (res) {
        if (res == rebaseAction) {
            auto confirm = QMessageBox::question(this, tr("Rebase"),
                                                 tr("Are you sure you want to rebase the current branch onto '%1'?").arg(targetName),
                                                 QMessageBox::Yes | QMessageBox::No);
            if (confirm == QMessageBox::Yes) {
                emit statusMessage(tr("Rebasing current branch..."));
                emit repositoryRebase(selectedHash);
            }
        } else if (res == cherrypickAction) {
            auto confirm = QMessageBox::question(this, tr("Cherry-pick"),
                                                 tr("Are you sure you want to cherry-pick commit '%1' onto the current branch?").arg(shortHash),
                                                 QMessageBox::Yes | QMessageBox::No);
            if (confirm == QMessageBox::Yes) {
                emit statusMessage(tr("Cherry-picking commit..."));
                try {
                    m_git->cherrypick(selectedHash);
                    QMessageBox::information(this, tr("Cherry-pick"), tr("Commit '%1' cherry-picked successfully.").arg(shortHash));
                } catch (const QGitError &error) {
                    QMessageBox::warning(this, tr("Cherry-pick"), error.errorString());
                }
                emit statusMessage(tr("Ready"));
                refreshData();
            }
        } else if (res == revertAction) {
            auto confirm = QMessageBox::question(this, tr("Revert"),
                                                 tr("Are you sure you want to revert commit '%1'? This will apply the inverse changes onto your current branch.").arg(shortHash),
                                                 QMessageBox::Yes | QMessageBox::No);
            if (confirm == QMessageBox::Yes) {
                emit statusMessage(tr("Reverting commit..."));
                try {
                    m_git->revert(selectedHash);
                    QMessageBox::information(this, tr("Revert"), tr("Commit '%1' reverted successfully.").arg(shortHash));
                } catch (const QGitError &error) {
                    QMessageBox::warning(this, tr("Revert"), error.errorString());
                }
                emit statusMessage(tr("Ready"));
                refreshData();
            }
        } else if (res == softResetAction) {
            auto confirm = QMessageBox::question(this, tr("Soft Reset"),
                                                 tr("Are you sure you want to perform a soft reset to '%1'?\n\nThis will move the branch pointer, but keep all modifications staged.").arg(shortHash),
                                                 QMessageBox::Yes | QMessageBox::No);
            if (confirm == QMessageBox::Yes) {
                emit statusMessage(tr("Resetting current branch (soft)..."));
                try {
                    m_git->reset(selectedHash, GIT_RESET_SOFT);
                    QMessageBox::information(this, tr("Reset"), tr("Branch reset to '%1' successfully (soft).").arg(shortHash));
                } catch (const QGitError &error) {
                    QMessageBox::critical(this, tr("Reset Error"), error.errorString());
                }
                emit statusMessage(tr("Ready"));
                refreshData();
            }
        } else if (res == mixedResetAction) {
            auto confirm = QMessageBox::question(this, tr("Mixed Reset"),
                                                 tr("Are you sure you want to perform a mixed reset to '%1'?\n\nThis will move the branch pointer and unstage changes, but keep them in your working directory.").arg(shortHash),
                                                 QMessageBox::Yes | QMessageBox::No);
            if (confirm == QMessageBox::Yes) {
                emit statusMessage(tr("Resetting current branch (mixed)..."));
                try {
                    m_git->reset(selectedHash, GIT_RESET_MIXED);
                    QMessageBox::information(this, tr("Reset"), tr("Branch reset to '%1' successfully (mixed).").arg(shortHash));
                } catch (const QGitError &error) {
                    QMessageBox::critical(this, tr("Reset Error"), error.errorString());
                }
                emit statusMessage(tr("Ready"));
                refreshData();
            }
        } else if (res == hardResetAction) {
            auto confirm = QMessageBox::question(this, tr("Hard Reset"),
                                                 tr("WARNING: Are you sure you want to perform a hard reset to '%1'?\n\nThis will move the branch pointer and discard ALL staged and unstaged changes. This cannot be undone!").arg(shortHash),
                                                 QMessageBox::Yes | QMessageBox::No);
            if (confirm == QMessageBox::Yes) {
                emit statusMessage(tr("Resetting current branch (hard)..."));
                try {
                    m_git->reset(selectedHash, GIT_RESET_HARD);
                    QMessageBox::information(this, tr("Reset"), tr("Branch reset to '%1' successfully (hard).").arg(shortHash));
                } catch (const QGitError &error) {
                    QMessageBox::critical(this, tr("Reset Error"), error.errorString());
                }
                emit statusMessage(tr("Ready"));
                refreshData();
            }
        } else if (res == createTagAction) {
            QGitCreateTagDialog dlg(selectedHash, this);
            if (dlg.exec() == QDialog::Accepted) {
                emit statusMessage(tr("Creating tag..."));
                emit repositoryCreateTag(dlg.tagName(), selectedHash, dlg.tagMessage(), dlg.forceCreate());
            }
        } else if (res == editNoteAction) {
            QString currentNote = m_git->getNote(selectedHash);
            bool ok;
            QString noteText = QInputDialog::getMultiLineText(
                this,
                tr("Edit Git Note"),
                tr("Enter note text for commit %1:").arg(shortHash),
                currentNote,
                &ok
            );
            if (ok) {
                emit statusMessage(tr("Setting Git Note..."));
                emit repositorySetNote(selectedHash, noteText);
            }
        } else if (res == deleteNoteAction) {
            auto confirm = QMessageBox::question(this, tr("Delete Git Note"),
                                                 tr("Are you sure you want to delete the Git Note for commit %1?").arg(shortHash),
                                                 QMessageBox::Yes | QMessageBox::No);
            if (confirm == QMessageBox::Yes) {
                emit statusMessage(tr("Deleting Git Note..."));
                emit repositoryRemoveNote(selectedHash);
            }
        }
    }
}

void QGitRepository::deleteTagReply(const QGitError &error)
{
    if (error.errorCode() != 0) {
        QMessageBox::critical(this, tr("Delete Tag Error"), error.errorString());
    } else {
        refreshData();
    }
}

void QGitRepository::createTagReply(const QGitError &error)
{
    emit statusMessage(tr("Ready"));
    if (error.errorCode() != 0) {
        QMessageBox::critical(this, tr("Create Tag Error"), error.errorString());
    } else {
        QMessageBox::information(this, tr("Tag Created"), tr("Tag created successfully."));
        refreshData();
    }
}

void QGitRepository::repositoryCleanReply(const QGitError &error)
{
    emit statusMessage(tr("Ready"));
    if (error.errorCode() != 0) {
        QMessageBox::critical(this, tr("Clean Error"), error.errorString());
    } else {
        QMessageBox::information(this, tr("Clean Successful"), tr("Working directory cleaned successfully."));
        refreshData();
    }
}

void QGitRepository::repositoryApplyPatchReply(const QGitError &error)
{
    emit statusMessage(tr("Ready"));
    if (error.errorCode() != 0) {
        QMessageBox::critical(this, tr("Apply Patch Error"), error.errorString());
    } else {
        QMessageBox::information(this, tr("Apply Patch Successful"), tr("Patch applied successfully."));
        refreshData();
    }
}

void QGitRepository::repositorySetNoteReply(const QGitError &error)
{
    emit statusMessage(tr("Ready"));
    if (error.errorCode() != 0) {
        QMessageBox::critical(this, tr("Note Error"), error.errorString());
    } else {
        int currentRow = ui->logHistory_commits->currentRow();
        if (currentRow >= 0) {
            on_logHistory_commits_currentCellChanged(currentRow, 0, 0, 0);
        }
    }
}

void QGitRepository::repositoryRemoveNoteReply(const QGitError &error)
{
    emit statusMessage(tr("Ready"));
    if (error.errorCode() != 0) {
        QMessageBox::critical(this, tr("Note Error"), error.errorString());
    } else {
        int currentRow = ui->logHistory_commits->currentRow();
        if (currentRow >= 0) {
            on_logHistory_commits_currentCellChanged(currentRow, 0, 0, 0);
        }
    }
}

void QGitRepository::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_F2 && ui->branchesTreeView->hasFocus()) {
        QTreeWidgetItem *item = ui->branchesTreeView->currentItem();
        if (item) {
            QString type = item->data(0, Qt::UserRole + 2).toString();
            if (type == "LocalBranch" || type == "Tag") {
                ui->branchesTreeView->editItem(item, 0);
                event->accept();
                return;
            }
        }
    }

    if (event->key() == Qt::Key_Escape && m_searchingCommits) {
        emit repositoryAbortSearch();
        event->accept();
        return;
    }

    if (event->key() == Qt::Key_Delete && ui->branchesTreeView->hasFocus()) {
        QTreeWidgetItem *item = ui->branchesTreeView->currentItem();
        if (!item) return;

        QString type = item->data(0, Qt::UserRole + 2).toString();
        QString fullName = item->data(0, Qt::UserRole + 1).toString();

        if (type == "LocalBranch") {
            if (item->font(0).bold()) {
                QMessageBox::information(this, tr("Delete Branch"), tr("Cannot delete the current branch."));
                return;
            }
            auto res = QMessageBox::question(this, tr("Delete Branch"), 
                                             tr("Are you sure you want to delete branch '%1'?").arg(fullName),
                                             QMessageBox::Yes | QMessageBox::No);
            if (res == QMessageBox::Yes) {
                QList<QGitBranch> toDelete;
                toDelete << QGitBranch(fullName, "", 0, GIT_BRANCH_LOCAL);
                m_git->deleteBranches(toDelete, false);
            }
        } else if (type == "Stash") {
            auto res = QMessageBox::question(this, tr("Drop Stash"), 
                                             tr("Are you sure you want to drop stash '%1'?").arg(fullName),
                                             QMessageBox::Yes | QMessageBox::No);
            if (res == QMessageBox::Yes) {
                emit statusMessage(tr("Dropping stash %1...").arg(fullName));
                m_git->stashRemove(fullName);
            }
        } else if (type == "Tag") {
            auto res = QMessageBox::question(this, tr("Delete Tag"), 
                                             tr("Are you sure you want to delete tag '%1'?").arg(fullName),
                                             QMessageBox::Yes | QMessageBox::No);
            if (res == QMessageBox::Yes) {
                m_git->deleteTag(fullName);
            }
        }
    }
    QWidget::keyPressEvent(event);
}

void QGitRepository::on_comboBox_gitStatusFiles_itemClicked(int index)
{
    Q_UNUSED(index)

    fetchRepositoryChangedFiles();
}

void QGitRepository::on_comboBox_gitViewOptions_activated(int index)
{
    if (index >= 0 && index <= 2)
    {
        m_layoutOption = index;

        if (m_layoutOption == 0) {
            ui->treeWidget_staged->setHeaderHidden(true);
            ui->treeWidget_staged->setColumnCount(1);
            ui->treeWidget_unstaged->setHeaderHidden(true);
            ui->treeWidget_unstaged->setColumnCount(1);
            ui->treeWidget_pending->setHeaderHidden(true);
            ui->treeWidget_pending->setColumnCount(1);
        }
        else if (m_layoutOption == 1) {
            ui->treeWidget_staged->setHeaderHidden(false);
            ui->treeWidget_staged->setColumnCount(3);
            ui->treeWidget_unstaged->setHeaderHidden(false);
            ui->treeWidget_unstaged->setColumnCount(3);
            ui->treeWidget_pending->setHeaderHidden(false);
            ui->treeWidget_pending->setColumnCount(3);

            QStringList headers = { tr("Name"), tr("Path"), tr("Status") };
            ui->treeWidget_staged->setHeaderLabels(headers);
            ui->treeWidget_unstaged->setHeaderLabels(headers);
            ui->treeWidget_pending->setHeaderLabels(headers);

            ui->treeWidget_staged->setColumnWidth(0, 200);
            ui->treeWidget_staged->setColumnWidth(1, 300);
            ui->treeWidget_unstaged->setColumnWidth(0, 200);
            ui->treeWidget_unstaged->setColumnWidth(1, 300);
            ui->treeWidget_pending->setColumnWidth(0, 200);
            ui->treeWidget_pending->setColumnWidth(1, 300);
        }
        else if (m_layoutOption == 2) {
            ui->treeWidget_staged->setHeaderHidden(true);
            ui->treeWidget_staged->setColumnCount(1);
            ui->treeWidget_unstaged->setHeaderHidden(true);
            ui->treeWidget_unstaged->setColumnCount(1);
            ui->treeWidget_pending->setHeaderHidden(true);
            ui->treeWidget_pending->setColumnCount(1);
        }

        updateStatusViews();
    }
    else if (index == 4 || index == 5)
    {
        if (index == 4)
        {
            ui->stackedWidget_staging->setCurrentIndex(1);
        }
        else if (index == 5)
        {
            ui->stackedWidget_staging->setCurrentIndex(0);
        }

        if (m_layoutOption == 0) {
            ui->treeWidget_pending->setHeaderHidden(true);
            ui->treeWidget_pending->setColumnCount(1);
        }
        else if (m_layoutOption == 1) {
            ui->treeWidget_pending->setHeaderHidden(false);
            ui->treeWidget_pending->setColumnCount(3);
            QStringList headers = { tr("Name"), tr("Path"), tr("Status") };
            ui->treeWidget_pending->setHeaderLabels(headers);
            ui->treeWidget_pending->setColumnWidth(0, 200);
            ui->treeWidget_pending->setColumnWidth(1, 300);
        }
        else if (m_layoutOption == 2) {
            ui->treeWidget_pending->setHeaderHidden(true);
            ui->treeWidget_pending->setColumnCount(1);
        }

        updateStatusViews();
    }
}

void QGitRepository::on_comboBox_logBranchFilter_currentTextChanged(const QString &text)
{
    Q_UNUSED(text)
    int index = ui->comboBox_logBranchFilter->currentIndex();
    if (index < 0) return;

    QVariant data = ui->comboBox_logBranchFilter->itemData(index);
    if (!data.isValid()) {
        // It's a separator! Revert to HEAD (index 1)
        ui->comboBox_logBranchFilter->blockSignals(true);
        ui->comboBox_logBranchFilter->setCurrentIndex(1);
        m_currentLogBranchFilter = ui->comboBox_logBranchFilter->itemData(1).toString();
        ui->comboBox_logBranchFilter->blockSignals(false);
        return;
    }

    m_currentLogBranchFilter = data.toString();
    m_logCommitsOffset = 0;
    m_allCommitsLoaded = false;
    ui->logHistory_commits->setRowCount(0);
    fetchCommits();
}



void QGitRepository::fetchRepositoryChangedFiles()
{
    int show = 0;
    int sort = 0;
    bool reversed = false;

    switch(ui->comboBox_gitStatusFiles->showFiles())
    {
    case QComboBoxGitStatusFiles::QShowPendingFiles:
        show = QGit::QGIT_STATUS_NEW | QGit::QGIT_STATUS_MODIFIED | QGit::QGIT_STATUS_DELETED | QGit::QGIT_STATUS_RENAMED | QGit::QGIT_STATUS_TYPECHANGE;
        break;
    case QComboBoxGitStatusFiles::QShowConflictFiles:
        show = QGit::QGIT_STATUS_CONFLICTED;
        break;
    case QComboBoxGitStatusFiles::QShowUntracked:
        show = QGit::QGIT_STATUS_NEW;
        break;
    case QComboBoxGitStatusFiles::QShowIgnored:
        show = QGit::QGIT_STATUS_IGNORED;
        break;
    case QComboBoxGitStatusFiles::QShowClean:
        show = QGit::QGIT_STATUS_NONE;
        break;
    case QComboBoxGitStatusFiles::QShowModified:
        show = QGit::QGIT_STATUS_MODIFIED | QGit::QGIT_STATUS_DELETED | QGit::QGIT_STATUS_RENAMED | QGit::QGIT_STATUS_TYPECHANGE;
        break;
    case QComboBoxGitStatusFiles::QShowAll:
        show = QGit::QGIT_STATUS_ALL;
        break;
    }

    switch (ui->comboBox_gitStatusFiles->showSortBy())
    {
    case QComboBoxGitStatusFiles::QUnsortedFiles:
        sort = QGit::QGIT_SORT_UNSORTED;
        break;
    case QComboBoxGitStatusFiles::QFilePathSortFiles:
        sort = QGit::QGIT_SORT_FILEPATH;
        break;
    case QComboBoxGitStatusFiles::QReversedFilePathSortFiles:
        sort = QGit::QGIT_SORT_FILEPATH;
        reversed = true;
        break;
    case QComboBoxGitStatusFiles::QFileNameSortFiles:
        sort = QGit::QGIT_SORT_FILENAME;
        break;
    case QComboBoxGitStatusFiles::QReversedFileNameSortFiles:
        sort = QGit::QGIT_SORT_FILENAME;
        reversed = true;
        break;
    case QComboBoxGitStatusFiles::QFileStatusSortFiles:
        sort = QGit::QGIT_SORT_STATUS;
        break;
    case QComboBoxGitStatusFiles::QCheckedUncheckedSortFiles:
        sort = QGit::QGIT_SORT_CHECKED;
        break;
    }

    ui->treeWidget_staged->setEnabled(false);
    ui->treeWidget_unstaged->setEnabled(false);
    emit repositoryChangedFiles(show, sort, reversed);
}

void QGitRepository::on_lineEdit_search_returnPressed()
{
    QString text = ui->lineEdit_search->text();
    if (text.length() >= 3)
    {
        m_searchingCommits = true;
        ui->lineEdit_search->setEnabled(false);
        ui->search_commits->setRowCount(0);

        QString type;
        int index = ui->comboBox_searchType->currentIndex();
        if (index == 0)
        {
            type = "message";
        }
        else if (index == 1)
        {
            type = "files";
        }
        else if (index == 2)
        {
            type = "author";
        }

        emit statusMessage(tr("Searching commits..."));
        emit repositorySearchCommits(text, type);
    }
}

void QGitRepository::onCommitFound(const QGitCommit &commit)
{
    ui->search_commits->addCommit(commit);
}

void QGitRepository::onSearchFinished()
{
    m_searchingCommits = false;
    emit clearStatusMessage();
    ui->lineEdit_search->setEnabled(true);
    ui->lineEdit_search->setFocus();
}

void QGitRepository::on_search_commits_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    Q_UNUSED(currentColumn)
    Q_UNUSED(previousRow)
    Q_UNUSED(previousColumn)

    if (currentRow < 0)
    {
        return;
    }

    QString commit_id = ui->search_commits->item(currentRow, 3)->data(Qt::UserRole).toString();

    if (!commit_id.isEmpty())
    {
        emit repositoryGetCommitDiff(commit_id, ui->comboBox_gitDiffOptions->ignoreWhitespace());
    }
}

void QGitRepository::on_search_files_itemSelectionChanged()
{
    int row = ui->search_files->currentRow();
    if (row < 0) return;

    bool ignoreWhitespace = ui->comboBox_gitDiffOptions->ignoreWhitespace();

    ui->search_diff->setIgnoreWhitespace(ignoreWhitespace);
    ui->search_diff->setGitDiff(m_commitDiff.parents().at(0).commitHash(), m_commitDiff.id(), {ui->search_files->item(row, 0)->data(Qt::UserRole).toString()});
}

void QGitRepository::on_branchesTreeView_itemChanged(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column)

    ui->branchesTreeView->blockSignals(true);

    QString type = item->data(0, Qt::UserRole + 2).toString();
    QString oldFullName = item->data(0, Qt::UserRole + 1).toString();
    QString newText = item->text(0).trimmed();

    if (newText.isEmpty())
    {
        QMessageBox::critical(this, tr("Rename Error"), tr("Name cannot be empty."));
        item->setText(0, oldFullName.split('/').last());
        ui->branchesTreeView->blockSignals(false);
        return;
    }

    QStringList parts = oldFullName.split('/');
    if (!parts.isEmpty()) {
        parts.last() = newText;
    }
    QString newFullName = parts.join('/');

    if (newFullName == oldFullName)
    {
        ui->branchesTreeView->blockSignals(false);
        return;
    }

    // Revert text temporarily, it will be updated by refreshData() once the backend operation finishes.
    item->setText(0, oldFullName.split('/').last());
    ui->branchesTreeView->blockSignals(false);

    if (type == "LocalBranch")
    {
        emit statusMessage(tr("Renaming branch %1 to %2...").arg(oldFullName, newFullName));
        emit repositoryRenameBranch(oldFullName, newFullName);
    }
    else if (type == "Tag")
    {
        emit statusMessage(tr("Renaming tag %1 to %2...").arg(oldFullName, newFullName));
        emit repositoryRenameTag(oldFullName, newFullName);
    }
}



void QGitRepository::collectFilePaths(QTreeWidgetItem *item, QStringList &paths)
{
    if (!item) return;
    QString path = item->data(0, Qt::UserRole).toString();
    if (!path.isEmpty())
    {
        paths << path;
    }
    for (int i = 0; i < item->childCount(); ++i)
    {
        collectFilePaths(item->child(i), paths);
    }
}
