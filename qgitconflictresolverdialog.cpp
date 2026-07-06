#include "qgitconflictresolverdialog.h"
#include "ui_qgitconflictresolverdialog.h"
#include <QScrollBar>
#include <QTextBlock>
#include <QTextCursor>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>

QGitConflictResolverDialog::QGitConflictResolverDialog(QGit *git, const QString &filePath, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::QGitConflictResolverDialog)
    , m_git(git)
    , m_filePath(filePath)
{
    ui->setupUi(this);

    // Distribute splitter widths equally
    ui->splitter->setSizes(QList<int>() << 300 << 300 << 300);

    // Sync scrollbars
    connect(ui->plainTextEdit_ours->verticalScrollBar(), &QScrollBar::valueChanged, this, &QGitConflictResolverDialog::syncScrollLeft);
    connect(ui->plainTextEdit_merged->verticalScrollBar(), &QScrollBar::valueChanged, this, &QGitConflictResolverDialog::syncScrollMiddle);
    connect(ui->plainTextEdit_theirs->verticalScrollBar(), &QScrollBar::valueChanged, this, &QGitConflictResolverDialog::syncScrollRight);

    // Set header labels to show filename
    ui->label_ours->setText(tr("Ours - %1").arg(m_filePath));
    ui->label_merged->setText(tr("Merged Result - %1").arg(m_filePath));
    ui->label_theirs->setText(tr("Theirs - %1").arg(m_filePath));

    loadConflictFiles();
    refreshConflictList();
}

QGitConflictResolverDialog::~QGitConflictResolverDialog()
{
    delete ui;
}

void QGitConflictResolverDialog::syncScrollLeft(int value)
{
    if (ui->plainTextEdit_merged->verticalScrollBar()->value() != value) {
        ui->plainTextEdit_merged->verticalScrollBar()->setValue(value);
    }
    if (ui->plainTextEdit_theirs->verticalScrollBar()->value() != value) {
        ui->plainTextEdit_theirs->verticalScrollBar()->setValue(value);
    }
}

void QGitConflictResolverDialog::syncScrollMiddle(int value)
{
    if (ui->plainTextEdit_ours->verticalScrollBar()->value() != value) {
        ui->plainTextEdit_ours->verticalScrollBar()->setValue(value);
    }
    if (ui->plainTextEdit_theirs->verticalScrollBar()->value() != value) {
        ui->plainTextEdit_theirs->verticalScrollBar()->setValue(value);
    }
}

void QGitConflictResolverDialog::syncScrollRight(int value)
{
    if (ui->plainTextEdit_ours->verticalScrollBar()->value() != value) {
        ui->plainTextEdit_ours->verticalScrollBar()->setValue(value);
    }
    if (ui->plainTextEdit_merged->verticalScrollBar()->value() != value) {
        ui->plainTextEdit_merged->verticalScrollBar()->setValue(value);
    }
}

void QGitConflictResolverDialog::loadConflictFiles()
{
    try {
        QString ancestorText, oursText, theirsText;
        if (m_git->conflictContents(m_filePath, ancestorText, oursText, theirsText)) {
            ui->plainTextEdit_ours->setPlainText(oursText);
            ui->plainTextEdit_theirs->setPlainText(theirsText);
        }

        // Load the conflicted working directory file
        QString fullPath = m_git->path().absoluteFilePath(m_filePath);
        QFile file(fullPath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            ui->plainTextEdit_merged->setPlainText(QString::fromUtf8(file.readAll()));
            file.close();
        }
    } catch (const QGitError &error) {
        QMessageBox::critical(this, tr("Load Error"), error.errorString());
    }
}

void QGitConflictResolverDialog::refreshConflictList()
{
    int currentSelected = ui->listConflicts->currentRow();

    ui->listConflicts->clear();
    QString text = ui->plainTextEdit_merged->toPlainText();

    int pos = 0;
    int count = 0;
    while ((pos = text.indexOf("<<<<<<<", pos)) != -1) {
        count++;
        // Calculate line number
        int line = 1;
        for (int i = 0; i < pos; ++i) {
            if (text[i] == '\n') line++;
        }
        ui->listConflicts->addItem(tr("Conflict Hunk %1 (Line %2)").arg(count).arg(line));
        pos += 7;
    }

    if (count > 0) {
        int nextRow = qMin(currentSelected >= 0 ? currentSelected : 0, count - 1);
        ui->listConflicts->setCurrentRow(nextRow);
        ui->pushButton_useOurs->setEnabled(true);
        ui->pushButton_useTheirs->setEnabled(true);
    } else {
        ui->pushButton_useOurs->setEnabled(false);
        ui->pushButton_useTheirs->setEnabled(false);
    }
}

void QGitConflictResolverDialog::scrollToConflict(int index)
{
    QString text = ui->plainTextEdit_merged->toPlainText();
    int pos = 0;
    for (int i = 0; i <= index; ++i) {
        pos = text.indexOf("<<<<<<<", pos);
        if (pos == -1) return;
        if (i < index) pos += 7;
    }

    int line = 1;
    for (int i = 0; i < pos; ++i) {
        if (text[i] == '\n') line++;
    }

    QTextBlock block = ui->plainTextEdit_merged->document()->findBlockByLineNumber(line - 1);
    QTextCursor cursor(block);
    ui->plainTextEdit_merged->setTextCursor(cursor);
    ui->plainTextEdit_merged->ensureCursorVisible();
}

void QGitConflictResolverDialog::resolveConflictBlock(int index, bool useOurs)
{
    QString text = ui->plainTextEdit_merged->toPlainText();
    int pos = 0;
    for (int i = 0; i <= index; ++i) {
        pos = text.indexOf("<<<<<<<", pos);
        if (pos == -1) return;
        if (i < index) pos += 7;
    }

    int endOurs = text.indexOf("=======", pos);
    if (endOurs == -1) return;

    int endTheirs = text.indexOf(">>>>>>>", endOurs);
    if (endTheirs == -1) return;

    int endLine = text.indexOf("\n", endTheirs);
    if (endLine == -1) endLine = text.length();
    else endLine += 1; // Include the newline

    QString replacement;
    if (useOurs) {
        int startOurs = text.indexOf("\n", pos);
        if (startOurs != -1 && startOurs < endOurs) {
            replacement = text.mid(startOurs + 1, endOurs - startOurs - 1);
        }
    } else {
        int startTheirs = text.indexOf("\n", endOurs);
        if (startTheirs != -1 && startTheirs < endTheirs) {
            replacement = text.mid(startTheirs + 1, endTheirs - startTheirs - 1);
        }
    }

    QTextCursor cursor(ui->plainTextEdit_merged->document());
    cursor.setPosition(pos);
    cursor.setPosition(endLine, QTextCursor::KeepAnchor);
    cursor.insertText(replacement);

    refreshConflictList();
}

void QGitConflictResolverDialog::on_listConflicts_currentRowChanged(int currentRow)
{
    if (currentRow >= 0) {
        scrollToConflict(currentRow);
    }
}

void QGitConflictResolverDialog::on_pushButton_useOurs_clicked()
{
    int row = ui->listConflicts->currentRow();
    if (row >= 0) {
        resolveConflictBlock(row, true);
    }
}

void QGitConflictResolverDialog::on_pushButton_useTheirs_clicked()
{
    int row = ui->listConflicts->currentRow();
    if (row >= 0) {
        resolveConflictBlock(row, false);
    }
}

void QGitConflictResolverDialog::on_pushButton_resolve_clicked()
{
    QString content = ui->plainTextEdit_merged->toPlainText();
    if (content.contains("<<<<<<<") || content.contains("=======") || content.contains(">>>>>>>")) {
        auto response = QMessageBox::warning(this, tr("Unresolved Conflicts"),
                                             tr("The file still contains conflict markers. Are you sure you want to mark it resolved?"),
                                             QMessageBox::Yes | QMessageBox::No);
        if (response != QMessageBox::Yes) return;
    }

    try {
        m_git->resolveConflict(m_filePath, content);
        accept();
    } catch (const QGitError &error) {
        QMessageBox::critical(this, tr("Resolve Error"), error.errorString());
    }
}

void QGitConflictResolverDialog::on_pushButton_cancel_clicked()
{
    reject();
}
