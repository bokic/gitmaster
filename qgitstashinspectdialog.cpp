#include "qgitstashinspectdialog.h"
#include "ui_qgitstashinspectdialog.h"
#include "qloghistoryitemdelegate.h"

#include <QCryptographicHash>
#include <QFileInfo>
#include <QIcon>
#include <QMessageBox>

QGitStashInspectDialog::QGitStashInspectDialog(QGit *git, const QString &stashRef, const QString &stashName, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::QGitStashInspectDialog)
    , m_git(git)
    , m_stashRef(stashRef)
    , m_stashName(stashName)
{
    ui->setupUi(this);
    setWindowTitle(tr("Inspect Stash - %1").arg(stashName.isEmpty() ? stashRef : stashName));

    ui->logHistory_commits->setItemDelegate(new QLogHistoryItemDelegate(ui->logHistory_commits));
    ui->logHistory_commits->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->logHistory_commits->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->logHistory_commits->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->logHistory_commits->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    ui->logHistory_commits->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);

    ui->diffWidget->setReadonly(true);
    ui->diffWidget->setIgnoreWhitespace(ui->comboBox_gitDiffOptions->ignoreWhitespace());

    ui->splitter_vertical->setStretchFactor(0, 1);
    ui->splitter_vertical->setStretchFactor(1, 2);
    ui->splitter_bottom->setStretchFactor(0, 1);
    ui->splitter_bottom->setStretchFactor(1, 2);
    ui->splitter_left->setStretchFactor(0, 1);
    ui->splitter_left->setStretchFactor(1, 2);

    connect(ui->diffWidget, &QGitDiffWidget::requestGitDiff, m_git, &QGit::commitDiffContent);
    connect(m_git, &QGit::commitDiffContentReply, this, &QGitStashInspectDialog::onCommitDiffContentReply);
    connect(m_git, &QGit::commitDiffReply, this, &QGitStashInspectDialog::onCommitDiffReply);
    connect(m_git, &QGit::listCommitsReply, this, &QGitStashInspectDialog::onListCommitsReply);

    loadStashDetails();
}

QGitStashInspectDialog::~QGitStashInspectDialog()
{
    delete ui;
}

void QGitStashInspectDialog::loadStashDetails()
{
    if (!m_git || m_stashRef.isEmpty())
        return;

    bool ignoreWhitespace = ui->comboBox_gitDiffOptions->ignoreWhitespace();
    m_git->commitDiff(m_stashRef, ignoreWhitespace);
}

void QGitStashInspectDialog::onCommitDiffReply(const QString &commit_id, const QGitCommit &diff, const QGitError &error)
{
    Q_UNUSED(commit_id)
    if (error.hasError())
    {
        QMessageBox::warning(this, tr("Stash Error"), tr("Failed to load stash details: %1").arg(error.errorString()));
        return;
    }

    if (m_stashCommit.id().isEmpty())
    {
        // First response is the stash commit itself
        m_stashCommit = diff;
        m_activeCommitDiff = diff;

        m_parentHashes.clear();
        for (int i = 0; i < m_stashCommit.parents().count(); ++i)
        {
            m_parentHashes.insert(m_stashCommit.parents().at(i).commitHash().toLower());
        }

        ui->logHistory_commits->setHighlightedCommits(m_parentHashes);
        ui->logHistory_commits->clearCommits();
        ui->logHistory_commits->addCommit(m_stashCommit);
        m_commitMap.insert(m_stashCommit.id().toLower(), m_stashCommit);

        // Fetch commit history leading from the stash commit so parents and context appear in table
        m_git->listCommits(m_stashCommit.id(), 0, 100);
    }
    else
    {
        m_activeCommitDiff = diff;
        m_commitMap.insert(diff.id().toLower(), diff);
    }

    // Populate metadata HTML
    QString html;
    QStringList parentsHtml;
    for (int i = 0; i < m_activeCommitDiff.parents().count(); i++)
    {
        parentsHtml << m_activeCommitDiff.parents().at(i).commitHash().left(10);
    }

    QString email = m_activeCommitDiff.author().email();
    html += QStringLiteral("<div>");
    if (!email.isEmpty())
    {
        html += QStringLiteral("<img src=\"https://www.gravatar.com/avatar/") + QCryptographicHash::hash(email.trimmed().toUtf8(), QCryptographicHash::Md5).toHex() + QStringLiteral("?s=32\" width=\"32\" height=\"32\" style=\"float: right\" />");
    }
    html += QStringLiteral("</div>");
    if (m_activeCommitDiff.id() == m_stashCommit.id())
    {
        html += QStringLiteral("<b>Stash:</b> ") + (m_stashName.isEmpty() ? m_stashRef : m_stashName.toHtmlEscaped()) + QStringLiteral("<br />");
    }
    html += QStringLiteral("<b>Commit:</b> ") + m_activeCommitDiff.id() + QStringLiteral("<br />");
    if (!parentsHtml.isEmpty())
    {
        html += (parentsHtml.count() > 1 ? QStringLiteral("<b>Parents:</b> ") : QStringLiteral("<b>Parent:</b> ")) + parentsHtml.join(", ") + QStringLiteral("<br />");
    }
    html += QStringLiteral("<b>Date:</b> ") + m_activeCommitDiff.time().toString() + QStringLiteral("<br />");
    html += QStringLiteral("<b>Author:</b> ") + m_activeCommitDiff.author().name().toHtmlEscaped() + QStringLiteral(" &lt;") + email.toHtmlEscaped() + QStringLiteral("&gt;<br />");
    html += QStringLiteral("<br />");
    html += m_activeCommitDiff.message().toHtmlEscaped().replace(QLatin1Char('\n'), QStringLiteral("<br />"));

    ui->textBrowser_info->setHtml(html);

    // Populate file list
    ui->tableWidget_files->clearContents();

    QMap<QString, QGitDiffFile> uniqueFiles;
    for (int i = 0; i < m_activeCommitDiff.parents().count(); i++)
    {
        const auto parentFiles = m_activeCommitDiff.parents().at(i).files();
        for (const auto &f : parentFiles)
        {
            QString path = f.new_file().path();
            if (!uniqueFiles.contains(path))
            {
                uniqueFiles.insert(path, f);
            }
        }
    }

    QList<QGitDiffFile> files = uniqueFiles.values();
    ui->tableWidget_files->setRowCount(files.count());

    for (int c = 0; c < files.count(); ++c)
    {
        QString path = files.at(c).new_file().path();
        QString filename = QFileInfo(path).fileName();
        QString pathname = path.left(path.length() - filename.length());
        if (pathname.endsWith('/') || pathname.endsWith('\\'))
        {
            pathname.resize(pathname.length() - 1);
        }

        QIcon item_icon;
        switch (files.at(c).status())
        {
        case GIT_DELTA_ADDED:
            item_icon = QIcon(":/icons/file_new");
            break;
        case GIT_DELTA_DELETED:
            item_icon = QIcon(":/icons/file_removed");
            break;
        case GIT_DELTA_MODIFIED:
            item_icon = QIcon(":/icons/file_modified");
            break;
        default:
            item_icon = QIcon(":/icons/file_unknown");
            break;
        }

        auto *item0 = new QTableWidgetItem(item_icon, filename);
        item0->setData(Qt::UserRole, path);
        ui->tableWidget_files->setItem(c, 0, item0);

        auto *item1 = new QTableWidgetItem(pathname);
        ui->tableWidget_files->setItem(c, 1, item1);
    }

    if (ui->tableWidget_files->rowCount() > 0)
    {
        ui->tableWidget_files->setCurrentCell(0, 0);
        updateDiffForCurrentFile();
    }
    else
    {
        ui->diffWidget->setGitDiff("", "", {});
    }
}

void QGitStashInspectDialog::onListCommitsReply(const QList<QGitCommit> &commits, const QGitError &error)
{
    if (error.hasError() || commits.isEmpty())
        return;

    for (const auto &commit : commits)
    {
        // Avoid duplicating the stash commit itself if revwalk includes it
        if (commit.id().compare(m_stashCommit.id(), Qt::CaseInsensitive) == 0)
            continue;

        ui->logHistory_commits->addCommit(commit);
        m_commitMap.insert(commit.id().toLower(), commit);
    }

    // Keep stash commit (first row) selected initially
    if (ui->logHistory_commits->rowCount() > 0 && ui->logHistory_commits->currentRow() < 0)
    {
        ui->logHistory_commits->selectRow(0);
    }
}

void QGitStashInspectDialog::on_logHistory_commits_itemSelectionChanged()
{
    int row = ui->logHistory_commits->currentRow();
    if (row < 0)
        return;

    QTableWidgetItem *item = ui->logHistory_commits->item(row, 4);
    if (!item)
        return;

    QString commitHash = item->data(Qt::UserRole).toString();
    if (commitHash.isEmpty())
        return;

    if (commitHash.compare(m_activeCommitDiff.id(), Qt::CaseInsensitive) == 0)
        return;

    bool ignoreWhitespace = ui->comboBox_gitDiffOptions->ignoreWhitespace();
    m_git->commitDiff(commitHash, ignoreWhitespace);
}

void QGitStashInspectDialog::on_tableWidget_files_itemSelectionChanged()
{
    updateDiffForCurrentFile();
}

void QGitStashInspectDialog::on_comboBox_gitDiffOptions_optionsChanged()
{
    bool ignoreWhitespace = ui->comboBox_gitDiffOptions->ignoreWhitespace();
    ui->diffWidget->setIgnoreWhitespace(ignoreWhitespace);
    updateDiffForCurrentFile();
}

void QGitStashInspectDialog::updateDiffForCurrentFile()
{
    if (m_activeCommitDiff.parents().isEmpty())
        return;

    int row = ui->tableWidget_files->currentRow();
    if (row < 0)
        return;

    auto *item = ui->tableWidget_files->item(row, 0);
    if (!item)
        return;

    QString filePath = item->data(Qt::UserRole).toString();
    if (filePath.isEmpty())
        return;

    bool ignoreWhitespace = ui->comboBox_gitDiffOptions->ignoreWhitespace();
    ui->diffWidget->setIgnoreWhitespace(ignoreWhitespace);
    ui->diffWidget->setGitDiff(m_activeCommitDiff.parents().at(0).commitHash(), m_activeCommitDiff.id(), {filePath});
}

void QGitStashInspectDialog::onCommitDiffContentReply(const QString &first, const QString &second, const QList<QGitDiffFile> &files, const QGitError &error)
{
    ui->diffWidget->responseGitDiff(first, second, files, error);
}
