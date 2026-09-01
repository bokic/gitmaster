#include "qgitexportpatchdialog.h"
#include "ui_qgitexportpatchdialog.h"
#include "qgit.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QHeaderView>
#include <QDir>
#include <QPushButton>

QGitExportPatchDialog::QGitExportPatchDialog(QGit *git, const QString &selectedCommitHash, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::QGitExportPatchDialog)
    , m_git(git)
    , m_selectedHash(selectedCommitHash)
{
    ui->setupUi(this);

    // Setup table headers
    ui->tableWidget_commits->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->tableWidget_commits->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->tableWidget_commits->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    ui->tableWidget_commits->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    ui->tableWidget_commits->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);

    // Initial output directory
    if (m_git) {
        ui->lineEdit_outputDir->setText(m_git->path().absolutePath());
    } else {
        ui->lineEdit_outputDir->setText(QDir::homePath());
    }

    if (m_selectedHash.isEmpty()) {
        ui->radioButton_selected->setEnabled(false);
        ui->radioButton_rangeToHead->setEnabled(false);
        ui->radioButton_lastN->setChecked(true);
    } else {
        ui->radioButton_selected->setText(tr("Selected commit (%1)").arg(m_selectedHash.left(7)));
        ui->radioButton_selected->setChecked(true);
    }

    updateCommitList();
}

QGitExportPatchDialog::~QGitExportPatchDialog()
{
    delete ui;
}

QString QGitExportPatchDialog::outputDirectory() const
{
    return ui->lineEdit_outputDir->text().trimmed();
}

QStringList QGitExportPatchDialog::selectedCommitIds() const
{
    QStringList list;
    for (const auto &commit : m_currentCommits) {
        list.append(commit.id());
    }
    return list;
}

QString QGitExportPatchDialog::subjectPrefix() const
{
    return ui->lineEdit_prefix->text().trimmed();
}

bool QGitExportPatchDialog::isNumbered() const
{
    return ui->checkBox_numbered->isChecked();
}

bool QGitExportPatchDialog::detectRenames() const
{
    return ui->checkBox_detectRenames->isChecked();
}

void QGitExportPatchDialog::on_radioButton_selected_toggled(bool checked)
{
    if (checked) updateCommitList();
}

void QGitExportPatchDialog::on_radioButton_rangeToHead_toggled(bool checked)
{
    if (checked) updateCommitList();
}

void QGitExportPatchDialog::on_radioButton_lastN_toggled(bool checked)
{
    if (checked) updateCommitList();
}

void QGitExportPatchDialog::on_radioButton_custom_toggled(bool checked)
{
    if (checked) updateCommitList();
}

void QGitExportPatchDialog::on_spinBox_lastN_valueChanged(int)
{
    if (ui->radioButton_lastN->isChecked()) updateCommitList();
}

void QGitExportPatchDialog::on_lineEdit_custom_textChanged(const QString &)
{
    if (ui->radioButton_custom->isChecked()) updateCommitList();
}

void QGitExportPatchDialog::on_pushButton_browse_clicked()
{
    QString initialDir = ui->lineEdit_outputDir->text().trimmed();
    if (initialDir.isEmpty() && m_git) {
        initialDir = m_git->path().absolutePath();
    }
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Output Directory"), initialDir);
    if (!dir.isEmpty()) {
        ui->lineEdit_outputDir->setText(dir);
    }
}

void QGitExportPatchDialog::updateCommitList()
{
    if (!m_git) return;

    QList<QGitCommit> commits;

    if (ui->radioButton_selected->isChecked()) {
        if (!m_selectedHash.isEmpty()) {
            commits = m_git->getCommitList(QStringList() << m_selectedHash);
        }
    } else if (ui->radioButton_rangeToHead->isChecked()) {
        if (!m_selectedHash.isEmpty()) {
            commits = m_git->getCommitsFromCommitToHead(m_selectedHash);
        }
    } else if (ui->radioButton_lastN->isChecked()) {
        int n = ui->spinBox_lastN->value();
        commits = m_git->getLastNCommits(n);
    } else if (ui->radioButton_custom->isChecked()) {
        QString spec = ui->lineEdit_custom->text().trimmed();
        if (!spec.isEmpty()) {
            commits = m_git->getCommitsFromRevSpec(spec);
        }
    }

    populateTable(commits);
}

void QGitExportPatchDialog::populateTable(const QList<QGitCommit> &commits)
{
    m_currentCommits = commits;
    ui->tableWidget_commits->setRowCount(0);

    for (int i = 0; i < commits.size(); ++i) {
        const auto &c = commits.at(i);
        int row = ui->tableWidget_commits->rowCount();
        ui->tableWidget_commits->insertRow(row);

        auto indexItem = new QTableWidgetItem(QString::number(i + 1));
        indexItem->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget_commits->setItem(row, 0, indexItem);

        auto shaItem = new QTableWidgetItem(c.id().left(7));
        shaItem->setToolTip(c.id());
        ui->tableWidget_commits->setItem(row, 1, shaItem);

        QString subject = c.message().section('\n', 0, 0);
        auto subjItem = new QTableWidgetItem(subject);
        subjItem->setToolTip(c.message());
        ui->tableWidget_commits->setItem(row, 2, subjItem);

        auto authorItem = new QTableWidgetItem(c.author().name());
        authorItem->setToolTip(QString("%1 <%2>").arg(c.author().name(), c.author().email()));
        ui->tableWidget_commits->setItem(row, 3, authorItem);

        auto dateItem = new QTableWidgetItem(c.time().toString("yyyy-MM-dd hh:mm"));
        ui->tableWidget_commits->setItem(row, 4, dateItem);
    }

    if (commits.isEmpty()) {
        ui->label_commitSummary->setText(tr("No commits selected or found for export."));
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    } else {
        ui->label_commitSummary->setText(tr("Total: %n commit(s) ready to be exported as .patch file(s).", "", commits.size()));
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    }
}

void QGitExportPatchDialog::accept()
{
    QString outDir = outputDirectory();
    if (outDir.isEmpty()) {
        QMessageBox::warning(this, tr("Invalid Destination"), tr("Please specify a valid output directory."));
        return;
    }

    QDir dir(outDir);
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            QMessageBox::warning(this, tr("Directory Error"), tr("Could not create directory:\n%1").arg(outDir));
            return;
        }
    }

    if (m_currentCommits.isEmpty()) {
        QMessageBox::warning(this, tr("No Commits"), tr("No commits selected for export."));
        return;
    }

    QDialog::accept();
}
