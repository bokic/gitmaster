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
#include <qgitbranch.h>

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
#include <QInputDialog>
#include <QList>
#include <QFile>
#include <QTextStream>

#define COMMIT_COUNT_TO_LOAD 100


#include <QLineEdit>
#include <QStyledItemDelegate>

class QGitBranchTreeItemDelegate : public QStyledItemDelegate
{
public:
    explicit QGitBranchTreeItemDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QWidget *editor = QStyledItemDelegate::createEditor(parent, option, index);
        QLineEdit *lineEdit = qobject_cast<QLineEdit *>(editor);
        if (lineEdit) {
            QString text = lineEdit->text();
            int suffixIndex = text.indexOf(QStringLiteral(" [↑"));
            if (suffixIndex == -1) {
                suffixIndex = text.indexOf(QStringLiteral(" [↓"));
            }
            if (suffixIndex != -1) {
                lineEdit->setText(text.left(suffixIndex).trimmed());
            }
        }
        return editor;
    }
};

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
    , m_git(new QGit())
{
    QString name;
    QString email;

    ui->setupUi(this);

    ui->commit_diff->setContentsMargins(10, 10, 10, 10);

    ui->logHistory_commits->setItemDelegate(new QLogHistoryItemDelegate(ui->logHistory_commits));
    ui->branchesTreeView->setItemDelegate(new QGitBranchTreeItemDelegate(this));

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
    ui->search_files->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->logHistory_commits->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->search_commits->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

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

    ui->listWidget_unstaged->setContextMenuPolicy(Qt::CustomContextMenu);
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

    m_thread.setObjectName("QGit(repo)");
    m_thread.start();

    m_git->setPath(QDir(m_path));

    connect(ui->comboBox_gitDiffOptions, &QComboBoxGitDiffOptions::optionsChanged, this, &QGitRepository::on_comboBox_gitDiffOptions_optionsChanged);
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

    if (ui->repositoryDetail->currentIndex() != 0)
    {
        fetchRepositoryChangedFiles();
    }

    emit repositoryBranches();
    emit repositoryStashes();

    auto *mainWindow = QGitMasterMainWindow::instance();
    if (mainWindow)
    {
        mainWindow->updateRemoteActions(this);
    }
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
        QString remoteName = dlg.remote().name;
        if (remoteName == tr("custom"))
        {
            remoteName = dlg.remote().url;
        }
        QString branchName = dlg.branch();
        bool rebase = dlg.rebase();

        QGitMasterMainWindow::instance()->updateStatusBarText(tr("Pulling..."));
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

        QGitMasterMainWindow::instance()->updateStatusBarText(tr("Pushing..."));
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
            QGitMasterMainWindow::instance()->updateStatusBarText(tr("Merging branch %1...").arg(branchName));
            emit repositoryMerge(branchName);
        }
    }
}

void QGitRepository::repositoryMergeReply(QGitError error)
{
    QGitMasterMainWindow::instance()->clearStatusBarText();

    if (error.errorCode()) {
        QMessageBox::critical(this, tr("Merge Error"), 
                              tr("Could not merge the selected branch. Details: %1").arg(error.errorString()));
    } else {
        refreshData();
    }
}

void QGitRepository::repositoryRebaseReply(QGitError error)
{
    QGitMasterMainWindow::instance()->clearStatusBarText();

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

void QGitRepository::repositoryPullReply(QGitError error)
{
    if (error.errorCode())
    {
        QGitMasterMainWindow::instance()->updateStatusBarText(error.errorString());
        QMessageBox::critical(this, tr("Pull Error"), error.errorString());
    }
    else
    {
        QGitMasterMainWindow::instance()->clearStatusBarText();
        refreshData();
    }
}

void QGitRepository::repositoryUpdateSubmoduleReply(QGitError error)
{
    if (error.errorCode())
    {
        QGitMasterMainWindow::instance()->updateStatusBarText(error.errorString());
        QMessageBox::critical(this, tr("Submodule Update Error"), error.errorString());
    }
    else
    {
        QGitMasterMainWindow::instance()->clearStatusBarText();
        refreshData();
        QMessageBox::information(this, tr("Submodule Updated"), tr("Submodule updated successfully."));
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
    itemRemoteBranches->setData(0, Qt::UserRole + 2, QStringLiteral("RemotesHeader"));


    itemWorkingCopy->setData(0, Qt::UserRole + 2, "WorkingCopy");
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
                            QString displayName = name;
                            if (depth == items.count() - 1)
                            {
                                if (branch.ahead() > 0 || branch.behind() > 0)
                                {
                                    QStringList indicators;
                                    if (branch.ahead() > 0) {
                                        indicators << QStringLiteral("↑%1").arg(branch.ahead());
                                    }
                                    if (branch.behind() > 0) {
                                        indicators << QStringLiteral("↓%1").arg(branch.behind());
                                    }
                                    displayName += QStringLiteral(" [%1]").arg(indicators.join(QStringLiteral(" ")));
                                }
                            }

                            QTreeWidgetItem *child = new QTreeWidgetItem(QStringList() << displayName);
                            child->setData(0, Qt::UserRole, branch.hash());
                            if (depth == items.count() - 1) {
                                child->setData(0, Qt::UserRole + 1, items.mid(2).join('/'));
                                child->setData(0, Qt::UserRole + 2, "LocalBranch");
                                child->setFlags(child->flags() | Qt::ItemIsEditable);
                            }

                            bool isCurrent = false;
                            if (depth == items.count() - 1 && items.mid(2).join('/') == current_branch) {
                                isCurrent = true;
                            }

                            if (isCurrent)
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
        child->setFlags(child->flags() | Qt::ItemIsEditable);

        child->setIcon(0, m_iconTag);
        itemTags->addChild(child);
    }

    QList<QGitSubmodule> submods;
    try {
        submods = m_git->submodules();
    } catch (...) {}

    QTreeWidgetItem *itemSubmodules = new QTreeWidgetItem(QStringList() << tr("Submodules"));
    for (const auto &sub : submods) {
        QString label = sub.name;
        QString statusText;
        if (sub.status & GIT_SUBMODULE_STATUS_WD_UNINITIALIZED) {
            statusText = tr(" [Uninitialized]");
        } else if (sub.status & (GIT_SUBMODULE_STATUS_WD_ADDED | GIT_SUBMODULE_STATUS_WD_DELETED | 
                                 GIT_SUBMODULE_STATUS_WD_MODIFIED | GIT_SUBMODULE_STATUS_WD_INDEX_MODIFIED | 
                                 GIT_SUBMODULE_STATUS_WD_WD_MODIFIED)) {
            statusText = tr(" [Dirty]");
        }

        QTreeWidgetItem *child = new QTreeWidgetItem(QStringList() << (label + statusText));
        child->setData(0, Qt::UserRole, sub.name);
        child->setData(0, Qt::UserRole + 1, sub.path);
        child->setData(0, Qt::UserRole + 2, QStringLiteral("Submodule"));
        child->setIcon(0, m_iconSubmodule);
        itemSubmodules->addChild(child);
    }

    items.append(itemFileStatus);
    items.append(itemLocalBranches);
    items.append(itemTags);
    items.append(itemRemoteBranches);
    if (itemSubmodules->childCount() > 0) {
        items.append(itemSubmodules);
    } else {
        delete itemSubmodules;
    }

    ui->branchesTreeView->clear();
    ui->branchesTreeView->addTopLevelItems(items);
    ui->branchesTreeView->expandAll();

    ui->logHistory_commits->setReferences(branches, tags, current_branch);
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
    Q_UNUSED(error)
    int stagedCnt = 0;
    int unstagedCnt = 0;

    ui->listWidget_staged->blockSignals(true);
    ui->listWidget_unstaged->blockSignals(true);

    ui->listWidget_staged->setEnabled(true);
    ui->listWidget_unstaged->setEnabled(true);

    for(int c = 0; c < files.count(); c++)
    {
        const QString &file = files.at(c).first;
        git_status_t status = files.at(c).second;

        // --- Staged Reconciliation ---
        if (status & (GIT_STATUS_INDEX_NEW | GIT_STATUS_INDEX_MODIFIED | GIT_STATUS_INDEX_DELETED | GIT_STATUS_INDEX_RENAMED | GIT_STATUS_INDEX_TYPECHANGE))
        {
            QListWidgetItem *item = nullptr;
            for (int i = stagedCnt; i < ui->listWidget_staged->count(); ++i) {
                if (ui->listWidget_staged->item(i)->text() == file) {
                    while (stagedCnt < i) {
                        delete ui->listWidget_staged->takeItem(stagedCnt);
                        i--;
                    }
                    item = ui->listWidget_staged->item(stagedCnt);
                    break;
                }
            }
            if (!item) {
                item = new QListWidgetItem(file);
                item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
                item->setCheckState(Qt::Checked);
                ui->listWidget_staged->insertItem(stagedCnt, item);
            }

            item->setData(Qt::UserRole, (int)status);

            if (status & GIT_STATUS_INDEX_NEW) item->setIcon(m_iconFileNew);
            else if (status & GIT_STATUS_INDEX_DELETED) item->setIcon(m_iconFileRemoved);
            else if (status & GIT_STATUS_INDEX_RENAMED) item->setIcon(m_iconFileRenamed);
            else item->setIcon(m_iconFileModified);

            stagedCnt++;
        }

        // --- Unstaged Reconciliation ---
        if ((status == GIT_STATUS_CURRENT) || (status & (GIT_STATUS_WT_NEW | GIT_STATUS_WT_MODIFIED | GIT_STATUS_WT_DELETED | GIT_STATUS_WT_TYPECHANGE | GIT_STATUS_WT_RENAMED | GIT_STATUS_WT_UNREADABLE | GIT_STATUS_IGNORED | GIT_STATUS_CONFLICTED)))
        {
            QListWidgetItem *item = nullptr;
            for (int i = unstagedCnt; i < ui->listWidget_unstaged->count(); ++i) {
                if (ui->listWidget_unstaged->item(i)->text() == file) {
                    while (unstagedCnt < i) {
                        delete ui->listWidget_unstaged->takeItem(unstagedCnt);
                        i--;
                    }
                    item = ui->listWidget_unstaged->item(unstagedCnt);
                    break;
                }
            }
            if (!item) {
                item = new QListWidgetItem(file);
                item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
                item->setCheckState(Qt::Unchecked);
                ui->listWidget_unstaged->insertItem(unstagedCnt, item);
            }

            item->setData(Qt::UserRole, (int)status);

            uint32_t wt_status = status & (GIT_STATUS_CURRENT | GIT_STATUS_WT_NEW | GIT_STATUS_WT_MODIFIED | GIT_STATUS_WT_DELETED | GIT_STATUS_IGNORED | GIT_STATUS_CONFLICTED | GIT_STATUS_WT_RENAMED);
            if (wt_status == GIT_STATUS_CURRENT) item->setIcon(m_iconFileClean);
            else if (wt_status & GIT_STATUS_WT_NEW) item->setIcon(m_iconFileNew);
            else if (wt_status & GIT_STATUS_WT_MODIFIED) item->setIcon(m_iconFileModified);
            else if (wt_status & GIT_STATUS_WT_DELETED) item->setIcon(m_iconFileRemoved);
            else if (wt_status & GIT_STATUS_WT_RENAMED) item->setIcon(m_iconFileRenamed);
            else if (wt_status & GIT_STATUS_IGNORED) item->setIcon(m_iconFileIgnored);
            else if (wt_status & GIT_STATUS_CONFLICTED) item->setIcon(m_iconFileConflict);
            else item->setIcon(m_iconFileUnknown);

            unstagedCnt++;
        }
    }

    while(ui->listWidget_staged->count() > stagedCnt)
    {
        delete ui->listWidget_staged->takeItem(stagedCnt);
    }
    while(ui->listWidget_unstaged->count() > unstagedCnt)
    {
        delete ui->listWidget_unstaged->takeItem(unstagedCnt);
    }

    ui->listWidget_staged->blockSignals(false);
    ui->listWidget_unstaged->blockSignals(false);

    ui->commit_diff->refresh();

    if (ui->listWidget_staged->count() > 0 || ui->checkBox_amendCommit->isChecked())
    {
        ui->pushButton_commit->setEnabled(true);
    }
    else
    {
        ui->pushButton_commit->setEnabled(false);
    }

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

    auto *mainWindow = QGitMasterMainWindow::instance();
    if (mainWindow)
    {
        mainWindow->setStashEnabled(this, hasStashableChanges);
        mainWindow->refreshRepositoryTree();
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

void QGitRepository::repositoryDiscardFilesReply(QGitError error)
{
    if (error.errorCode() != 0) {
        QMessageBox::critical(this, tr("Discard Error"), error.errorString());
    }
    fetchRepositoryChangedFiles();
}

void QGitRepository::repositoryCommitReply(QString commit_id, QGitError error)
{
    Q_UNUSED(commit_id)

    ui->plainTextEdit_commitMessage->setEnabled(true);
    ui->pushButton_commit->setEnabled(true);

    if (error.errorCode() != 0) {
        QMessageBox::critical(this, tr("Commit Error"), error.errorString());
    } else {
        ui->plainTextEdit_commitMessage->clear();
        ui->checkBox_amendCommit->setChecked(false);
        m_draftCommitMessage.clear();
        refreshData();
    }
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
            html += m_commitDiff.message();
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
            html += m_commitDiff.message();
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

    QString type = item->data(0, Qt::UserRole + 2).toString();
    if (type == "Submodule")
    {
        QString subPath = item->data(0, Qt::UserRole + 1).toString();
        QString fullSubPath = m_git->path().absoluteFilePath(subPath);
        QGitMasterMainWindow::instance()->openRepository(fullSubPath);
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

    if (type == "WorkingCopy")
    {
        QMenu menu(this);
        QAction *applyPatchAction = menu.addAction(tr("Apply Patch..."));
        QAction *cleanAction = menu.addAction(tr("Clean Working Directory..."));
        menu.addSeparator();
        QAction *showReflogAction = menu.addAction(tr("Show Reference Log (Reflog)..."));

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
                QGitMasterMainWindow::instance()->updateStatusBarText(tr("Applying patch..."));
                emit repositoryApplyPatch(patchPath);
            }
        }
        else if (selectedAction == cleanAction)
        {
            QGitCleanDialog dlg(this);
            if (dlg.exec() == QDialog::Accepted) {
                QGitMasterMainWindow::instance()->updateStatusBarText(tr("Cleaning working directory..."));
                emit repositoryClean(dlg.removeIgnored(), dlg.removeDirectories());
            }
        }
        else if (selectedAction == showReflogAction)
        {
            QGitReflogDialog dlg(QStringLiteral("HEAD"), this, this);
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
            QGitMasterMainWindow::instance()->openRepository(fullSubPath);
        }
        else if (selectedAction == updateAction)
        {
            QGitMasterMainWindow::instance()->updateStatusBarText(tr("Updating submodule %1...").arg(subName));
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
            QGitMasterMainWindow::instance()->updateStatusBarText(tr("Checking out branch %1...").arg(fullName));
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

void QGitRepository::renameTagReply(QGitError error)
{
    if (error.errorCode())
    {
        QMessageBox::critical(this, tr("Rename Tag Error"), error.errorString());
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
        if (ui->listWidget_staged->count() > 0)
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
    bool ignoreWhitespace = ui->comboBox_gitDiffOptions->ignoreWhitespace();
    QTableWidgetItem *item = ui->logHistory_files->currentItem();

    if (item && item->column() == 0)
    {
        ui->logHistory_diff->setIgnoreWhitespace(ignoreWhitespace);
        ui->logHistory_diff->setGitDiff(m_commitDiff.parents().at(0).commitHash(), m_commitDiff.id(), {item->data(Qt::UserRole).toString()});
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

void QGitRepository::on_listWidget_staged_itemSelectionChanged()
{
    QList<QString> files;
    QMap<QString, git_status_t> statuses;

    if (ui->listWidget_staged->isActiveWindow())
    {
        const auto &staged = ui->listWidget_staged->selectedItems();
        for(auto row: staged) {
            files << row->text();
            statuses.insert(row->text(), (git_status_t)row->data(Qt::UserRole).toInt());
        }

        ui->commit_diff->setGitDiff("", "staged", files, statuses);

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
    QMap<QString, git_status_t> statuses;

    if (ui->listWidget_unstaged->isActiveWindow())
    {
        const auto &unstaged = ui->listWidget_unstaged->selectedItems();
        for(const auto &row: unstaged) {
            files << row->text();
            statuses.insert(row->text(), (git_status_t)row->data(Qt::UserRole).toInt());
        }

        ui->commit_diff->setGitDiff("", "unstaged", files, statuses);

        const auto &staged = ui->listWidget_staged->selectedItems();
        for(auto item: staged)
        {
            item->setSelected(false);
        }
    }

    m_stageingFiles = true;
}

void QGitRepository::on_listWidget_unstaged_customContextMenuRequested(const QPoint &pos)
{
    const auto &selected = ui->listWidget_unstaged->selectedItems();

    QMenu menu(this);
    QAction *resolveAction = nullptr;
    QAction *discardAction = nullptr;
    QAction *ignoreAction = nullptr;

    if (!selected.isEmpty()) {
        uint32_t status = selected.first()->data(Qt::UserRole).toUInt();
        bool hasConflict = (status & GIT_STATUS_CONFLICTED);
        bool isUntracked = (status & GIT_STATUS_WT_NEW);

        if (hasConflict) {
            resolveAction = menu.addAction(tr("Resolve Conflicts..."));
        }
        discardAction = menu.addAction(tr("Discard changes"));

        if (isUntracked && selected.size() == 1) {
            menu.addSeparator();
            ignoreAction = menu.addAction(tr("Add to .gitignore..."));
        }
        menu.addSeparator();
    }

    QAction *cleanAction = menu.addAction(tr("Clean Working Directory..."));

    QAction *res = menu.exec(ui->listWidget_unstaged->mapToGlobal(pos));
    if (resolveAction && res == resolveAction) {
        QGitConflictResolverDialog dlg(m_git, selected.first()->text(), this);
        if (dlg.exec() == QDialog::Accepted) {
            refreshData();
        }
    } else if (discardAction && res == discardAction) {
        auto confirm = QMessageBox::question(this, tr("Discard changes"),
                                             tr("Are you sure you want to discard changes in the selected files? This operation cannot be undone."),
                                             QMessageBox::Yes | QMessageBox::No);
        if (confirm == QMessageBox::Yes) {
            QStringList files;
            for (auto item : selected) {
                files << item->text();
            }
            emit repositoryDiscardFiles(files);
        }
    } else if (ignoreAction && res == ignoreAction) {
        QString filePath = selected.first()->text();
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
                // Ensure we start on a new line if the file already has content
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
            QGitMasterMainWindow::instance()->updateStatusBarText(tr("Cleaning working directory..."));
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
    QString targetName = selectedHash.left(7);
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
        cherrypickAction = menu.addAction(tr("Cherry-pick commit '%1'").arg(selectedHash.left(7)));
    }

    revertAction = menu.addAction(tr("Revert commit '%1'").arg(selectedHash.left(7)));

    QMenu *resetMenu = new QMenu(tr("Reset current branch to '%1'").arg(selectedHash.left(7)), &menu);
    QAction *softResetAction = resetMenu->addAction(tr("Soft (keep changes staged)"));
    QAction *mixedResetAction = resetMenu->addAction(tr("Mixed (keep changes unstaged)"));
    QAction *hardResetAction = resetMenu->addAction(tr("Hard (discard all changes)"));
    menu.addMenu(resetMenu);

    menu.addSeparator();
    QAction *createTagAction = menu.addAction(tr("Create Tag at '%1'...").arg(selectedHash.left(7)));
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
                QGitMasterMainWindow::instance()->updateStatusBarText(tr("Rebasing current branch..."));
                emit repositoryRebase(selectedHash);
            }
        } else if (res == cherrypickAction) {
            auto confirm = QMessageBox::question(this, tr("Cherry-pick"),
                                                 tr("Are you sure you want to cherry-pick commit '%1' onto the current branch?").arg(selectedHash.left(7)),
                                                 QMessageBox::Yes | QMessageBox::No);
            if (confirm == QMessageBox::Yes) {
                QGitMasterMainWindow::instance()->updateStatusBarText(tr("Cherry-picking commit..."));
                try {
                    m_git->cherrypick(selectedHash);
                    QMessageBox::information(this, tr("Cherry-pick"), tr("Commit '%1' cherry-picked successfully.").arg(selectedHash.left(7)));
                } catch (const QGitError &error) {
                    QMessageBox::warning(this, tr("Cherry-pick"), error.errorString());
                }
                QGitMasterMainWindow::instance()->updateStatusBarText(tr("Ready"));
                refreshData();
            }
        } else if (res == revertAction) {
            auto confirm = QMessageBox::question(this, tr("Revert"),
                                                 tr("Are you sure you want to revert commit '%1'? This will apply the inverse changes onto your current branch.").arg(selectedHash.left(7)),
                                                 QMessageBox::Yes | QMessageBox::No);
            if (confirm == QMessageBox::Yes) {
                QGitMasterMainWindow::instance()->updateStatusBarText(tr("Reverting commit..."));
                try {
                    m_git->revert(selectedHash);
                    QMessageBox::information(this, tr("Revert"), tr("Commit '%1' reverted successfully.").arg(selectedHash.left(7)));
                } catch (const QGitError &error) {
                    QMessageBox::warning(this, tr("Revert"), error.errorString());
                }
                QGitMasterMainWindow::instance()->updateStatusBarText(tr("Ready"));
                refreshData();
            }
        } else if (res == softResetAction) {
            auto confirm = QMessageBox::question(this, tr("Soft Reset"),
                                                 tr("Are you sure you want to perform a soft reset to '%1'?\n\nThis will move the branch pointer, but keep all modifications staged.").arg(selectedHash.left(7)),
                                                 QMessageBox::Yes | QMessageBox::No);
            if (confirm == QMessageBox::Yes) {
                QGitMasterMainWindow::instance()->updateStatusBarText(tr("Resetting current branch (soft)..."));
                try {
                    m_git->reset(selectedHash, GIT_RESET_SOFT);
                    QMessageBox::information(this, tr("Reset"), tr("Branch reset to '%1' successfully (soft).").arg(selectedHash.left(7)));
                } catch (const QGitError &error) {
                    QMessageBox::critical(this, tr("Reset Error"), error.errorString());
                }
                QGitMasterMainWindow::instance()->updateStatusBarText(tr("Ready"));
                refreshData();
            }
        } else if (res == mixedResetAction) {
            auto confirm = QMessageBox::question(this, tr("Mixed Reset"),
                                                 tr("Are you sure you want to perform a mixed reset to '%1'?\n\nThis will move the branch pointer and unstage changes, but keep them in your working directory.").arg(selectedHash.left(7)),
                                                 QMessageBox::Yes | QMessageBox::No);
            if (confirm == QMessageBox::Yes) {
                QGitMasterMainWindow::instance()->updateStatusBarText(tr("Resetting current branch (mixed)..."));
                try {
                    m_git->reset(selectedHash, GIT_RESET_MIXED);
                    QMessageBox::information(this, tr("Reset"), tr("Branch reset to '%1' successfully (mixed).").arg(selectedHash.left(7)));
                } catch (const QGitError &error) {
                    QMessageBox::critical(this, tr("Reset Error"), error.errorString());
                }
                QGitMasterMainWindow::instance()->updateStatusBarText(tr("Ready"));
                refreshData();
            }
        } else if (res == hardResetAction) {
            auto confirm = QMessageBox::question(this, tr("Hard Reset"),
                                                 tr("WARNING: Are you sure you want to perform a hard reset to '%1'?\n\nThis will move the branch pointer and discard ALL staged and unstaged changes. This cannot be undone!").arg(selectedHash.left(7)),
                                                 QMessageBox::Yes | QMessageBox::No);
            if (confirm == QMessageBox::Yes) {
                QGitMasterMainWindow::instance()->updateStatusBarText(tr("Resetting current branch (hard)..."));
                try {
                    m_git->reset(selectedHash, GIT_RESET_HARD);
                    QMessageBox::information(this, tr("Reset"), tr("Branch reset to '%1' successfully (hard).").arg(selectedHash.left(7)));
                } catch (const QGitError &error) {
                    QMessageBox::critical(this, tr("Reset Error"), error.errorString());
                }
                QGitMasterMainWindow::instance()->updateStatusBarText(tr("Ready"));
                refreshData();
            }
        } else if (res == createTagAction) {
            QGitCreateTagDialog dlg(selectedHash, this);
            if (dlg.exec() == QDialog::Accepted) {
                QGitMasterMainWindow::instance()->updateStatusBarText(tr("Creating tag..."));
                emit repositoryCreateTag(dlg.tagName(), selectedHash, dlg.tagMessage(), dlg.forceCreate());
            }
        } else if (res == editNoteAction) {
            QString currentNote = m_git->getNote(selectedHash);
            bool ok;
            QString noteText = QInputDialog::getMultiLineText(
                this,
                tr("Edit Git Note"),
                tr("Enter note text for commit %1:").arg(selectedHash.left(7)),
                currentNote,
                &ok
            );
            if (ok) {
                QGitMasterMainWindow::instance()->updateStatusBarText(tr("Setting Git Note..."));
                emit repositorySetNote(selectedHash, noteText);
            }
        } else if (res == deleteNoteAction) {
            auto confirm = QMessageBox::question(this, tr("Delete Git Note"),
                                                 tr("Are you sure you want to delete the Git Note for commit %1?").arg(selectedHash.left(7)),
                                                 QMessageBox::Yes | QMessageBox::No);
            if (confirm == QMessageBox::Yes) {
                QGitMasterMainWindow::instance()->updateStatusBarText(tr("Deleting Git Note..."));
                emit repositoryRemoveNote(selectedHash);
            }
        }
    }
}

void QGitRepository::deleteTagReply(QGitError error)
{
    if (error.errorCode() != 0) {
        QMessageBox::critical(this, tr("Delete Tag Error"), error.errorString());
    } else {
        refreshData();
    }
}

void QGitRepository::createTagReply(QGitError error)
{
    QGitMasterMainWindow::instance()->updateStatusBarText(tr("Ready"));
    if (error.errorCode() != 0) {
        QMessageBox::critical(this, tr("Create Tag Error"), error.errorString());
    } else {
        QMessageBox::information(this, tr("Tag Created"), tr("Tag created successfully."));
        refreshData();
    }
}

void QGitRepository::repositoryCleanReply(QGitError error)
{
    QGitMasterMainWindow::instance()->updateStatusBarText(tr("Ready"));
    if (error.errorCode() != 0) {
        QMessageBox::critical(this, tr("Clean Error"), error.errorString());
    } else {
        QMessageBox::information(this, tr("Clean Successful"), tr("Working directory cleaned successfully."));
        refreshData();
    }
}

void QGitRepository::repositoryApplyPatchReply(QGitError error)
{
    QGitMasterMainWindow::instance()->updateStatusBarText(tr("Ready"));
    if (error.errorCode() != 0) {
        QMessageBox::critical(this, tr("Apply Patch Error"), error.errorString());
    } else {
        QMessageBox::information(this, tr("Apply Patch Successful"), tr("Patch applied successfully."));
        refreshData();
    }
}

void QGitRepository::repositorySetNoteReply(QGitError error)
{
    QGitMasterMainWindow::instance()->updateStatusBarText(tr("Ready"));
    if (error.errorCode() != 0) {
        QMessageBox::critical(this, tr("Note Error"), error.errorString());
    } else {
        int currentRow = ui->logHistory_commits->currentRow();
        if (currentRow >= 0) {
            on_logHistory_commits_currentCellChanged(currentRow, 0, 0, 0);
        }
    }
}

void QGitRepository::repositoryRemoveNoteReply(QGitError error)
{
    QGitMasterMainWindow::instance()->updateStatusBarText(tr("Ready"));
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

        QGitMasterMainWindow::instance()->updateStatusBarText(tr("Searching commits..."));
        emit repositorySearchCommits(text, type);
    }
}

void QGitRepository::onCommitFound(QGitCommit commit)
{
    ui->search_commits->addCommit(commit);
}

void QGitRepository::onSearchFinished()
{
    m_searchingCommits = false;
    QGitMasterMainWindow::instance()->clearStatusBarText();
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
        QGitMasterMainWindow::instance()->updateStatusBarText(tr("Renaming branch %1 to %2...").arg(oldFullName, newFullName));
        emit repositoryRenameBranch(oldFullName, newFullName);
    }
    else if (type == "Tag")
    {
        QGitMasterMainWindow::instance()->updateStatusBarText(tr("Renaming tag %1 to %2...").arg(oldFullName, newFullName));
        emit repositoryRenameTag(oldFullName, newFullName);
    }
}
