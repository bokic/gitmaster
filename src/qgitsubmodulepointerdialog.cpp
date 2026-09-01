#include "qgitsubmodulepointerdialog.h"
#include "ui_qgitsubmodulepointerdialog.h"
#include <QPushButton>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QDateTime>
#include <git2.h>

QGitSubmodulePointerDialog::QGitSubmodulePointerDialog(
    const QDir &repoPath,
    const QGitSubmodule &submodule,
    QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::QGitSubmodulePointerDialog)
    , m_repoPath(repoPath)
    , m_submodule(submodule)
{
    ui->setupUi(this);

    // Setup Info Labels
    ui->label_name_val->setText(m_submodule.name);
    ui->label_path_val->setText(m_submodule.path);

    QString headDisplay;
    if (!m_submodule.headId.isEmpty()) {
        headDisplay = m_submodule.headId;
    } else if (!m_submodule.indexId.isEmpty()) {
        headDisplay = m_submodule.indexId + tr(" (in index)");
    } else {
        headDisplay = tr("Uninitialized");
    }
    ui->label_head_val->setText(headDisplay);

    if (!m_submodule.branch.isEmpty()) {
        ui->lineEdit_trackedBranch->setText(m_submodule.branch);
        ui->checkBox_updateTrackedBranch->setChecked(true);
    } else {
        ui->checkBox_updateTrackedBranch->setChecked(false);
        ui->lineEdit_trackedBranch->setEnabled(false);
    }

    // Configure Tables
    ui->branchesTable->setColumnCount(3);
    ui->branchesTable->setHorizontalHeaderLabels(QStringList() << tr("Branch Name") << tr("Type") << tr("Commit ID"));
    ui->branchesTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->branchesTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->branchesTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->branchesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->branchesTable->setSelectionMode(QAbstractItemView::SingleSelection);

    ui->tagsTable->setColumnCount(3);
    ui->tagsTable->setHorizontalHeaderLabels(QStringList() << tr("Tag Name") << tr("Commit ID") << tr("Date"));
    ui->tagsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tagsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->tagsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->tagsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tagsTable->setSelectionMode(QAbstractItemView::SingleSelection);

    ui->commitsTable->setColumnCount(4);
    ui->commitsTable->setHorizontalHeaderLabels(QStringList() << tr("Commit ID") << tr("Message") << tr("Author") << tr("Date"));
    ui->commitsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->commitsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->commitsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->commitsTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    ui->commitsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->commitsTable->setSelectionMode(QAbstractItemView::SingleSelection);

    loadSubmoduleData();
    populateBranchesTable();
    populateTagsTable();
    populateCommitsTable();
    updateButtons();
}

QGitSubmodulePointerDialog::~QGitSubmodulePointerDialog()
{
    delete ui;
}

void QGitSubmodulePointerDialog::loadSubmoduleData()
{
    QDir subDir(m_repoPath.absoluteFilePath(m_submodule.path));
    if (!QGit::isGitRepository(subDir)) {
        m_isInitialized = false;
        return;
    }
    m_isInitialized = true;

    QGit subGit(subDir);
    m_branches = subGit.branches(GIT_BRANCH_ALL);

    // Read tags and commits using libgit2 directly
    git_repository *repo = nullptr;
    if (git_repository_open(&repo, subDir.absolutePath().toUtf8().constData()) == 0) {
        git_strarray tag_names = {nullptr, 0};
        if (git_tag_list(&tag_names, repo) == 0) {
            for (size_t i = 0; i < tag_names.count; ++i) {
                const char *tag_name = tag_names.strings[i];
                QByteArray tagFullName = QByteArray("refs/tags/") + tag_name;
                git_reference *tag_ref = nullptr;
                if (git_reference_lookup(&tag_ref, repo, tagFullName.constData()) == 0) {
                    git_object *obj = nullptr;
                    if (git_reference_peel(&obj, tag_ref, GIT_OBJECT_COMMIT) == 0) {
                        const git_commit *commit = reinterpret_cast<const git_commit *>(obj);
                        git_time_t tag_time = git_commit_time(commit);
                        char oid_str[GIT_OID_HEXSZ + 1];
                        git_oid_tostr(oid_str, sizeof(oid_str), git_object_id(obj));
                        m_tags.append(QGitTag(QString::fromUtf8(tag_name), QString::fromLatin1(oid_str), tag_time));
                        git_object_free(obj);
                    }
                    git_reference_free(tag_ref);
                }
            }
            git_strarray_dispose(&tag_names);
        }

        // Read recent commits
        git_revwalk *walker = nullptr;
        if (git_revwalk_new(&walker, repo) == 0) {
            git_revwalk_sorting(walker, GIT_SORT_TIME | GIT_SORT_TOPOLOGICAL);
            git_revwalk_push_glob(walker, "refs/heads/*");
            git_revwalk_push_glob(walker, "refs/remotes/*");
            git_revwalk_push_head(walker);

            git_oid oid;
            int count = 0;
            while (git_revwalk_next(&oid, walker) == 0 && count < 200) {
                git_commit *commit = nullptr;
                if (git_commit_lookup(&commit, repo, &oid) == 0) {
                    char idBuf[GIT_OID_HEXSZ + 1];
                    git_oid_tostr(idBuf, sizeof(idBuf), &oid);
                    QString hash = QString::fromLatin1(idBuf);

                    const git_signature *authorSig = git_commit_author(commit);
                    QDateTime authorTime = authorSig ? QDateTime::fromSecsSinceEpoch(authorSig->when.time) : QDateTime();
                    QGitSignature author(
                        authorSig ? QString::fromUtf8(authorSig->name) : QString(),
                        authorSig ? QString::fromUtf8(authorSig->email) : QString(),
                        authorTime
                    );
                    const git_signature *committerSig = git_commit_committer(commit);
                    QDateTime committerTime = committerSig ? QDateTime::fromSecsSinceEpoch(committerSig->when.time) : QDateTime();
                    QGitSignature committer(
                        committerSig ? QString::fromUtf8(committerSig->name) : QString(),
                        committerSig ? QString::fromUtf8(committerSig->email) : QString(),
                        committerTime
                    );

                    git_time_t timeSec = git_commit_time(commit);
                    QDateTime dateTime = QDateTime::fromSecsSinceEpoch(timeSec);
                    const char *rawMsg = git_commit_message(commit);
                    QString fullMsg = rawMsg ? QString::fromUtf8(rawMsg) : QString();
                    QString summary = fullMsg.section('\n', 0, 0).trimmed();
                    QString desc = fullMsg.section('\n', 1).trimmed();

                    QList<QGitCommitDiffParent> parents;
                    unsigned int parentCount = git_commit_parentcount(commit);
                    for (unsigned int p = 0; p < parentCount; ++p) {
                        const git_oid *parentId = git_commit_parent_id(commit, p);
                        if (parentId) {
                            char pBuf[GIT_OID_HEXSZ + 1];
                            git_oid_tostr(pBuf, sizeof(pBuf), parentId);
                            parents.append(QGitCommitDiffParent(QByteArray(pBuf)));
                        }
                    }

                    m_commits.append(QGitCommit(hash, parents, dateTime, author, committer, summary, desc));
                    git_commit_free(commit);
                    count++;
                }
            }
            git_revwalk_free(walker);
        }
        git_repository_free(repo);
    }
}

void QGitSubmodulePointerDialog::populateBranchesTable()
{
    ui->branchesTable->setRowCount(0);
    QString filter = ui->lineEdit_filterBranches->text().trimmed();

    int row = 0;
    for (const auto &branch : m_branches) {
        if (!filter.isEmpty() && !branch.name().contains(filter, Qt::CaseInsensitive)) {
            continue;
        }

        ui->branchesTable->insertRow(row);

        auto nameItem = new QTableWidgetItem(branch.name());
        nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
        nameItem->setData(Qt::UserRole, branch.name());
        nameItem->setData(Qt::UserRole + 1, branch.hash());
        ui->branchesTable->setItem(row, 0, nameItem);

        QString typeStr = (branch.type() == GIT_BRANCH_LOCAL) ? tr("Local") : tr("Remote");
        auto typeItem = new QTableWidgetItem(typeStr);
        typeItem->setFlags(typeItem->flags() & ~Qt::ItemIsEditable);
        ui->branchesTable->setItem(row, 1, typeItem);

        auto hashItem = new QTableWidgetItem(branch.hash().left(8));
        hashItem->setFlags(hashItem->flags() & ~Qt::ItemIsEditable);
        ui->branchesTable->setItem(row, 2, hashItem);

        row++;
    }
}

void QGitSubmodulePointerDialog::populateTagsTable()
{
    ui->tagsTable->setRowCount(0);
    QString filter = ui->lineEdit_filterTags->text().trimmed();

    int row = 0;
    for (const auto &tag : m_tags) {
        if (!filter.isEmpty() && !tag.name().contains(filter, Qt::CaseInsensitive)) {
            continue;
        }

        ui->tagsTable->insertRow(row);

        auto nameItem = new QTableWidgetItem(tag.name());
        nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
        nameItem->setData(Qt::UserRole, tag.name());
        nameItem->setData(Qt::UserRole + 1, tag.hash());
        ui->tagsTable->setItem(row, 0, nameItem);

        auto hashItem = new QTableWidgetItem(tag.hash().left(8));
        hashItem->setFlags(hashItem->flags() & ~Qt::ItemIsEditable);
        ui->tagsTable->setItem(row, 1, hashItem);

        QDateTime dateTime = QDateTime::fromSecsSinceEpoch(tag.time());
        auto dateItem = new QTableWidgetItem(dateTime.toString(QStringLiteral("yyyy-MM-dd hh:mm")));
        dateItem->setFlags(dateItem->flags() & ~Qt::ItemIsEditable);
        ui->tagsTable->setItem(row, 2, dateItem);

        row++;
    }
}

void QGitSubmodulePointerDialog::populateCommitsTable()
{
    ui->commitsTable->setRowCount(0);
    QString filter = ui->lineEdit_filterCommits->text().trimmed();

    int row = 0;
    for (const auto &commit : m_commits) {
        if (!filter.isEmpty()) {
            bool matches = commit.message().contains(filter, Qt::CaseInsensitive)
                        || commit.id().contains(filter, Qt::CaseInsensitive)
                        || commit.author().name().contains(filter, Qt::CaseInsensitive);
            if (!matches) continue;
        }

        ui->commitsTable->insertRow(row);

        auto hashItem = new QTableWidgetItem(commit.id().left(8));
        hashItem->setFlags(hashItem->flags() & ~Qt::ItemIsEditable);
        hashItem->setData(Qt::UserRole, commit.id());
        ui->commitsTable->setItem(row, 0, hashItem);

        auto msgItem = new QTableWidgetItem(commit.message());
        msgItem->setFlags(msgItem->flags() & ~Qt::ItemIsEditable);
        ui->commitsTable->setItem(row, 1, msgItem);

        auto authorItem = new QTableWidgetItem(commit.author().name());
        authorItem->setFlags(authorItem->flags() & ~Qt::ItemIsEditable);
        ui->commitsTable->setItem(row, 2, authorItem);

        auto dateItem = new QTableWidgetItem(commit.time().toString(QStringLiteral("yyyy-MM-dd hh:mm")));
        dateItem->setFlags(dateItem->flags() & ~Qt::ItemIsEditable);
        ui->commitsTable->setItem(row, 3, dateItem);

        row++;
    }
}

void QGitSubmodulePointerDialog::on_tabWidget_currentChanged(int index)
{
    Q_UNUSED(index)
    updateButtons();
}

void QGitSubmodulePointerDialog::on_branchesTable_itemSelectionChanged()
{
    int row = ui->branchesTable->currentRow();
    if (row >= 0) {
        auto item = ui->branchesTable->item(row, 0);
        if (item) {
            QString branchName = item->data(Qt::UserRole).toString();
            // Automatically propose tracked branch name if it's a branch
            QString cleanName = branchName;
            if (cleanName.startsWith(QStringLiteral("refs/heads/"))) {
                cleanName = cleanName.mid(11);
            } else if (cleanName.startsWith(QStringLiteral("refs/remotes/origin/"))) {
                cleanName = cleanName.mid(20);
            } else if (cleanName.startsWith(QStringLiteral("refs/remotes/"))) {
                cleanName = cleanName.section('/', 2);
            }
            if (ui->checkBox_updateTrackedBranch->isChecked() && ui->lineEdit_trackedBranch->text().isEmpty()) {
                ui->lineEdit_trackedBranch->setText(cleanName);
            }
        }
    }
    updateButtons();
}

void QGitSubmodulePointerDialog::on_tagsTable_itemSelectionChanged()
{
    updateButtons();
}

void QGitSubmodulePointerDialog::on_commitsTable_itemSelectionChanged()
{
    updateButtons();
}

void QGitSubmodulePointerDialog::on_lineEdit_customCommit_textChanged(const QString &text)
{
    Q_UNUSED(text)
    updateButtons();
}

void QGitSubmodulePointerDialog::on_lineEdit_filterBranches_textChanged(const QString &text)
{
    Q_UNUSED(text)
    populateBranchesTable();
    updateButtons();
}

void QGitSubmodulePointerDialog::on_lineEdit_filterTags_textChanged(const QString &text)
{
    Q_UNUSED(text)
    populateTagsTable();
    updateButtons();
}

void QGitSubmodulePointerDialog::on_lineEdit_filterCommits_textChanged(const QString &text)
{
    Q_UNUSED(text)
    populateCommitsTable();
    updateButtons();
}

void QGitSubmodulePointerDialog::on_checkBox_updateTrackedBranch_toggled(bool checked)
{
    ui->lineEdit_trackedBranch->setEnabled(checked);
    updateButtons();
}

void QGitSubmodulePointerDialog::updateButtons()
{
    bool hasTarget = !selectedTarget().isEmpty();
    bool hasTrackedBranchChange = shouldUpdateTrackedBranch();

    QPushButton *okBtn = ui->buttonBox->button(QDialogButtonBox::Ok);
    if (okBtn) {
        okBtn->setEnabled(hasTarget || hasTrackedBranchChange);
    }
}

QString QGitSubmodulePointerDialog::selectedTarget() const
{
    int currentTab = ui->tabWidget->currentIndex();
    if (currentTab == 0) { // Branches
        int row = ui->branchesTable->currentRow();
        if (row >= 0) {
            auto item = ui->branchesTable->item(row, 0);
            if (item) return item->data(Qt::UserRole).toString();
        }
    } else if (currentTab == 1) { // Tags
        int row = ui->tagsTable->currentRow();
        if (row >= 0) {
            auto item = ui->tagsTable->item(row, 0);
            if (item) return item->data(Qt::UserRole).toString();
        }
    } else if (currentTab == 2) { // Commits
        int row = ui->commitsTable->currentRow();
        if (row >= 0) {
            auto item = ui->commitsTable->item(row, 0);
            if (item) return item->data(Qt::UserRole).toString();
        }
    } else if (currentTab == 3) { // Custom
        return ui->lineEdit_customCommit->text().trimmed();
    }
    return QString();
}

QString QGitSubmodulePointerDialog::trackedBranch() const
{
    if (ui->checkBox_updateTrackedBranch->isChecked()) {
        return ui->lineEdit_trackedBranch->text().trimmed();
    }
    return QString();
}

bool QGitSubmodulePointerDialog::shouldUpdateTrackedBranch() const
{
    return ui->checkBox_updateTrackedBranch->isChecked();
}
