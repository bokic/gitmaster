// SPDX-License-Identifier: LGPL-3.0-or-later
#include "qgitcommitpickerdialog.h"
#include "qloghistorytablewidget.h"
#include "qloghistoryitemdelegate.h"

#include <QDialogButtonBox>
#include <QHeaderView>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollBar>
#include <QVBoxLayout>

namespace {
constexpr int kCommitPageSize = 100;
}

QGitCommitPickerDialog::QGitCommitPickerDialog(const QDir &repoPath, QWidget *parent)
    : QDialog(parent)
    , m_repoPath(repoPath)
    , m_thread(this)
    , m_git(new QGit())
{
    setWindowTitle(tr("Pick Commit"));
    resize(820, 520);

    auto *layout = new QVBoxLayout(this);

    m_filterEdit = new QLineEdit(this);
    m_filterEdit->setPlaceholderText(tr("Filter loaded commits by message, author, or hash..."));
    m_filterEdit->setClearButtonEnabled(true);
    layout->addWidget(m_filterEdit);

    m_commitTable = new QLogHistoryTableWidget(this);
    m_commitTable->setColumnCount(5);
    m_commitTable->setHorizontalHeaderLabels({tr("Graph"), tr("Message"), tr("Date"), tr("Author"), tr("Commit")});
    m_commitTable->setItemDelegate(new QLogHistoryItemDelegate(m_commitTable));
    m_commitTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_commitTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_commitTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_commitTable->verticalHeader()->setVisible(false);
    m_commitTable->verticalHeader()->setDefaultSectionSize(24);
    m_commitTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_commitTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_commitTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_commitTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_commitTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    layout->addWidget(m_commitTable);

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    layout->addWidget(m_buttonBox);

    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_filterEdit, &QLineEdit::textChanged, this, &QGitCommitPickerDialog::onFilterTextChanged);
    connect(m_commitTable, &QTableWidget::currentCellChanged, this, &QGitCommitPickerDialog::onCurrentCellChanged);
    connect(m_commitTable, &QTableWidget::cellDoubleClicked, this, &QGitCommitPickerDialog::onCellDoubleClicked);
    connect(m_commitTable->verticalScrollBar(), &QScrollBar::valueChanged, this, &QGitCommitPickerDialog::onScrollValueChanged);

    m_git->moveToThread(&m_thread);
    connect(this, &QGitCommitPickerDialog::requestCommits, m_git, &QGit::listCommits);
    connect(m_git, &QGit::listCommitsReply, this, &QGitCommitPickerDialog::onCommitsReply);

    m_thread.setObjectName(QStringLiteral("QGit(commitPicker)"));
    m_thread.start();

    if (!m_repoPath.path().isEmpty()) {
        QGit::invokeSync(m_git, [&]() { m_git->setPath(m_repoPath); });
        m_commitTable->setReferences(
            m_git->branches(GIT_BRANCH_ALL),
            m_git->tags(),
            QGit::branchNameFromPath(m_repoPath.absolutePath()));
        fetchMoreCommits();
    }
}

QGitCommitPickerDialog::~QGitCommitPickerDialog()
{
    if (m_git) {
        m_git->disconnect(this);
        this->disconnect(m_git);
    }

    m_thread.requestInterruption();
    m_thread.quit();
    if (!m_thread.wait(5000)) {
        m_thread.terminate();
        if (!m_thread.wait(1000)) {
            m_git = nullptr;
        }
    }

    delete m_git;
    m_git = nullptr;
}

QString QGitCommitPickerDialog::selectedCommitId() const
{
    const int row = m_commitTable->currentRow();
    if (row < 0)
        return QString();

    const QTableWidgetItem *item = m_commitTable->item(row, 4);
    return item ? item->data(Qt::UserRole).toString() : QString();
}

void QGitCommitPickerDialog::fetchMoreCommits()
{
    if (m_loading || m_allLoaded)
        return;

    m_loading = true;
    emit requestCommits(QStringLiteral("all"), m_offset, kCommitPageSize);
}

void QGitCommitPickerDialog::onCommitsReply(const QList<QGitCommit> &commits, const QGitError &error)
{
    Q_UNUSED(error)
    m_loading = false;

    for (const auto &commit : commits) {
        m_commitTable->addCommit(commit);
    }

    m_offset += static_cast<int>(commits.size());
    if (commits.size() < kCommitPageSize)
        m_allLoaded = true;

    if (!m_filterEdit->text().isEmpty())
        onFilterTextChanged(m_filterEdit->text());
}

void QGitCommitPickerDialog::onFilterTextChanged(const QString &text)
{
    const QString query = text.trimmed();
    for (int row = 0; row < m_commitTable->rowCount(); ++row) {
        if (query.isEmpty()) {
            m_commitTable->setRowHidden(row, false);
            continue;
        }

        const QTableWidgetItem *msgItem = m_commitTable->item(row, 1);
        const QTableWidgetItem *authorItem = m_commitTable->item(row, 3);
        const QTableWidgetItem *hashItem = m_commitTable->item(row, 4);

        const bool matches =
            (msgItem && msgItem->text().contains(query, Qt::CaseInsensitive)) ||
            (authorItem && authorItem->text().contains(query, Qt::CaseInsensitive)) ||
            (hashItem && hashItem->data(Qt::UserRole).toString().contains(query, Qt::CaseInsensitive));

        m_commitTable->setRowHidden(row, !matches);
    }
}

void QGitCommitPickerDialog::onCurrentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    Q_UNUSED(currentColumn)
    Q_UNUSED(previousRow)
    Q_UNUSED(previousColumn)

    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(currentRow >= 0);
}

void QGitCommitPickerDialog::onCellDoubleClicked(int row, int column)
{
    Q_UNUSED(column)
    if (row >= 0) {
        m_commitTable->selectRow(row);
        accept();
    }
}

void QGitCommitPickerDialog::onScrollValueChanged(int value)
{
    if (value == m_commitTable->verticalScrollBar()->maximum())
        fetchMoreCommits();
}
