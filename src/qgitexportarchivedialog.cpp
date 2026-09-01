#include "qgitexportarchivedialog.h"
#include "ui_qgitexportarchivedialog.h"
#include "qgit.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>

QGitExportArchiveDialog::QGitExportArchiveDialog(QGit *git, const QString &selectedRefOrCommit, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::QGitExportArchiveDialog)
    , m_git(git)
    , m_initialRefOrCommit(selectedRefOrCommit)
{
    ui->setupUi(this);

    // Format choices
    ui->comboBox_format->addItem(tr("ZIP Archive (.zip)"), QStringLiteral("zip"));
    ui->comboBox_format->addItem(tr("Gzip Tarball (.tar.gz)"), QStringLiteral("tar.gz"));
    ui->comboBox_format->addItem(tr("Bzip2 Tarball (.tar.bz2)"), QStringLiteral("tar.bz2"));
    ui->comboBox_format->addItem(tr("Uncompressed Tarball (.tar)"), QStringLiteral("tar"));

    populateRefs();

    // Default output path to user's home or parent directory of repo
    updateDefaultFileName();
}

QGitExportArchiveDialog::~QGitExportArchiveDialog()
{
    delete ui;
}

void QGitExportArchiveDialog::populateRefs()
{
    if (!m_git) return;

    m_branches = m_git->branches(GIT_BRANCH_ALL);
    for (const auto &b : m_branches) {
        ui->comboBox_branch->addItem(b.name());
    }

    m_tags = m_git->tags();
    for (const auto &t : m_tags) {
        ui->comboBox_tag->addItem(t.name());
    }

    bool matched = false;

    if (!m_initialRefOrCommit.isEmpty()) {
        QString initial = m_initialRefOrCommit;
        if (initial.startsWith(QStringLiteral("refs/tags/"))) {
            initial = initial.mid(10);
            int idx = ui->comboBox_tag->findText(initial);
            if (idx >= 0) {
                ui->radioButton_tag->setChecked(true);
                ui->comboBox_tag->setCurrentIndex(idx);
                matched = true;
            } else {
                ui->radioButton_tag->setChecked(true);
                ui->comboBox_tag->addItem(initial);
                ui->comboBox_tag->setCurrentText(initial);
                matched = true;
            }
        } else if (initial.startsWith(QStringLiteral("refs/heads/"))) {
            initial = initial.mid(11);
            int idx = ui->comboBox_branch->findText(initial);
            if (idx >= 0) {
                ui->radioButton_branch->setChecked(true);
                ui->comboBox_branch->setCurrentIndex(idx);
                matched = true;
            }
        } else {
            int tagIdx = ui->comboBox_tag->findText(initial);
            int branchIdx = ui->comboBox_branch->findText(initial);
            if (tagIdx >= 0) {
                ui->radioButton_tag->setChecked(true);
                ui->comboBox_tag->setCurrentIndex(tagIdx);
                matched = true;
            } else if (branchIdx >= 0) {
                ui->radioButton_branch->setChecked(true);
                ui->comboBox_branch->setCurrentIndex(branchIdx);
                matched = true;
            } else {
                // Set as commit / custom ref
                ui->radioButton_commit->setChecked(true);
                ui->lineEdit_commit->setText(initial);
                matched = true;
            }
        }
    }

    if (!matched) {
        ui->radioButton_currentHead->setChecked(true);
    }
}

QString QGitExportArchiveDialog::getEffectiveRefName() const
{
    if (ui->radioButton_currentHead->isChecked()) {
        return QStringLiteral("HEAD");
    } else if (ui->radioButton_branch->isChecked()) {
        return ui->comboBox_branch->currentText();
    } else if (ui->radioButton_tag->isChecked()) {
        return ui->comboBox_tag->currentText();
    } else if (ui->radioButton_commit->isChecked()) {
        QString c = ui->lineEdit_commit->text().trimmed();
        return c.isEmpty() ? QStringLiteral("HEAD") : c;
    }
    return QStringLiteral("HEAD");
}

void QGitExportArchiveDialog::updateDefaultFileName()
{
    if (m_userModifiedOutputFile) return;

    QString repoName = QStringLiteral("repository");
    if (m_git) {
        repoName = m_git->path().dirName();
    }

    QString ref = getEffectiveRefName();
    if (ref.contains('/')) {
        ref = ref.section('/', -1);
    }
    // Clean ref string of invalid filename characters
    QString cleanRef;
    for (const QChar &ch : ref) {
        if (ch.isLetterOrNumber() || ch == '_' || ch == '-' || ch == '.') {
            cleanRef.append(ch);
        } else {
            cleanRef.append('-');
        }
    }
    while (cleanRef.endsWith('-')) cleanRef.chop(1);
    if (cleanRef.isEmpty()) cleanRef = QStringLiteral("export");

    QString ext = QStringLiteral(".zip");
    QString fmt = format();
    if (fmt == QStringLiteral("tar.gz")) {
        ext = QStringLiteral(".tar.gz");
    } else if (fmt == QStringLiteral("tar.bz2")) {
        ext = QStringLiteral(".tar.bz2");
    } else if (fmt == QStringLiteral("tar")) {
        ext = QStringLiteral(".tar");
    }

    QString defaultName = QString("%1-%2%3").arg(repoName, cleanRef, ext);
    QString baseDir = QDir::homePath();
    if (m_git) {
        QDir parentDir = m_git->path();
        if (parentDir.cdUp()) {
            baseDir = parentDir.absolutePath();
        }
    }

    ui->lineEdit_outputFile->setText(QDir(baseDir).filePath(defaultName));

    if (ui->lineEdit_prefix->text().isEmpty() || ui->lineEdit_prefix->text().startsWith(repoName)) {
        ui->lineEdit_prefix->setText(QString("%1-%2/").arg(repoName, cleanRef));
    }
}

void QGitExportArchiveDialog::on_lineEdit_outputFile_textEdited(const QString &)
{
    m_userModifiedOutputFile = true;
}

void QGitExportArchiveDialog::on_radioButton_currentHead_toggled(bool checked)
{
    if (checked) updateDefaultFileName();
}

void QGitExportArchiveDialog::on_radioButton_branch_toggled(bool checked)
{
    ui->comboBox_branch->setEnabled(checked);
    if (checked) updateDefaultFileName();
}

void QGitExportArchiveDialog::on_radioButton_tag_toggled(bool checked)
{
    ui->comboBox_tag->setEnabled(checked);
    if (checked) updateDefaultFileName();
}

void QGitExportArchiveDialog::on_radioButton_commit_toggled(bool checked)
{
    ui->lineEdit_commit->setEnabled(checked);
    if (checked) updateDefaultFileName();
}

void QGitExportArchiveDialog::on_comboBox_branch_currentIndexChanged(int)
{
    if (ui->radioButton_branch->isChecked()) {
        updateDefaultFileName();
    }
}

void QGitExportArchiveDialog::on_comboBox_tag_currentIndexChanged(int)
{
    if (ui->radioButton_tag->isChecked()) {
        updateDefaultFileName();
    }
}

void QGitExportArchiveDialog::on_lineEdit_commit_textChanged(const QString &)
{
    if (ui->radioButton_commit->isChecked()) {
        updateDefaultFileName();
    }
}

void QGitExportArchiveDialog::on_comboBox_format_currentIndexChanged(int)
{
    // Adjust file extension
    QString currentPath = ui->lineEdit_outputFile->text().trimmed();
    if (!currentPath.isEmpty()) {
        QString fmt = format();
        QString newExt = QStringLiteral(".zip");
        if (fmt == QStringLiteral("tar.gz")) newExt = QStringLiteral(".tar.gz");
        else if (fmt == QStringLiteral("tar.bz2")) newExt = QStringLiteral(".tar.bz2");
        else if (fmt == QStringLiteral("tar")) newExt = QStringLiteral(".tar");

        if (currentPath.endsWith(QStringLiteral(".tar.gz"), Qt::CaseInsensitive)) {
            currentPath.chop(7);
            currentPath += newExt;
            ui->lineEdit_outputFile->setText(currentPath);
        } else if (currentPath.endsWith(QStringLiteral(".tar.bz2"), Qt::CaseInsensitive)) {
            currentPath.chop(8);
            currentPath += newExt;
            ui->lineEdit_outputFile->setText(currentPath);
        } else if (currentPath.endsWith(QStringLiteral(".zip"), Qt::CaseInsensitive)) {
            currentPath.chop(4);
            currentPath += newExt;
            ui->lineEdit_outputFile->setText(currentPath);
        } else if (currentPath.endsWith(QStringLiteral(".tar"), Qt::CaseInsensitive)) {
            currentPath.chop(4);
            currentPath += newExt;
            ui->lineEdit_outputFile->setText(currentPath);
        }
    }
}

void QGitExportArchiveDialog::on_pushButton_browse_clicked()
{
    QString fmt = format();
    QString filter;
    if (fmt == QStringLiteral("zip")) {
        filter = tr("ZIP Archives (*.zip);;All Files (*)");
    } else if (fmt == QStringLiteral("tar.gz")) {
        filter = tr("Gzip Tarballs (*.tar.gz *.tgz);;All Files (*)");
    } else if (fmt == QStringLiteral("tar.bz2")) {
        filter = tr("Bzip2 Tarballs (*.tar.bz2 *.tbz2);;All Files (*)");
    } else {
        filter = tr("Tarballs (*.tar);;All Files (*)");
    }

    QString selected = QFileDialog::getSaveFileName(this, tr("Save Archive As"), ui->lineEdit_outputFile->text(), filter);
    if (!selected.isEmpty()) {
        m_userModifiedOutputFile = true;
        ui->lineEdit_outputFile->setText(selected);
    }
}

QString QGitExportArchiveDialog::outputFilePath() const
{
    return ui->lineEdit_outputFile->text().trimmed();
}

QString QGitExportArchiveDialog::targetRefOrCommit() const
{
    return getEffectiveRefName();
}

QString QGitExportArchiveDialog::prefix() const
{
    return ui->lineEdit_prefix->text().trimmed();
}

QString QGitExportArchiveDialog::format() const
{
    return ui->comboBox_format->currentData().toString();
}

void QGitExportArchiveDialog::accept()
{
    QString outPath = outputFilePath();
    if (outPath.isEmpty()) {
        QMessageBox::warning(this, tr("Export Archive"), tr("Please specify a destination archive file path."));
        return;
    }

    QString ref = targetRefOrCommit();
    if (ref.isEmpty()) {
        QMessageBox::warning(this, tr("Export Archive"), tr("Please specify a target branch, tag, or commit."));
        return;
    }

    QDialog::accept();
}
