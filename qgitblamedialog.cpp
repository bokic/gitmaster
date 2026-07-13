#include "qgitblamedialog.h"
#include "ui_qgitblamedialog.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHeaderView>
#include <QColor>
#include <QFont>
#include <QApplication>
#include <QPalette>

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static const QStringList kIgnoredDirs  = { ".git", "build", ".cache", "__pycache__" };
static const QStringList kIgnoredExts  = {
    ".o", ".a", ".so", ".dll", ".exe", ".obj",
    ".png", ".jpg", ".jpeg", ".gif", ".svg", ".ico",
    ".ttf", ".woff", ".woff2",
    ".zip", ".tar", ".gz", ".bz2", ".xz",
    ".pdf", ".pyc"
};


// ---------------------------------------------------------------------------
// Colour palette for blame hunks (one colour per unique commit, cycling)
// ---------------------------------------------------------------------------

QColor QGitBlameDialog::hunkColor(const QString &hash, int index)
{
    Q_UNUSED(hash)
    // Use two alternating neutral tints so adjacent hunks from different
    // commits are visually separated without being distracting.
    static const QColor kPalette[] = {
        QColor(230, 240, 255),   // soft blue
        QColor(225, 255, 230),   // soft green
        QColor(255, 245, 220),   // soft amber
        QColor(245, 225, 255),   // soft violet
        QColor(220, 248, 248),   // soft cyan
        QColor(255, 230, 230),   // soft rose
        QColor(240, 255, 220),   // soft lime
        QColor(255, 235, 245),   // soft pink
    };
    return kPalette[index % 8];
}

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

QGitBlameDialog::QGitBlameDialog(QGit *git, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::QGitBlameDialog)
    , m_git(git)
{
    ui->setupUi(this);
    setWindowTitle(tr("Git Blame Browser"));
    resize(1100, 680);

    // Splitter initial proportions  (left: 28%, right: 72%)
    ui->splitter->setSizes({ 300, 800 });

    // Blame table column sizing
    ui->blameTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents); // Line
    ui->blameTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents); // Commit
    ui->blameTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents); // Author
    ui->blameTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents); // Date
    ui->blameTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);          // Content
    ui->blameTable->verticalHeader()->setVisible(false);
    ui->blameTable->verticalHeader()->setDefaultSectionSize(20);

    // Monospace font for the content column
    QFont monoFont;
    monoFont.setFamily(QStringLiteral("Monospace"));
    monoFont.setFixedPitch(true);
    monoFont.setPointSize(9);
    ui->blameTable->setFont(monoFont);

    // File tree
    ui->fileTree->setIconSize(QSize(16, 16));

    populateFileTree(git->path().absolutePath());
}

QGitBlameDialog::~QGitBlameDialog()
{
    delete ui;
}

// ---------------------------------------------------------------------------
// File tree population
// ---------------------------------------------------------------------------

void QGitBlameDialog::populateFileTree(const QString &rootPath)
{
    ui->fileTree->clear();

    auto *root = new QTreeWidgetItem(ui->fileTree);
    root->setText(0, QFileInfo(rootPath).fileName());
    root->setData(0, Qt::UserRole, QString()); // not a selectable file
    root->setData(0, Qt::UserRole + 1, rootPath);

    QIcon folderIcon = QApplication::style()->standardIcon(QStyle::SP_DirIcon);
    QIcon fileIcon   = QApplication::style()->standardIcon(QStyle::SP_FileIcon);
    root->setIcon(0, folderIcon);

    addDirectoryToTree(root, rootPath, rootPath);
    root->setExpanded(true);

    // Store icons for re-use during filtering
    ui->fileTree->setProperty("folderIcon", QVariant::fromValue(folderIcon));
    ui->fileTree->setProperty("fileIcon",   QVariant::fromValue(fileIcon));
}

void QGitBlameDialog::addDirectoryToTree(QTreeWidgetItem *parent,
                                          const QString &dirPath,
                                          const QString &repoRoot)
{
    QDir dir(dirPath);
    QIcon folderIcon = QApplication::style()->standardIcon(QStyle::SP_DirIcon);
    QIcon fileIcon   = QApplication::style()->standardIcon(QStyle::SP_FileIcon);

    // Directories first
    const QFileInfoList dirs = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    for (const QFileInfo &info : dirs) {
        if (kIgnoredDirs.contains(info.fileName())) continue;

        auto *item = new QTreeWidgetItem(parent);
        item->setText(0, info.fileName());
        item->setIcon(0, folderIcon);
        item->setData(0, Qt::UserRole, QString());       // directory — no blame path
        item->setData(0, Qt::UserRole + 1, info.absoluteFilePath());

        addDirectoryToTree(item, info.absoluteFilePath(), repoRoot);
    }

    // Files
    const QFileInfoList files = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
    for (const QFileInfo &info : files) {
        if (kIgnoredExts.contains(info.suffix().prepend('.'))) continue;

        QString relPath = QDir(repoRoot).relativeFilePath(info.absoluteFilePath());

        auto *item = new QTreeWidgetItem(parent);
        item->setText(0, info.fileName());
        item->setIcon(0, fileIcon);
        item->setData(0, Qt::UserRole,     relPath);                  // relative path for blame
        item->setData(0, Qt::UserRole + 1, info.absoluteFilePath()); // absolute for reading
    }
}

// ---------------------------------------------------------------------------
// Filter
// ---------------------------------------------------------------------------

void QGitBlameDialog::filterTree(QTreeWidgetItem *item, const QString &text)
{
    bool anyChildVisible = false;
    for (int i = 0; i < item->childCount(); i++) {
        QTreeWidgetItem *child = item->child(i);
        filterTree(child, text);
        if (!child->isHidden()) anyChildVisible = true;
    }

    if (item->childCount() > 0) {
        // Directory: show if any child is visible
        item->setHidden(!anyChildVisible && !text.isEmpty());
    } else {
        // File: show if name matches (or filter is empty)
        item->setHidden(!text.isEmpty() && !item->text(0).contains(text, Qt::CaseInsensitive));
    }
}

void QGitBlameDialog::on_lineEdit_filter_textChanged(const QString &text)
{
    QTreeWidgetItem *root = ui->fileTree->topLevelItem(0);
    if (!root) return;
    filterTree(root, text);
    if (!text.isEmpty()) {
        ui->fileTree->expandAll();
    } else {
        // Collapse back but keep root expanded
        ui->fileTree->collapseAll();
        root->setExpanded(true);
    }
}

// ---------------------------------------------------------------------------
// File selection → load blame
// ---------------------------------------------------------------------------

void QGitBlameDialog::on_fileTree_itemClicked(QTreeWidgetItem *item, int /*column*/)
{
    QString relPath = item->data(0, Qt::UserRole).toString();
    if (relPath.isEmpty()) return; // directory node

    showBlameForFile(relPath);
}

void QGitBlameDialog::showBlameForFile(const QString &relPath)
{
    ui->blameTable->clearContents();
    ui->blameTable->setRowCount(0);
    ui->label_filePath->setText(relPath);
    ui->label_status->setText(tr("Loading blame for %1...").arg(relPath));
    QApplication::processEvents();

    // Read the actual file lines
    QString absPath = m_git->path().absoluteFilePath(relPath);
    QFile f(absPath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        ui->label_status->setText(tr("Cannot open file: %1").arg(relPath));
        return;
    }
    QStringList lines;
    QTextStream ts(&f);
    while (!ts.atEnd()) lines << ts.readLine();
    f.close();

    if (lines.isEmpty()) {
        ui->label_status->setText(tr("File is empty."));
        return;
    }

    // Compute blame hunks (pinned to m_pinnedCommitId if set, otherwise HEAD)
    QList<QGitBlameHunk> hunks = m_git->blameFile(relPath, m_pinnedCommitId);
    if (hunks.isEmpty()) {
        ui->label_status->setText(tr("No blame data (file may be untracked)."));
        return;
    }

    // Build a per-line hunk lookup (lineNumber → hunk index)
    // hunks are already ordered by final_start_line_number
    QVector<int> lineHunk(lines.size() + 1, -1); // 1-based
    for (int h = 0; h < hunks.size(); h++) {
        const auto &hunk = hunks[h];
        for (size_t l = 0; l < hunk.linesInHunk; l++) {
            size_t lineNo = hunk.finalStartLineNumber + l;
            if (lineNo >= 1 && static_cast<int>(lineNo) <= lines.size()) {
                lineHunk[static_cast<int>(lineNo)] = h;
            }
        }
    }

    // Assign palette index per unique commit (stable insertion-order map)
    QMap<QString, int> commitColorIndex;
    int colorCounter = 0;
    for (const auto &hunk : hunks) {
        if (!commitColorIndex.contains(hunk.commitHash)) {
            commitColorIndex[hunk.commitHash] = colorCounter++;
        }
    }

    QFont monoFont;
    monoFont.setFamily(QStringLiteral("Monospace"));
    monoFont.setFixedPitch(true);
    monoFont.setPointSize(9);

    ui->blameTable->setUpdatesEnabled(false);
    ui->blameTable->setRowCount(lines.size());

    for (int row = 0; row < lines.size(); row++) {
        int lineNo = row + 1;
        int hunkIdx = (lineNo < lineHunk.size()) ? lineHunk[lineNo] : -1;

        QString shortHash, authorName, dateStr, summary, fullHash;
        QColor  bgColor = QColor(245, 245, 245);
        bool isFirstInHunk = false;

        if (hunkIdx >= 0) {
            const auto &hunk = hunks[hunkIdx];
            fullHash   = hunk.commitHash;
            int pIdx   = commitColorIndex.value(fullHash, 0);
            bgColor    = hunkColor(fullHash, pIdx);
            isFirstInHunk = (static_cast<size_t>(lineNo) == hunk.finalStartLineNumber);

            if (isFirstInHunk) {
                shortHash  = hunk.commitHash.left(8);
                authorName = hunk.authorName;
                dateStr    = QDateTime::fromSecsSinceEpoch(hunk.authorTime)
                                 .toString(QStringLiteral("yyyy-MM-dd"));
                summary    = hunk.summary;
            }
        }

        auto makeItem = [&](const QString &text) -> QTableWidgetItem * {
            auto *i = new QTableWidgetItem(text);
            i->setBackground(bgColor);
            i->setData(Qt::UserRole, fullHash); // full hash on every cell for double-click
            return i;
        };

        // Line number (right-aligned)
        auto *lineItem = makeItem(QString::number(lineNo));
        lineItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        lineItem->setForeground(QColor(120, 120, 120));

        // Commit hash (monospace, shown only on first line of hunk)
        auto *hashItem = makeItem(shortHash);
        hashItem->setFont(monoFont);
        if (isFirstInHunk && !shortHash.isEmpty()) {
            hashItem->setForeground(QColor(30, 80, 180));
        }

        // Author
        auto *authorItem = makeItem(authorName);
        authorItem->setForeground(QColor(60, 60, 60));

        // Date
        auto *dateItem = makeItem(dateStr);
        dateItem->setForeground(QColor(100, 100, 100));

        // File content (monospace, full text)
        auto *contentItem = makeItem(lines[row]);
        contentItem->setFont(monoFont);
        contentItem->setForeground(QApplication::palette().color(QPalette::Text));
        contentItem->setBackground(QApplication::palette().color(QPalette::Base)); // neutral bg for content

        ui->blameTable->setItem(row, 0, lineItem);
        ui->blameTable->setItem(row, 1, hashItem);
        ui->blameTable->setItem(row, 2, authorItem);
        ui->blameTable->setItem(row, 3, dateItem);
        ui->blameTable->setItem(row, 4, contentItem);
    }

    ui->blameTable->setUpdatesEnabled(true);
    ui->label_status->setText(tr("%1 lines — %2 blame hunks").arg(lines.size()).arg(hunks.size()));
}

// ---------------------------------------------------------------------------
// Double-click on blame row → navigate to commit
// ---------------------------------------------------------------------------

void QGitBlameDialog::on_blameTable_itemDoubleClicked(QTableWidgetItem *item)
{
    QString hash = item->data(Qt::UserRole).toString();
    if (!hash.isEmpty()) {
        emit commitNavigationRequested(hash);
    }
}

// ---------------------------------------------------------------------------
// Pre-selection: load blame for a specific file (e.g. from logHistory_files)
// ---------------------------------------------------------------------------

void QGitBlameDialog::preSelectFile(const QString &relPath, const QString &commitId)
{
    // Walk the tree to find (and visually select) the matching item
    QTreeWidgetItemIterator it(ui->fileTree);
    while (*it) {
        QString itemRel = (*it)->data(0, Qt::UserRole).toString();
        if (itemRel == relPath) {
            ui->fileTree->setCurrentItem(*it);
            // Make sure all ancestors are expanded
            QTreeWidgetItem *parent = (*it)->parent();
            while (parent) {
                parent->setExpanded(true);
                parent = parent->parent();
            }
            ui->fileTree->scrollToItem(*it);
            break;
        }
        ++it;
    }

    // Load the blame, pinned to commitId if provided
    m_pinnedCommitId = commitId;
    showBlameForFile(relPath);
}
