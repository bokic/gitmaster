// SPDX-License-Identifier: LGPL-3.0-or-later
#include "qgitmergedialog.h"
#include "ui_qgitmergedialog.h"
#include <QDateTime>
#include <algorithm>

namespace {

QString shortBranchName(const QString &name)
{
    if (name.startsWith(QStringLiteral("refs/heads/")))
        return name.mid(11);
    if (name.startsWith(QStringLiteral("refs/remotes/")))
        return name.mid(13);
    return name;
}

} // namespace

QGitMergeDialog::QGitMergeDialog(QDir repoPath, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::QGitMergeDialog)
    , m_repoPath(repoPath)
{
    ui->setupUi(this);
    m_git.setPath(m_repoPath);

    // Make the tool buttons checkable and set default checked state
    ui->mergedFromLog_toolButton->setCheckable(true);
    ui->mergeFetched_toolButton->setCheckable(true);
    ui->mergedFromLog_toolButton->setChecked(true);
    ui->mergeFetched_toolButton->setChecked(false);
    ui->stackedWidget->setCurrentIndex(0);

    // Enable/disable similarity based on checkbox
    ui->spinBox->setEnabled(ui->checkBox_6->isChecked());
    ui->label_2->setEnabled(ui->checkBox_6->isChecked());

    // Configure tableWidget settings
    ui->tableWidget->setColumnCount(4);
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << tr("Branch Name") << tr("Type") << tr("Commit ID") << tr("Date/Time"));
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Populate data
    populateBranches();
    populateLogTable();
    updateButtonsState();

    // Setup connections
    connect(ui->pushButton, &QPushButton::clicked, this, &QGitMergeDialog::accept);
    connect(ui->pushButton_2, &QPushButton::clicked, this, &QGitMergeDialog::reject);
    connect(ui->tableWidget, &QTableWidget::itemSelectionChanged, this, &QGitMergeDialog::updateButtonsState);
    connect(ui->tableWidget, &QTableWidget::cellDoubleClicked, this, [this](int row, int) {
        if (row >= 0) {
            ui->tableWidget->selectRow(row);
            accept();
        }
    });
    connect(ui->comboBox_3, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &QGitMergeDialog::updateButtonsState);
}

QGitMergeDialog::~QGitMergeDialog()
{
    delete ui;
}

QString QGitMergeDialog::selectedBranch() const
{
    if (ui->stackedWidget->currentIndex() == 0) {
        int currentRow = ui->tableWidget->currentRow();
        if (currentRow >= 0) {
            const QTableWidgetItem *item = ui->tableWidget->item(currentRow, 0);
            if (item) {
                const QString fullRef = item->data(Qt::UserRole).toString();
                return fullRef.isEmpty() ? item->text() : fullRef;
            }
        }
    } else {
        return ui->comboBox_3->currentData().toString();
    }
    return QString();
}

bool QGitMergeDialog::commitImmediately() const
{
    return ui->checkBox_2->isChecked();
}

bool QGitMergeDialog::includeMessages() const
{
    return ui->checkBox_3->isChecked();
}

bool QGitMergeDialog::createCommitEvenIfFastForward() const
{
    return ui->checkBox_4->isChecked();
}

bool QGitMergeDialog::rebaseInsteadOfMerge() const
{
    return ui->checkBox_5->isChecked();
}

bool QGitMergeDialog::detectRenames() const
{
    return ui->checkBox_6->isChecked();
}

int QGitMergeDialog::renameSimilarity() const
{
    return ui->spinBox->value();
}

void QGitMergeDialog::setRebaseMode(bool rebaseOnly)
{
    if (rebaseOnly) {
        setWindowTitle(tr("Rebase"));
        ui->groupBox->setVisible(false);
        ui->mergedFromLog_toolButton->setText(tr("Rebase from Log"));
        ui->mergeFetched_toolButton->setText(tr("Rebase Fetched"));
        ui->label_3->setText(tr("Rebase from Log"));
        ui->label_4->setText(tr("Rebase from fetched remote branch:"));
        ui->label_5->setText(tr("Rebasing current branch: <b>%1</b>").arg(m_currentBranch.toHtmlEscaped()));
        ui->checkBox_5->setChecked(true);
    } else {
        setWindowTitle(tr("Pick a commit to merge into your current branch"));
        ui->groupBox->setVisible(true);
        ui->mergedFromLog_toolButton->setText(tr("Merge From Log"));
        ui->mergeFetched_toolButton->setText(tr("Merge Fetched"));
        ui->label_3->setText(tr("Merge From Log"));
        ui->label_4->setText(tr("Merge from fetched remote branch:"));
        ui->label_5->setText(tr("Merging into current branch: <b>%1</b>").arg(m_currentBranch.toHtmlEscaped()));
        ui->checkBox_5->setChecked(false);
    }
}

void QGitMergeDialog::on_mergedFromLog_toolButton_clicked()
{
    ui->mergedFromLog_toolButton->setChecked(true);
    ui->mergeFetched_toolButton->setChecked(false);
    ui->stackedWidget->setCurrentIndex(0);
    updateButtonsState();
}

void QGitMergeDialog::on_mergeFetched_toolButton_clicked()
{
    ui->mergeFetched_toolButton->setChecked(true);
    ui->mergedFromLog_toolButton->setChecked(false);
    ui->stackedWidget->setCurrentIndex(1);
    updateButtonsState();
}

void QGitMergeDialog::on_checkBox_6_toggled(bool checked)
{
    ui->spinBox->setEnabled(checked);
    ui->label_2->setEnabled(checked);
}

void QGitMergeDialog::on_comboBox_2_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    populateLogTable();
}

void QGitMergeDialog::on_comboBox_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    populateLogTable();
}

void QGitMergeDialog::on_checkBox_toggled(bool checked)
{
    Q_UNUSED(checked)
    populateLogTable();
}

void QGitMergeDialog::on_lineEdit_textChanged(const QString &text)
{
    Q_UNUSED(text)
    populateLogTable();
}

void QGitMergeDialog::updateButtonsState()
{
    bool hasSelection = false;
    if (ui->stackedWidget->currentIndex() == 0) {
        hasSelection = ui->tableWidget->currentRow() >= 0;
    } else {
        hasSelection = ui->comboBox_3->currentIndex() >= 0;
    }
    ui->pushButton->setEnabled(hasSelection);
}

void QGitMergeDialog::populateBranches()
{
    m_allBranches = m_git.branches(GIT_BRANCH_ALL);
    m_currentBranch = shortBranchName(m_git.currentBranch());
    ui->comboBox_3->clear();

    ui->label_5->setText(tr("Merging into current branch: <b>%1</b>").arg(m_currentBranch.toHtmlEscaped()));

    for (const auto &branch : std::as_const(m_allBranches)) {
        if (branch.type() == GIT_BRANCH_REMOTE) {
            ui->comboBox_3->addItem(shortBranchName(branch.name()), branch.name());
        }
    }
}

void QGitMergeDialog::populateLogTable()
{
    ui->tableWidget->setRowCount(0);
    const bool showRemote = ui->checkBox->isChecked();
    const QString filterText = ui->lineEdit->text().trimmed();
    const int filterTypeIndex = ui->comboBox_2->currentIndex(); // 0: All Branches, 1: Local Branches
    const int sortOrderIndex = ui->comboBox->currentIndex();    // 0: Date Order, 1: Name Order

    QList<QGitBranch> filtered;
    for (const auto &branch : std::as_const(m_allBranches)) {
        if (branch.type() == GIT_BRANCH_LOCAL && shortBranchName(branch.name()) == m_currentBranch)
            continue;

        if (branch.type() == GIT_BRANCH_REMOTE && !showRemote)
            continue;

        if (filterTypeIndex == 1 && branch.type() != GIT_BRANCH_LOCAL)
            continue;

        if (!filterText.isEmpty() && !branch.name().contains(filterText, Qt::CaseInsensitive))
            continue;

        filtered.append(branch);
    }

    if (sortOrderIndex == 0) {
        std::sort(filtered.begin(), filtered.end(), [](const QGitBranch &a, const QGitBranch &b) {
            return a.time() > b.time();
        });
    } else {
        std::sort(filtered.begin(), filtered.end(), [](const QGitBranch &a, const QGitBranch &b) {
            return a.name().compare(b.name(), Qt::CaseInsensitive) < 0;
        });
    }

    ui->tableWidget->setRowCount(static_cast<int>(filtered.size()));
    for (int row = 0; row < filtered.size(); ++row) {
        const auto &branch = filtered.at(row);

        auto nameItem = new QTableWidgetItem(shortBranchName(branch.name()));
        nameItem->setData(Qt::UserRole, branch.name());
        nameItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        ui->tableWidget->setItem(row, 0, nameItem);

        auto typeStr = (branch.type() == GIT_BRANCH_LOCAL) ? tr("Local") : tr("Remote");
        auto typeItem = new QTableWidgetItem(typeStr);
        typeItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        ui->tableWidget->setItem(row, 1, typeItem);

        auto hashItem = new QTableWidgetItem(branch.hash().left(8));
        hashItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        hashItem->setData(Qt::UserRole, branch.hash());
        ui->tableWidget->setItem(row, 2, hashItem);

        QDateTime dateTime = QDateTime::fromSecsSinceEpoch(branch.time());
        auto timeItem = new QTableWidgetItem(dateTime.toString());
        timeItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        ui->tableWidget->setItem(row, 3, timeItem);
    }
    updateButtonsState();
}
