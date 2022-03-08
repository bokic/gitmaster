#include "qgitmastermainwindow.h"
#include "ui_qgitmastermainwindow.h"
#include "qloghistoryitemdelegate.h"
#include "qgitrepository.h"
#include "ui_qgitrepository.h"
#include <QCryptographicHash>
#include <QTableWidgetItem>
#include <QTreeWidgetItem>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QMessageBox>
#include <QScrollBar>
#include <QWindow>
#include <QList>

#define COMMIT_COUNT_TO_LOAD 100


QGitRepository::QGitRepository(const QString &path, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::QGitRepository)
    , m_path(path)
    , m_allCommitsLoaded(false)
    , m_git(new QGit())
{
    QString name;
    QString email;

    ui->setupUi(this);

    ui->logHistory_commits->setItemDelegate(new QLogHistoryItemDelegate());

    m_git->moveToThread(&m_thread);

    ui->splitter_3->setStretchFactor(0, 1);
    ui->splitter_3->setStretchFactor(1, 1);
    ui->splitter_3->setSizes(QList<int>() << 100000 << 100000);

    ui->logHistory_splitter_2->setStretchFactor(0, 1);
    ui->logHistory_splitter_2->setStretchFactor(1, 1);
    ui->logHistory_splitter_2->setSizes(QList<int>() << 100000 << 100000);

    if (QGit::gitRepositoryDefaultSignature(m_path, name, email))
    {
        QByteArray hash;
        QNetworkReply *reply = nullptr;

        hash = QCryptographicHash::hash(email.trimmed().toLatin1(), QCryptographicHash::Md5).toHex();

        QString urlStr = QString("http://www.gravatar.com/avatar/%1?s=24").arg(QString::fromLatin1(hash));
        auto url = QUrl(urlStr);
        auto req = QNetworkRequest(url);
        reply = m_networkManager.get(req);

        connect(reply, SIGNAL(finished()), this, SLOT(gravatarImageDownloadFinished()));
        reply = nullptr;

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
    ui->logHistory_commits->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    activateCommitOperation(false);

    connect(this, SIGNAL(localStash(QString)), m_git, SLOT(stashSave(QString)), Qt::QueuedConnection);
    connect(m_git, SIGNAL(stashSaveReply(QGitError)), this, SLOT(localStashSaveReply(QGitError)), Qt::QueuedConnection);
    connect(this, SIGNAL(repositoryBranches()), m_git, SLOT(listBranchesAndTags()), Qt::QueuedConnection);
    connect(m_git, SIGNAL(listBranchesAndTagsReply(QList<QGitBranch>,QList<QString>,QGitError)), this, SLOT(repositoryBranchesAndTagsReply(QList<QGitBranch>,QList<QString>,QGitError)), Qt::QueuedConnection);

    connect(this, SIGNAL(repositoryStashes()), m_git, SLOT(listStashes()), Qt::QueuedConnection);
    connect(m_git, SIGNAL(listStashesReply(QStringList,QGitError)), this, SLOT(repositoryStashesReply(QStringList,QGitError)), Qt::QueuedConnection);

    connect(this, SIGNAL(repositoryChangedFiles(int,int,bool)), m_git, SLOT(listChangedFiles(int,int,bool)), Qt::QueuedConnection);
    connect(m_git, SIGNAL(listChangedFilesReply(QMap<QString,git_status_t>,QGitError)), this, SLOT(repositoryChangedFilesReply(QMap<QString,git_status_t>,QGitError)), Qt::QueuedConnection);

    connect(this, SIGNAL(repositoryStageFiles(QStringList)), m_git, SLOT(stageFiles(QStringList)), Qt::QueuedConnection);
    connect(m_git, SIGNAL(stageFilesReply(QGitError)), this, SLOT(repositoryStageFilesReply(QGitError)), Qt::QueuedConnection);

    connect(this, SIGNAL(repositoryUnstageFiles(QStringList)), m_git, SLOT(unstageFiles(QStringList)), Qt::QueuedConnection);
    connect(m_git, SIGNAL(unstageFilesReply(QGitError)), this, SLOT(repositoryUnstageFilesReply(QGitError)), Qt::QueuedConnection);

    connect(this, SIGNAL(repositoryCommit(QString)), m_git, SLOT(commit(QString)), Qt::QueuedConnection);
    connect(m_git, SIGNAL(commitReply(QString,QGitError)), this, SLOT(repositoryCommitReply(QString,QGitError)), Qt::QueuedConnection);

    connect(this, SIGNAL(repositoryGetCommits(QString,int)), m_git, SLOT(listCommits(QString,int)));
    connect(m_git, SIGNAL(listCommitsReply(QList<QGitCommit>,QGitError)), this, SLOT(repositoryGetCommitsReply(QList<QGitCommit>,QGitError)));

    connect(this, SIGNAL(repositoryGetCommitDiff(QString)), m_git, SLOT(commitDiff(QString)));
    connect(m_git, SIGNAL(commitDiffReply(QString,QGitCommit,QGitError)), this, SLOT(repositoryGetCommitDiffReply(QString,QGitCommit,QGitError)));

    connect(ui->commit_diff, SIGNAL(select(QString,QVector<QGitDiffWidgetLine>)), this, SLOT(selectedLines(QString,QVector<QGitDiffWidgetLine>)));

    connect(this, SIGNAL(stageFileLines(QString,QVector<QGitDiffWidgetLine>)), m_git, SLOT(stageFileLines(QString,QVector<QGitDiffWidgetLine>)));
    connect(this, SIGNAL(unstageFileLines(QString,QVector<QGitDiffWidgetLine>)), m_git, SLOT(unstageFileLines(QString,QVector<QGitDiffWidgetLine>)));

    connect(ui->logHistory_commits->verticalScrollBar(), SIGNAL(sliderMoved(int)), this, SLOT(historyTableSliderMoved(int)));

    m_thread.start();

    m_git->setPath(QDir(m_path));
    emit repositoryBranches();
    emit repositoryStashes();
}

QGitRepository::~QGitRepository()
{
    QAbstractItemDelegate *delegate = nullptr;

    m_thread.quit();
    m_thread.wait();

    delegate = ui->logHistory_commits->itemDelegate();

    delete m_git; m_git = nullptr;

    delete ui;

    delete delegate;
}

void QGitRepository::refreshData()
{
    on_repositoryDetail_currentChanged(ui->repositoryDetail->currentIndex());
}

void QGitRepository::stash(const QString &name)
{
    m_git->stashSave(name);
}

void QGitRepository::gravatarImageDownloadFinished()
{
    auto reply = dynamic_cast<QNetworkReply *>(sender());

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

    } else {
        emit repositoryStashes();
    }
}

void QGitRepository::repositoryBranchesAndTagsReply(QList<QGitBranch> branches, QList<QString> tags, QGitError error)
{
    Q_UNUSED(error)

    QList<QTreeWidgetItem *> items;
    QTreeWidgetItem *itemFileStatus = new QTreeWidgetItem(QStringList() << tr("File Status"));
    QTreeWidgetItem *itemLocalBranches = new QTreeWidgetItem(QStringList() << tr("Branches"));
    QTreeWidgetItem *itemTags = new QTreeWidgetItem(QStringList() << tr("Tags"));
    QTreeWidgetItem *itemRemoteBranches = new QTreeWidgetItem(QStringList() << tr("Remotes"));

    itemFileStatus->addChild(new QTreeWidgetItem(QStringList() << tr("Working Copy")));

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
                        item->addChild(child);
                        item = child;
                    }
                }
            }
        }
    }

    for(const auto &tag: tags)
    {
        QTreeWidgetItem *child = new QTreeWidgetItem(QStringList() << tag);

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

    if (!stashes.isEmpty())
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(QStringList() << tr("Stashes"));

        for(const auto &stash: stashes)
        {
            item->addChild(new QTreeWidgetItem(QStringList() << stash));
        }

        ui->branchesTreeView->addTopLevelItem(item);

        ui->branchesTreeView->expandAll();
    }
}

void QGitRepository::repositoryChangedFilesReply(QMap<QString, git_status_t> files, QGitError error)
{
    QListWidgetItem *item = nullptr;

    Q_UNUSED(error)

    ui->listWidget_staged->clear();
    ui->listWidget_unstaged->clear();
    ui->listWidget_staged->setEnabled(true);
    ui->listWidget_unstaged->setEnabled(true);


    QIcon icon_file_new = QIcon(":/images/file_new.svg");
    QIcon icon_file_modified = QIcon(":/images/file_modified.svg");
    QIcon icon_file_removed = QIcon(":/images/file_removed.svg");
    QIcon icon_file_unknown = QIcon(":/images/file_unknown.svg");

    QMapIterator<QString, git_status_t> i(files);
    while (i.hasNext())
    {
        i.next();

        const QString &file = i.key();
        const git_status_t status = i.value();

        if (status & (GIT_STATUS_INDEX_NEW | GIT_STATUS_INDEX_MODIFIED | GIT_STATUS_INDEX_DELETED | GIT_STATUS_INDEX_RENAMED | GIT_STATUS_INDEX_TYPECHANGE))
        {
            item = new QListWidgetItem(file);

            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(Qt::Checked);

            switch(status)
            {
            case GIT_STATUS_INDEX_NEW:
                item->setIcon(icon_file_new);
                break;
            case GIT_STATUS_INDEX_MODIFIED:
                item->setIcon(icon_file_modified);
                break;
            case GIT_STATUS_INDEX_DELETED:
                item->setIcon(icon_file_removed);
                break;
            default:
                item->setIcon(icon_file_unknown);
                break;
            }

            ui->listWidget_staged->addItem(item);
        }

        if ((status == GIT_STATUS_CURRENT)||(status & (GIT_STATUS_WT_NEW | GIT_STATUS_WT_MODIFIED | GIT_STATUS_WT_DELETED | GIT_STATUS_WT_TYPECHANGE | GIT_STATUS_WT_RENAMED | GIT_STATUS_WT_UNREADABLE | GIT_STATUS_IGNORED | GIT_STATUS_CONFLICTED)))
        {
            item = new QListWidgetItem(file);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(Qt::Unchecked);

            switch(status)
            {
            case GIT_STATUS_WT_NEW:
                item->setIcon(icon_file_new);
                break;
            case GIT_STATUS_WT_MODIFIED:
                item->setIcon(icon_file_modified);
                break;
            case GIT_STATUS_WT_DELETED:
                item->setIcon(icon_file_removed);
                break;
            default:
                item->setIcon(icon_file_unknown);
                break;
            }

            ui->listWidget_unstaged->addItem(item);
        }
    }

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

    QGitMasterMainWindow::instance()->updateStatusBarText(QString());
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

        QIcon icon_file_new = QIcon(":/images/file_new.svg");
        QIcon icon_file_modified = QIcon(":/images/file_modified.svg");
        QIcon icon_file_removed = QIcon(":/images/file_removed.svg");
        QIcon icon_file_unknown = QIcon(":/images/file_unknown.svg");

        int currentRow = ui->logHistory_commits->currentRow();

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
        html +=   QStringLiteral("<img src=\"https://www.gravatar.com/avatar/") + QCryptographicHash::hash(email.trimmed().toLatin1(), QCryptographicHash::Md5).toHex() + QStringLiteral("?s=32\" width=\"32\" height=\"32\" style=\"float: right\" />");
        html += QStringLiteral("</div>");
        html += QStringLiteral("<b>Commit:</b> ") + commit_id + QStringLiteral("<br />");
        html += QStringLiteral("<b>Parents:</b> ") + parentsHtml.join(", ") + QStringLiteral("<br />");
        html += QStringLiteral("<b>Date:</b> ") + m_commitDiff.time().toString() + QStringLiteral("<br />");
        html += QStringLiteral("<b>Labels:</b> ") + labelsHtml.join(", ") + QStringLiteral("<br />");
        html += QStringLiteral("<br />");
        html += m_commitDiff.message();

        ui->logHistory_info->setHtml(html);

        ui->logHistory_files->clearContents();

        auto files = m_commitDiff.parents().at(0).files(); // TODO: For now from first parent only!
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
                item_icon = icon_file_new;
                break;
            case GIT_DELTA_DELETED:
                item_icon = icon_file_removed;
                break;
            case GIT_DELTA_MODIFIED:
                item_icon = icon_file_modified;
                break;
            default:
                item_icon = icon_file_unknown; // TODO: Add icons for other statuses.
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

void QGitRepository::on_repositoryDetail_currentChanged(int index)
{
    switch(index) {
    case 0:
        disconnect(ui->logHistory_diff, SIGNAL(requestGitDiff(QString,QString,QList<QString>)), m_git, SLOT(commitDiffContent(QString,QString,QList<QString>)));
        disconnect(m_git, SIGNAL(commitDiffContentReply(QString,QString,QList<QGitDiffFile>,QGitError)), ui->logHistory_diff, SLOT(responseGitDiff(QString,QString,QList<QGitDiffFile>,QGitError)));

        connect(ui->commit_diff, SIGNAL(requestGitDiff(QString,QString,QList<QString>)), m_git, SLOT(commitDiffContent(QString,QString,QList<QString>)));
        connect(m_git, SIGNAL(commitDiffContentReply(QString,QString,QList<QGitDiffFile>,QGitError)), ui->commit_diff, SLOT(responseGitDiff(QString,QString,QList<QGitDiffFile>,QGitError)));

        fetchRepositoryChangedFiles();
        break;
    case 1:
        disconnect(ui->commit_diff, SIGNAL(requestGitDiff(QString,QString,QList<QString>)), m_git, SLOT(commitDiffContent(QString,QString,QList<QString>)));
        disconnect(m_git, SIGNAL(commitDiffContentReply(QString,QString,QList<QGitDiffFile>,QGitError)), ui->commit_diff, SLOT(responseGitDiff(QString,QString,QList<QGitDiffFile>,QGitError)));

        connect(ui->logHistory_diff, SIGNAL(requestGitDiff(QString,QString,QList<QString>)), m_git, SLOT(commitDiffContent(QString,QString,QList<QString>)));
        connect(m_git, SIGNAL(commitDiffContentReply(QString,QString,QList<QGitDiffFile>,QGitError)), ui->logHistory_diff, SLOT(responseGitDiff(QString,QString,QList<QGitDiffFile>,QGitError)));

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

        if (item->isSelected())
        {
            selectedItems.append(item->text());

            delete ui->listWidget_staged->takeItem(row);
        }
    }

    if ((selectedItems.isEmpty())&&(item->checkState() == Qt::Unchecked))
    {
        selectedItems.append(item->text());
    }

    if (!selectedItems.isEmpty())
    {
        emit repositoryUnstageFiles(selectedItems);
    }

    m_stageFiles = true;
}

void QGitRepository::on_listWidget_unstaged_itemChanged(QListWidgetItem *item)
{
    QStringList selectedItems;

    for(int row = ui->listWidget_unstaged->count() - 1; row >= 0; row--)
    {
        QListWidgetItem *item = ui->listWidget_unstaged->item(row);

        if (item->isSelected())
        {
            selectedItems.append(item->text());

            delete ui->listWidget_unstaged->takeItem(row);
        }
    }

    if ((selectedItems.isEmpty())&&(item->checkState() == Qt::Checked))
    {
        selectedItems.append(item->text());
    }

    if (!selectedItems.isEmpty())
    {
        emit repositoryStageFiles(selectedItems);
    }

    m_stageFiles = false;
}

void QGitRepository::on_pushButton_commit_clicked()
{
    ui->plainTextEdit_commitMessage->setEnabled(false);
    ui->pushButton_commit->setEnabled(false);

    emit repositoryCommit(ui->plainTextEdit_commitMessage->toPlainText());
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
    if (m_stageFiles)
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

    auto selected = ui->logHistory_files->selectedItems();
    int parent = 0; // TODO: Parent is hardcoded.

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
        auto selected = ui->listWidget_staged->selectedItems();

        for(auto row: selected) {
            files << row->text();
        }

        ui->commit_diff->setGitDiff("", "staged", files);

        for(auto item: ui->listWidget_unstaged->selectedItems())
        {
            item->setSelected(false);
        }
    }
}

void QGitRepository::on_listWidget_unstaged_itemSelectionChanged()
{
    QList<QString> files;

    if (ui->listWidget_unstaged->isActiveWindow())
    {
        auto selected = ui->listWidget_unstaged->selectedItems();

        for(auto row: selected) {
            files << row->text();
        }

        ui->commit_diff->setGitDiff("", "unstaged", files);

        for(auto item: ui->listWidget_staged->selectedItems())
        {
            item->setSelected(false);
        }
    }
}

void QGitRepository::on_comboBox_gitStatusFiles_itemClicked(QListWidgetItem *item)
{
    Q_UNUSED(item)

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

    ui->listWidget_staged->clear();
    ui->listWidget_unstaged->clear();
    ui->listWidget_staged->setEnabled(false);
    ui->listWidget_unstaged->setEnabled(false);
    emit repositoryChangedFiles(show, sort, reversed);
}
