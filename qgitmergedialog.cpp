#include "qgitmergedialog.h"
#include "ui_qgitmergedialog.h"
#include <QDateTime>

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

    // Populate data
    populateBranches();
    populateLogTable();
    updateButtonsState();

    // Setup connections
    connect(ui->pushButton, &QPushButton::clicked, this, &QGitMergeDialog::accept);
    connect(ui->pushButton_2, &QPushButton::clicked, this, &QGitMergeDialog::reject);
    connect(ui->tableWidget, &QTableWidget::itemSelectionChanged, this, &QGitMergeDialog::updateButtonsState);
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
            return ui->tableWidget->item(currentRow, 0)->text();
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
    ui->comboBox_3->clear();

    QString current = m_git.currentBranch();
    ui->label_5->setText(tr("Merging into current branch: <b>%1</b>").arg(current));

    for (const auto &branch : m_allBranches) {
        if (branch.name() != current) {
            ui->comboBox_3->addItem(branch.name(), branch.name());
        }
    }
}

void QGitMergeDialog::populateLogTable()
{
    ui->tableWidget->setRowCount(0);
    QString current = m_git.currentBranch();
    bool showRemote = ui->checkBox->isChecked();
    QString filterText = ui->lineEdit->text().trimmed();
    int filterTypeIndex = ui->comboBox_2->currentIndex(); // 0: All Branches, 1: Current Branch (which we hide anyway or show differently)

    int row = 0;
    for (const auto &branch : m_allBranches) {
        if (branch.name() == current) continue;

        if (branch.type() == GIT_BRANCH_REMOTE && !showRemote) continue;

        // "Current Branch" filter makes no sense for picking a merge source, so we just show other branches.
        if (filterTypeIndex == 1 && branch.type() != GIT_BRANCH_LOCAL) continue;

        if (!filterText.isEmpty() && !branch.name().contains(filterText, Qt::CaseInsensitive)) continue;

        ui->tableWidget->insertRow(row);

        auto nameItem = new QTableWidgetItem(branch.name());
        nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
        ui->tableWidget->setItem(row, 0, nameItem);

        auto typeStr = (branch.type() == GIT_BRANCH_LOCAL) ? tr("Local") : tr("Remote");
        auto typeItem = new QTableWidgetItem(typeStr);
        typeItem->setFlags(typeItem->flags() & ~Qt::ItemIsEditable);
        ui->tableWidget->setItem(row, 1, typeItem);

        auto hashItem = new QTableWidgetItem(branch.hash().left(8));
        hashItem->setFlags(hashItem->flags() & ~Qt::ItemIsEditable);
        hashItem->setData(Qt::UserRole, branch.hash());
        ui->tableWidget->setItem(row, 2, hashItem);

        QDateTime dateTime = QDateTime::fromSecsSinceEpoch(branch.time());
        auto timeItem = new QTableWidgetItem(dateTime.toString());
        timeItem->setFlags(timeItem->flags() & ~Qt::ItemIsEditable);
        ui->tableWidget->setItem(row, 3, timeItem);

        row++;
    }
    updateButtonsState();
}
