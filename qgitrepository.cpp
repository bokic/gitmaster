#include "qgitrepository.h"
#include "ui_qgitrepository.h"
#include "qloghistoryitemdelegate.h"
#include "qgitmastermainwindow.h"
#include "qgitfetchdialog.h"
#include "qgitpulldialog.h"
#include "qgitpushdialog.h"
#include "qgitbranchdialog.h"
#include <qgitbranch.h>

#include <QCryptographicHash>
#include <QTableWidgetItem>
#include <QTreeWidgetItem>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QMessageBox>
#include <QInputDialog>
#include <QKeyEvent>
#include <QScrollBar>
#include <QWindow>
#include <QMenu>
#include <QAction>
#include <QInputDialog>
#include <QList>

#define COMMIT_COUNT_TO_LOAD 100


QGitRepository::QGitRepository(const QString &path, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::QGitRepository)
    , m_path(path)
    , m_allCommitsLoaded(false)
    , m_iconFileNew(":/small/added")
    , m_iconFileClean(":/small/clean")
    , m_iconFileModified(":/small/modified")
    , m_iconFileRemoved(":/small/deleted")
    , m_iconFileIgnored(":/small/ignored")
    , m_iconFileUnknown(":/small/unknown")
    , m_iconTag(":/small/tag")
    , m_iconWhiteCheckbox(":/small/white_checkbox")
    , m_iconCurrentBranch(":/small/current_branch")
    , m_iconBranch(":/small/branch")
    , m_iconRemote(":/small/remote")
    , m_iconStash(":/small/stash")
    , m_iconRemoteBranch(":/small/remote_branch")
    , m_git(new QGit())
{
    QString name;
    QString email;

    ui->setupUi(this);

    ui->commit_diff->setContentsMargins(10, 10, 10, 10);

    ui->logHistory_commits->setItemDelegate(new QLogHistoryItemDelegate());

    m_git->moveToThread(&m_thread);

    ui->splitter_3->setStretchFactor(0, 0);
    ui->splitter_3->setStretchFactor(1, 1);

    ui->logHistory_splitter_1->setStretchFactor(0, 0);
    ui->logHistory_splitter_1->setStretchFactor(1, 1);

    ui->logHistory_splitter_2->setStretchFactor(0, 0);
    ui->logHistory_splitter_2->setStretchFactor(1, 1);


    if (QGit::gitRepositoryDefaultSignature(m_path, name, email))
    {
        QByteArray hash;
        QNetworkReply *reply = nullptr;

        hash = QCryptographicHash::hash(email.trimmed().toUtf8(), QCryptographicHash::Md5).toHex();

        QString urlStr = QString("https://www.gravatar.com/avatar/%1?s=24").arg(QString::fromLatin1(hash));
        auto url = QUrl(urlStr);
        //auto req = QNetworkRequest(url);
        connect(&m_networkManager, &QNetworkAccessManager::finished, this, &QGitRepository::gravatarImageDownloadFinished);

        m_networkManager.get(QNetworkRequest(url));

        //reply = m_networkManager.get(req);

        //connect(reply, &QNetworkReply::finished, this, &QGitRepository::gravatarImageDownloadFinished);

        ui->label_signatureGravatarImage->setToolTip(tr("%1 <%2>").arg(name, email));
        ui->label_signatureEmail->setText(tr("%1 <%2>").arg(name, email));
    }

    //ui->label_signatureEmail->setVisible(false);
    //ui->toolButton->setVisible(false);
    //ui->comboBox_4->setVisible(false);
    //ui->checkBox_3->setVisible(false);
    //ui->pushButton_commit->setVisible(false);
    //ui->pushButton_2->setVisible(false);

    ui->logHistory_files->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->logHistory_commits->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

    activateCommitOperation(false);

    connect(this, &QGitRepository::localStash, m_git, &QGit::stashSave);
    connect(m_git, &QGit::stashSaveReply, this, &QGitRepository::localStashSaveReply);
    connect(m_git, &QGit::stashRemoveReply, this, &QGitRepository::localStashRemoveReply);

    connect(this, &QGitRepository::repositoryFetch, m_git, &QGit::fetch);
    connect(m_git, &QGit::fetchReply, this, &QGitRepository::repositoryFetchReply);

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

    connect(this, &QGitRepository::repositoryCommit, m_git, &QGit::commit);
    connect(m_git, &QGit::commitReply, this, &QGitRepository::repositoryCommitReply);

    connect(this, &QGitRepository::repositoryGetCommits, m_git, &QGit::listCommits);
    connect(m_git, &QGit::listCommitsReply, this, &QGitRepository::repositoryGetCommitsReply);

    connect(this, &QGitRepository::repositoryGetCommitDiff, m_git, &QGit::commitDiff);
    connect(m_git, &QGit::commitDiffReply, this, &QGitRepository::repositoryGetCommitDiffReply);

    connect(ui->commit_diff, &QGitDiffWidget::select, this, &QGitRepository::selectedLines);

    connect(this, &QGitRepository::stageFileLines, m_git, &QGit::stageFileLines);
    connect(this, &QGitRepository::unstageFileLines, m_git, &QGit::unstageFileLines);

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

    ui->branchesTreeView->setContextMenuPolicy(Qt::CustomContextMenu);

    m_thread.setObjectName("QGit(repo)");
    m_thread.start();

    m_git->setPath(QDir(m_path));
}

QGitRepository::~QGitRepository()
{
    QAbstractItemDelegate *delegate = nullptr;

    m_git->deleteLater();

    m_thread.quit();
    m_thread.wait();

    delegate = ui->logHistory_commits->itemDelegate();

    m_git = nullptr;

    delete ui;

    delete delegate;
}

void QGitRepository::refreshData()
{
    on_repositoryDetail_currentChanged(ui->repositoryDetail->currentIndex());

    emit repositoryBranches();
    emit repositoryStashes();
}

void QGitRepository::stash(const QString &name)
{
    m_git->stashSave(name);
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

        QGitMasterMainWindow::instance()->updateStatusBarText("Fetching...");
        emit repositoryFetch(fetchFromAllRemotes, purgeDeletedBranches, fetchAllTags);
    }
}

void QGitRepository::pull()
{
    QGitPullDialog dlg(this);

    if (dlg.exec() == QDialog::Accepted)
    {
        bool rebase = dlg.rebase();
        QGitMasterMainWindow::instance()->updateStatusBarText("Pulling...");
        m_git->pull(rebase);
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

        QGitMasterMainWindow::instance()->updateStatusBarText(tr("Pushing..."));
        emit repositoryPush(remote, branches, tags, force);
    }
}

QGit *QGitRepository::git() const
{
    return m_git;
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
    //auto reply = dynamic_cast<QNetworkReply *>(sender());

    if (reply) {
        QByteArray data = reply->readAll();

        QPixmap pixmap;

        pixmap.loadFromData(data);

        ui->label_signatureGravatarImage->setPixmap(pixmap);

        reply->deleteLater();
    }
}

void QGitRepository::localStashSaveReply(QGitError error)
{
    if (error.errorCode()) {
        QMessageBox::critical(this, tr("Error saving stash"), error.errorString());
    } else {
        refreshData();
    }
}

void QGitRepository::localStashRemoveReply(QGitError error)
{
    if (error.errorCode()) {
        QMessageBox::critical(this, tr("Error dropping stash"), error.errorString());
    } else {
        refreshData();
    }
}

void QGitRepository::repositoryFetchReply(QGitError error)
{
    if (error.errorCode())
    {
        QGitMasterMainWindow::instance()->updateStatusBarText(error.functionName());
    }
    else
    {
        QGitMasterMainWindow::instance()->clearStatusBarText();
    }
}

void QGitRepository::repositoryPushReply(QGitError error)
{
    if (error.errorCode())
    {
        QString errStr = error.errorString();
        QGitMasterMainWindow::instance()->updateStatusBarText(errStr);

        if (errStr.contains(QStringLiteral("rejected"), Qt::CaseInsensitive) || errStr.contains(QStringLiteral("non-fast-forward"), Qt::CaseInsensitive))
        {
            auto res = QMessageBox::question(this, tr("Push Rejected"), 
                                             tr("Push was rejected by the remote (likely non-fast-forward). Do you want to force push?"),
                                             QMessageBox::Yes | QMessageBox::No);
            if (res == QMessageBox::Yes)
            {
                QGitMasterMainWindow::instance()->updateStatusBarText(tr("Force Pushing..."));
                emit repositoryPush(m_lastRemote, m_lastBranches, m_lastTags, true);
                return;
            }
        }
    }
    else
    {
        QGitMasterMainWindow::instance()->clearStatusBarText();
    }
}

void QGitRepository::repositoryBranchesAndTagsReply(QList<QGitBranch> branches, QList<QGitTag> tags, QGitError error)
{
    Q_UNUSED(error)

    QList<QTreeWidgetItem *> items;
    QTreeWidgetItem *itemWorkingCopy = new QTreeWidgetItem(QStringList() << tr("Working Copy"));
    QTreeWidgetItem *itemFileStatus = new QTreeWidgetItem(QStringList() << tr("File Status"));
    QTreeWidgetItem *itemLocalBranches = new QTreeWidgetItem(QStringList() << tr("Branches"));
    QTreeWidgetItem *itemTags = new QTreeWidgetItem(QStringList() << tr("Tags"));
    QTreeWidgetItem *itemRemoteBranches = new QTreeWidgetItem(QStringList() << tr("Remotes"));


    itemWorkingCopy->setIcon(0, m_iconWhiteCheckbox);
    itemFileStatus->addChild(itemWorkingCopy);

    auto current_branch = QGit::getBranchNameFromPath(m_git->path().path());

    std::sort(branches.begin(), branches.end(), [](const QGitBranch &a, const QGitBranch &b) {
        return a.time() < b.time();
    });

    for(const auto &branch: branches)
    {
        QStringList items = branch.name().split('/');

        if (branch.type() & GIT_BRANCH_LOCAL)
        {
            if ((items.count() >= 3)&&(items[0] == QStringLiteral("refs"))&&(items[1] == QStringLiteral("heads")))
            {
                QTreeWidgetItem *item = itemLocalBranches;

                for(int depth = 2; depth < items.count(); depth++)
                {
                    const QString &name = items.at(depth);
                    bool found = false;

                    for(int c = 0; c < item->childCount(); c++)
                    {
                        if (item->child(c)->text(0) == name)
                        {
                            item = item->child(c);
                            found = true;

                            break;
                        }
                    }

                        if (!found)
                        {
                            QTreeWidgetItem *child = new QTreeWidgetItem(QStringList() << name);
                            child->setData(0, Qt::UserRole, branch.hash());
                            if (depth == items.count() - 1) {
                                child->setData(0, Qt::UserRole + 1, items.mid(2).join('/'));
                                child->setData(0, Qt::UserRole + 2, "LocalBranch");
                            }
                            if (name == current_branch)
                        {
                            auto font = child->font(0);
                            font.setBold(true);
                            child->setFont(0, font);

                            child->setIcon(0, m_iconCurrentBranch);
                        }
                        else
                        {
                            child->setIcon(0, m_iconBranch);
                        }

                        item->addChild(child);
                        item = child;
                    }
                }
            }
        }
        if (branch.type() & GIT_BRANCH_REMOTE)
        {
            if ((items.count() >= 4)&&(items[0] == QStringLiteral("refs"))&&(items[1] == QStringLiteral("remotes")))
            {
                QTreeWidgetItem *item = itemRemoteBranches;

                for(int depth = 2; depth < items.count(); depth++)
                {
                    const QString &name = items.at(depth);
                    bool found = false;

                    for(int c = 0; c < item->childCount(); c++)
                    {
                        if (item->child(c)->text(0) == name)
                        {
                            item = item->child(c);
                            found = true;

                            break;
                        }
                    }

                    if (!found)
                    {
                        QTreeWidgetItem *child = new QTreeWidgetItem(QStringList() << name);
                        child->setData(0, Qt::UserRole, branch.hash());
                        if (depth == items.count() - 1) {
                            child->setData(0, Qt::UserRole + 1, items.mid(2).join('/'));
                            child->setData(0, Qt::UserRole + 2, "RemoteBranch");
                        }

                        if (depth == 2)
                            child->setIcon(0, m_iconRemote);
                        else
                            child->setIcon(0, m_iconRemoteBranch);

                        item->addChild(child);
                        item = child;
                    }
                }
            }
        }
    }

    std::sort(tags.begin(), tags.end(), [](const QGitTag &a, const QGitTag &b) {
        return a.time() > b.time();
    });

    for(const auto &tag: tags)
    {
        QTreeWidgetItem *child = new QTreeWidgetItem(QStringList() << tag.name());
        child->setData(0, Qt::UserRole, tag.hash());
        child->setData(0, Qt::UserRole + 1, tag.name());
        child->setData(0, Qt::UserRole + 2, "Tag");

        child->setIcon(0, m_iconTag);
        itemTags->addChild(child);
    }

    items.append(itemFileStatus);
    items.append(itemLocalBranches);
    items.append(itemTags);
    items.append(itemRemoteBranches);

    ui->branchesTreeView->clear();
    ui->branchesTreeView->addTopLevelItems(items);
    ui->branchesTreeView->expandAll();
}

void QGitRepository::repositoryStashesReply(QStringList stashes, QGitError error)
{
    Q_UNUSED(error)

    QTreeWidgetItem *stashGroupItem = nullptr;
    for (int i = 0; i < ui->branchesTreeView->topLevelItemCount(); ++i) {
        if (ui->branchesTreeView->topLevelItem(i)->text(0) == tr("Stashes")) {
            stashGroupItem = ui->branchesTreeView->topLevelItem(i);
            break;
        }
    }

    if (stashes.isEmpty())
    {
        if (stashGroupItem) {
            delete ui->branchesTreeView->takeTopLevelItem(ui->branchesTreeView->indexOfTopLevelItem(stashGroupItem));
        }
        return;
    }

    if (!stashGroupItem)
    {
        stashGroupItem = new QTreeWidgetItem(QStringList() << tr("Stashes"));
        ui->branchesTreeView->addTopLevelItem(stashGroupItem);
    }
    else
    {
        stashGroupItem->takeChildren();
    }

    for(const auto &stash: stashes)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(QStringList() << stash);
        item->setIcon(0, m_iconStash);
        item->setData(0, Qt::UserRole + 1, stash);
        item->setData(0, Qt::UserRole + 2, "Stash");
        stashGroupItem->addChild(item);
    }

    ui->branchesTreeView->expandAll();
}

void QGitRepository::repositoryChangedFilesReply(QList<QPair<QString, git_status_t>> files, QGitError error)
{
    int stagedCnt = 0;
    int unstagedCnt = 0;

    Q_UNUSED(error)

    ui->listWidget_staged->setEnabled(true);
    ui->listWidget_unstaged->setEnabled(true);

    for(int c = 0; c < files.count(); c++)
    {
        const auto &file = files.at(c).first;
        auto status = files.at(c).second;

        if (status & (GIT_STATUS_INDEX_NEW | GIT_STATUS_INDEX_MODIFIED | GIT_STATUS_INDEX_DELETED | GIT_STATUS_INDEX_RENAMED | GIT_STATUS_INDEX_TYPECHANGE))
        {
            QListWidgetItem *item = nullptr;

            // UI item already on same row
            if ((ui->listWidget_staged->count() > stagedCnt)&&(ui->listWidget_staged->item(stagedCnt)->text() == file))
            {
                item = ui->listWidget_staged->item(stagedCnt);
            }

            // Check if UI item is further in the list
            if (!item)
            {
                for(int c2 = stagedCnt + 1; c2 < ui->listWidget_staged->count(); c2++)
                {
                    if (ui->listWidget_staged->item(c2)->text() == file)
                    {
                        int deleteCnt = c2 - c;
                        for(int c3 = 0; c3 < deleteCnt; c3++)
                        {
                            ui->listWidget_staged->takeItem(c3);
                        }
                        item = ui->listWidget_staged->item(stagedCnt);
                        break;
                    }
                }
            }

            // Add as new UI item
            if (!item)
            {
                item = new QListWidgetItem(file);

                item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
                item->setCheckState(Qt::Checked);

                if (stagedCnt < ui->listWidget_staged->count())
                    ui->listWidget_staged->insertItem(stagedCnt, item);
                else
                    ui->listWidget_staged->addItem(item);
            }

            switch(status & (GIT_STATUS_INDEX_NEW | GIT_STATUS_INDEX_MODIFIED | GIT_STATUS_INDEX_DELETED | GIT_STATUS_INDEX_RENAMED | GIT_STATUS_INDEX_TYPECHANGE))
            {
            case GIT_STATUS_INDEX_NEW:
                item->setIcon(m_iconFileNew);
                break;
            case GIT_STATUS_INDEX_DELETED:
                item->setIcon(m_iconFileRemoved);
                break;
            default:
                item->setIcon(m_iconFileModified);
                break;
            }

            stagedCnt++;
        }

        if ((status == GIT_STATUS_CURRENT)||(status & (GIT_STATUS_WT_NEW | GIT_STATUS_WT_MODIFIED | GIT_STATUS_WT_DELETED | GIT_STATUS_WT_TYPECHANGE | GIT_STATUS_WT_RENAMED | GIT_STATUS_WT_UNREADABLE | GIT_STATUS_IGNORED | GIT_STATUS_CONFLICTED)))
        {
            QListWidgetItem *item = nullptr;

            // UI item already on same row
            if ((ui->listWidget_unstaged->count() > unstagedCnt)&&(ui->listWidget_unstaged->item(unstagedCnt)->text() == file))
            {
                item = ui->listWidget_unstaged->item(unstagedCnt);
            }

            // Check if UI item is further in the list
            if (!item)
            {
                for(int c2 = unstagedCnt + 1; c2 < ui->listWidget_unstaged->count(); c2++)
                {
                    if (ui->listWidget_unstaged->item(c2)->text() == file)
                    {
                        int deleteCnt = c2 - c;
                        for(int c3 = 0; c3 < deleteCnt; c3++)
                        {
                            ui->listWidget_unstaged->takeItem(c3);
                        }
                        item = ui->listWidget_unstaged->item(unstagedCnt);
                        break;
                    }
                }
            }

            // Add as new UI item
            if (!item)
            {
                item = new QListWidgetItem(file);

                item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
                item->setCheckState(Qt::Unchecked);

                if (unstagedCnt < ui->listWidget_unstaged->count())
                    ui->listWidget_unstaged->insertItem(unstagedCnt, item);
                else
                    ui->listWidget_unstaged->addItem(item);
            }

            switch(status & (GIT_STATUS_CURRENT | GIT_STATUS_WT_NEW | GIT_STATUS_WT_MODIFIED | GIT_STATUS_WT_DELETED | GIT_STATUS_IGNORED | GIT_STATUS_CONFLICTED))
            {
            case GIT_STATUS_CURRENT:
                item->setIcon(m_iconFileClean);
                break;
            case GIT_STATUS_WT_NEW:
                item->setIcon(m_iconFileNew);
                break;
            case GIT_STATUS_WT_MODIFIED:
                item->setIcon(m_iconFileModified);
                break;
            case GIT_STATUS_WT_DELETED:
                item->setIcon(m_iconFileRemoved);
                break;
            case GIT_STATUS_IGNORED:
                item->setIcon(m_iconFileIgnored);
                break;
            default:
                item->setIcon(m_iconFileUnknown);
                break;
            }

            unstagedCnt++;
        }
    }

    while(ui->listWidget_staged->count() > stagedCnt)
    {
        delete ui->listWidget_staged->takeItem(ui->listWidget_staged->count() - 1);
    }
    while(ui->listWidget_unstaged->count() > unstagedCnt)
    {
        delete ui->listWidget_unstaged->takeItem(ui->listWidget_unstaged->count() - 1);
    }

    ui->commit_diff->refresh();

    if (ui->listWidget_staged->count() > 0)
    {
        ui->pushButton_commit->setEnabled(true);
    }
    else
    {
        ui->pushButton_commit->setEnabled(false);
    }
}

void QGitRepository::repositoryStageFilesReply(QGitError error)
{
    Q_UNUSED(error)

    fetchRepositoryChangedFiles();
}

void QGitRepository::repositoryUnstageFilesReply(QGitError error)
{
    Q_UNUSED(error)

    fetchRepositoryChangedFiles();
}

void QGitRepository::repositoryCommitReply(QString commit_id, QGitError error)
{
    Q_UNUSED(commit_id)
    Q_UNUSED(error)

    ui->plainTextEdit_commitMessage->clear();
	ui->plainTextEdit_commitMessage->setEnabled(true);
	ui->pushButton_commit->setEnabled(true);

    on_repositoryDetail_currentChanged(ui->repositoryDetail->currentIndex());
}

void QGitRepository::repositoryGetCommitsReply(QList<QGitCommit> commits, QGitError error)
{
    Q_UNUSED(error)

    for(const auto &commit: commits)
    {
        ui->logHistory_commits->addCommit(commit);
    }

    if (commits.count() < COMMIT_COUNT_TO_LOAD)
    {
        m_allCommitsLoaded = true;
    }

    if (!m_searchingCommitHash.isEmpty())
    {
        if (ui->logHistory_commits->selectCommit(m_searchingCommitHash)) {
            m_searchingCommitHash = "";
            QGitMasterMainWindow::instance()->clearStatusBarText();
        } else {
            if (m_allCommitsLoaded) {
                QMessageBox::information(this, tr("Commit not found"), 
                                         tr("The commit [%1] was not found in the repository history.").arg(m_searchingCommitHash.left(10)));
                m_searchingCommitHash = "";
                QGitMasterMainWindow::instance()->clearStatusBarText();
            } else {
                fetchCommits();
                QGitMasterMainWindow::instance()->updateStatusBarText(tr("Searching for commit %1...").arg(m_searchingCommitHash.left(10)));
            }
        }
    }
    else
    {
        QGitMasterMainWindow::instance()->clearStatusBarText();
    }
}

void QGitRepository::repositoryGetCommitDiffReply(QString commitId, QGitCommit diff, QGitError error)
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

        int currentRow = ui->logHistory_commits->currentRow();

        if (currentRow < 0)
        {
            return;
        }

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
        html += QStringLiteral("<b>Labels:</b> ") + labelsHtml.join(", ") + QStringLiteral("<br />");
        html += QStringLiteral("<br />");
        html += m_commitDiff.message();

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

            /*GIT_DELTA_UNMODIFIED = 0,
            GIT_DELTA_ADDED = 1,
            GIT_DELTA_DELETED = 2,
            GIT_DELTA_MODIFIED = 3,
            GIT_DELTA_RENAMED = 4,
            GIT_DELTA_COPIED = 5,
            GIT_DELTA_IGNORED = 6,
            GIT_DELTA_UNTRACKED = 7,
            GIT_DELTA_TYPECHANGE = 8,
            GIT_DELTA_UNREADABLE = 9,
            GIT_DELTA_CONFLICTED = 10,*/


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
                item_icon = m_iconFileUnknown; // TODO: Add icons for other statuses.
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
        }
    }
}

void QGitRepository::deleteBranchesReply(QGitError error)
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
                QGitMasterMainWindow::instance()->updateStatusBarText(tr("Searching for commit %1...").arg(hash.left(10)));
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
            QGitMasterMainWindow::instance()->updateStatusBarText(tr("Applying stash %1...").arg(fullName));
            m_git->stashApply(fullName);
        }
        else if (selectedAction == popAction)
        {
            QGitMasterMainWindow::instance()->updateStatusBarText(tr("Popping stash %1...").arg(fullName));
            m_git->stashPop(fullName);
        }
        else if (selectedAction == dropAction)
        {
            auto res = QMessageBox::question(this, tr("Drop Stash"), 
                                             tr("Are you sure you want to drop stash '%1'?").arg(fullName),
                                             QMessageBox::Yes | QMessageBox::No);
            if (res == QMessageBox::Yes)
            {
                QGitMasterMainWindow::instance()->updateStatusBarText(tr("Dropping stash %1...").arg(fullName));
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
        QAction *deleteAction = menu.addAction(tr("Delete Branch"));

        if (item->font(0).bold()) {
            checkoutAction->setEnabled(false);
            deleteAction->setEnabled(false);
        }

        QAction *selectedAction = menu.exec(ui->branchesTreeView->viewport()->mapToGlobal(pos));
        if (selectedAction == checkoutAction)
        {
            QGitMasterMainWindow::instance()->updateStatusBarText(tr("Checking out branch %1...").arg(fullName));
            m_git->checkoutBranch(fullName);
        }
        else if (selectedAction == renameAction)
        {
            QString newName = QInputDialog::getText(this, tr("Rename Branch"), tr("Enter new name for branch %1:").arg(fullName), QLineEdit::Normal, fullName);
            if (!newName.isEmpty() && newName != fullName) {
                QGitMasterMainWindow::instance()->updateStatusBarText(tr("Renaming branch %1 to %2...").arg(fullName, newName));
                m_git->renameBranch(fullName, newName);
            }
        }
        else if (selectedAction == setUpstreamAction)
        {
            QString upstream = QInputDialog::getText(this, tr("Set Upstream"), tr("Enter upstream for branch %1:").arg(fullName));
            if (!upstream.isEmpty()) {
                m_git->setUpstream(fullName, upstream);
            }
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
    }
    else if (type == "Tag")
    {
        QMenu menu(this);
        QAction *deleteAction = menu.addAction(tr("Delete Tag"));
        
        QAction *selectedAction = menu.exec(ui->branchesTreeView->viewport()->mapToGlobal(pos));
        if (selectedAction == deleteAction)
        {
             // TODO: Implement delete tag in backend
        }
    }
}

void QGitRepository::checkoutBranchReply(QGitError error)
{
    if (error.errorCode())
    {
        QMessageBox::critical(this, tr("Checkout Error"), error.errorString());
    }
    refreshData();
}

void QGitRepository::renameBranchReply(QGitError error)
{
    if (error.errorCode())
    {
        QMessageBox::critical(this, tr("Rename Error"), error.errorString());
    }
    refreshData();
}

void QGitRepository::setUpstreamReply(QGitError error)
{
    if (error.errorCode())
    {
        QMessageBox::critical(this, tr("Set Upstream Error"), error.errorString());
    }
    refreshData();
}

void QGitRepository::stashApplyReply(QGitError error)
{
    if (error.errorCode())
    {
        QMessageBox::critical(this, tr("Stash Apply Error"), 
                              tr("Could not apply stash. This might be due to merge conflicts or local changes.\n\nDetails: %1").arg(error.errorString()));
        QGitMasterMainWindow::instance()->updateStatusBarText(tr("Stash apply failed."));
    }
    else
    {
        QGitMasterMainWindow::instance()->updateStatusBarText(tr("Stash applied successfully."));
        refreshData();
    }
}

void QGitRepository::stashPopReply(QGitError error)
{
    if (error.errorCode())
    {
        QMessageBox::critical(this, tr("Stash Pop Error"), 
                              tr("Could not pop stash. Changes might have conflicts with your working copy.\n\nDetails: %1").arg(error.errorString()));
        QGitMasterMainWindow::instance()->updateStatusBarText(tr("Stash pop failed."));
    }
    else
    {
        QGitMasterMainWindow::instance()->updateStatusBarText(tr("Stash popped successfully."));
        refreshData();
    }
}

void QGitRepository::on_repositoryDetail_currentChanged(int index)
{
    switch(index) {
    case 0:
        disconnect(ui->logHistory_diff, &QGitDiffWidget::requestGitDiff, m_git, &QGit::commitDiffContent);
        disconnect(m_git, &QGit::commitDiffContentReply, ui->logHistory_diff, &QGitDiffWidget::responseGitDiff);

        connect(ui->commit_diff, &QGitDiffWidget::requestGitDiff, m_git, &QGit::commitDiffContent);
        connect(m_git, &QGit::commitDiffContentReply, ui->commit_diff, &QGitDiffWidget::responseGitDiff);

        fetchRepositoryChangedFiles();
        break;
    case 1:
        disconnect(ui->commit_diff, &QGitDiffWidget::requestGitDiff, m_git, &QGit::commitDiffContent);
        disconnect(m_git, &QGit::commitDiffContentReply, ui->commit_diff, &QGitDiffWidget::responseGitDiff);

        connect(ui->logHistory_diff, &QGitDiffWidget::requestGitDiff, m_git, &QGit::commitDiffContent);
        connect(m_git, &QGit::commitDiffContentReply, ui->logHistory_diff, &QGitDiffWidget::responseGitDiff);

        fetchCommits();
        break;
    case 2:
        Q_UNIMPLEMENTED();
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

    for(int c = 0; c < ui->listWidget_staged->count(); c++)
    {
        QString file = ui->listWidget_staged->item(c)->text();

        items.append(file);
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

    for(int c = 0; c < ui->listWidget_unstaged->count(); c++)
    {
        QString file = ui->listWidget_unstaged->item(c)->text();

        items.append(file);
    }

    if (items.count() > 0)
    {
        emit repositoryStageFiles(items);
    }
}

void QGitRepository::on_listWidget_staged_itemChanged(QListWidgetItem *item)
{
    QStringList selectedItems;

    for(int row = ui->listWidget_staged->count() - 1; row >= 0; row--)
    {
        QListWidgetItem *item = ui->listWidget_staged->item(row);

        if (item->checkState() == Qt::Unchecked)
        {
            selectedItems.append(item->text());

            delete ui->listWidget_staged->takeItem(row);
        }
    }

    if (!selectedItems.isEmpty())
    {
        emit repositoryUnstageFiles(selectedItems);
    }
}

void QGitRepository::on_listWidget_unstaged_itemChanged(QListWidgetItem *item)
{
    QStringList selectedItems;

    for(int row = ui->listWidget_unstaged->count() - 1; row >= 0; row--)
    {
        QListWidgetItem *item = ui->listWidget_unstaged->item(row);

        if (item->checkState() == Qt::Checked)
        {
            selectedItems.append(item->text());

            delete ui->listWidget_unstaged->takeItem(row);
        }
    }

    if (!selectedItems.isEmpty())
    {
        emit repositoryStageFiles(selectedItems);
    }
}

void QGitRepository::on_pushButton_commit_clicked()
{
    ui->plainTextEdit_commitMessage->setEnabled(false);
    ui->pushButton_commit->setEnabled(false);

    bool withPush = ui->checkBox_pushChangesImmidietely->isChecked();

    emit repositoryCommit(ui->plainTextEdit_commitMessage->toPlainText(), withPush);
}

void QGitRepository::on_pushButton_commitCancel_clicked()
{
    activateCommitOperation(false);
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

void QGitRepository::selectedLines(QString filename, QVector<QGitDiffWidgetLine> lines)
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
        emit repositoryGetCommitDiff(commit_id);
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
        int rows = ui->logHistory_commits->rowCount();

        if (rows > 0)
        {
            QString last_commit_hash = ui->logHistory_commits->item(rows - 1, 4)->data(Qt::UserRole).toString();

            emit repositoryGetCommits(last_commit_hash, COMMIT_COUNT_TO_LOAD);
        }
        else
        {
            emit repositoryGetCommits("", COMMIT_COUNT_TO_LOAD);
        }

        QGitMasterMainWindow::instance()->updateStatusBarText(tr("Fetching commits..."));
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
    QList<QString> files;

    const auto &selected = ui->logHistory_files->selectedItems();
    int parent = 0; // TODO: Parent is hardcoded.

    if (m_commitDiff.parents().size() <= parent)
    {
        return;
    }

    for(auto cell: selected) {
        if (cell->column() == 0)
        {
            files << cell->data(Qt::UserRole).toString();
        }
    }

    ui->logHistory_diff->setGitDiff(m_commitDiff.parents().at(parent).commitHash(), m_commitDiff.id(), files);
}

void QGitRepository::on_listWidget_staged_itemSelectionChanged()
{
    QList<QString> files;

    if (ui->listWidget_staged->isActiveWindow())
    {
        const auto &staged = ui->listWidget_staged->selectedItems();
        for(auto row: staged) {
            files << row->text();
        }

        ui->commit_diff->setGitDiff("", "staged", files);

        const auto &unstaged = ui->listWidget_unstaged->selectedItems();
        for(auto item: unstaged)
        {
            item->setSelected(false);
        }
    }

    m_stageingFiles = false;
}

void QGitRepository::on_listWidget_unstaged_itemSelectionChanged()
{
    QList<QString> files;

    if (ui->listWidget_unstaged->isActiveWindow())
    {
        const auto &unstaged = ui->listWidget_unstaged->selectedItems();
        for(const auto &row: unstaged) {
            files << row->text();
        }

        ui->commit_diff->setGitDiff("", "unstaged", files);

        const auto &staged = ui->listWidget_staged->selectedItems();
        for(const auto &item: staged)
        {
            item->setSelected(false);
        }
    }

    m_stageingFiles = true;
}

void QGitRepository::deleteTagReply(QGitError error)
{
    if (error.errorCode() != 0) {
        QMessageBox::critical(this, tr("Delete Tag Error"), error.errorString());
    } else {
        refreshData();
    }
}

void QGitRepository::keyPressEvent(QKeyEvent *event)
{
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
                QGitMasterMainWindow::instance()->updateStatusBarText(tr("Dropping stash %1...").arg(fullName));
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

/*    enum QGitFileSort {
        QUnsortedFiles, QFilePathSortFiles, QReversedFilePathSortFiles, QFileNameSortFiles, QReversedFileNameSortFiles, QFileStatusSortFiles, QCheckedUncheckedSortFiles
    };*/


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
        break;
    case QComboBoxGitStatusFiles::QFilePathSortFiles:
        break;
    case QComboBoxGitStatusFiles::QReversedFilePathSortFiles:
        break;
    case QComboBoxGitStatusFiles::QFileNameSortFiles:
        break;
    case QComboBoxGitStatusFiles::QReversedFileNameSortFiles:
        break;
    case QComboBoxGitStatusFiles::QFileStatusSortFiles:
        break;
    case QComboBoxGitStatusFiles::QCheckedUncheckedSortFiles:
        break;
    }

    ui->listWidget_staged->setEnabled(false);
    ui->listWidget_unstaged->setEnabled(false);
    emit repositoryChangedFiles(show, sort, reversed);
}
