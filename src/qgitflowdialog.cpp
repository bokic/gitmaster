// SPDX-License-Identifier: LGPL-3.0-or-later
#include "qgitflowdialog.h"
#include "ui_qgitflowdialog.h"
#include "qgitrepository.h"
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <git2.h>

namespace {

QString normalizedFlowSuffix(const QString &rawName, const QString &prefix)
{
    QString name = rawName.trimmed();
    if (name.startsWith(prefix))
        name = name.mid(prefix.length());
    return name;
}

bool isValidBranchRef(const QString &fullBranchName)
{
    int valid = 0;
    const QByteArray fullRef = (QStringLiteral("refs/heads/") + fullBranchName).toUtf8();
    return (git_reference_name_is_valid(&valid, fullRef.constData()) == 0) && (valid != 0);
}

const QString &errorInputStyle()
{
    static const QString style = QStringLiteral("background-color: rgba(220, 53, 69, 0.35);");
    return style;
}

} // namespace

QGitFlowDialog::QGitFlowDialog(QGitRepository *parent)
    : QDialog(parent)
    , ui(new Ui::QGitFlowDialog)
    , m_repo(parent)
{
    ui->setupUi(this);
    if (parent && parent->git())
        m_git.setPath(parent->git()->path());

    refreshBranches();

    // Enable/Disable push buttons based on input
    on_lineEdit_featureName_textChanged(ui->lineEdit_featureName->text());
    on_lineEdit_releaseName_textChanged(ui->lineEdit_releaseName->text());
    on_lineEdit_hotfixName_textChanged(ui->lineEdit_hotfixName->text());
}

QGitFlowDialog::~QGitFlowDialog()
{
    delete ui;
}

void QGitFlowDialog::refreshBranches()
{
    ui->comboBox_featureBranch->clear();
    ui->comboBox_releaseBranch->clear();
    ui->comboBox_hotfixBranch->clear();

    auto localBranches = m_git.branches(GIT_BRANCH_LOCAL);
    for (const auto &branch : localBranches) {
        QString name = branch.name();
        QStringView nameView(name);
        if (nameView.startsWith(QStringLiteral("refs/heads/")))
            nameView = nameView.sliced(11);

        if (nameView.startsWith(QStringLiteral("feature/"))) {
            ui->comboBox_featureBranch->addItem(nameView.toString());
        } else if (nameView.startsWith(QStringLiteral("release/"))) {
            ui->comboBox_releaseBranch->addItem(nameView.toString());
        } else if (nameView.startsWith(QStringLiteral("hotfix/"))) {
            ui->comboBox_hotfixBranch->addItem(nameView.toString());
        }
    }

    ui->pushButton_finishFeature->setEnabled(ui->comboBox_featureBranch->count() > 0);
    ui->pushButton_finishRelease->setEnabled(ui->comboBox_releaseBranch->count() > 0);
    ui->pushButton_finishHotfix->setEnabled(ui->comboBox_hotfixBranch->count() > 0);
}

bool QGitFlowDialog::hasBranch(const QString &name) const
{
    auto localBranches = m_git.branches(GIT_BRANCH_LOCAL);
    for (const auto &branch : localBranches) {
        QString bName = branch.name();
        QStringView bNameView(bName);
        if (bNameView.startsWith(QStringLiteral("refs/heads/")))
            bNameView = bNameView.sliced(11);
        if (bNameView == name)
            return true;
    }
    return false;
}

QString QGitFlowDialog::detectMainBranch() const
{
    if (hasBranch(QStringLiteral("main")))
        return QStringLiteral("main");
    if (hasBranch(QStringLiteral("master")))
        return QStringLiteral("master");
    return m_git.currentBranch();
}

bool QGitFlowDialog::ensureDevelopBranch()
{
    if (!hasBranch(QStringLiteral("develop"))) {
        QString mainBranch = detectMainBranch();
        if (mainBranch.isEmpty()) {
            QMessageBox::warning(this, tr("Git Flow Initialization"),
                                 tr("Cannot initialize Git Flow in an empty repository. Please make at least one commit first."));
            return false;
        }
        try {
            m_git.createLocalBranchInternal(QStringLiteral("develop"), mainBranch, QGit::BranchCreateOption::None);
            QMessageBox::information(this, tr("Git Flow Initialization"),
                                     tr("Branch 'develop' was automatically created from '%1'.").arg(mainBranch));
        } catch (const QGitError &err) {
            QMessageBox::critical(this, tr("Initialization Error"),
                                  tr("Failed to create 'develop' branch:\n%1").arg(err.errorString()));
            return false;
        }
    }
    return true;
}

void QGitFlowDialog::refreshAll()
{
    refreshBranches();
    if (m_repo)
        m_repo->refreshData();
}

void QGitFlowDialog::updateStartButtonState(QLineEdit *edit, QPushButton *button, const QString &prefix)
{
    const QString suffix = normalizedFlowSuffix(edit->text(), prefix);
    if (suffix.isEmpty()) {
        edit->setStyleSheet(QString());
        edit->setToolTip(QString());
        button->setEnabled(false);
        return;
    }

    const QString fullBranch = prefix + suffix;
    if (!isValidBranchRef(fullBranch)) {
        edit->setStyleSheet(errorInputStyle());
        edit->setToolTip(tr("Invalid git branch name."));
        button->setEnabled(false);
        return;
    }

    if (hasBranch(fullBranch)) {
        edit->setStyleSheet(errorInputStyle());
        edit->setToolTip(tr("Branch '%1' already exists.").arg(fullBranch));
        button->setEnabled(false);
        return;
    }

    edit->setStyleSheet(QString());
    edit->setToolTip(QString());
    button->setEnabled(true);
}

void QGitFlowDialog::startFlowBranch(const QString &prefix, QLineEdit *nameEdit, bool useDevelopBase, const QString &kindLabel)
{
    const QString suffix = normalizedFlowSuffix(nameEdit->text(), prefix);
    if (suffix.isEmpty())
        return;

    const QString branchName = prefix + suffix;
    if (!isValidBranchRef(branchName)) {
        QMessageBox::warning(this, tr("Invalid Branch Name"), tr("'%1' is not a valid git branch name.").arg(branchName));
        return;
    }
    if (hasBranch(branchName)) {
        QMessageBox::warning(this, tr("Branch Exists"), tr("Branch '%1' already exists.").arg(branchName));
        return;
    }

    try {
        QString baseBranch;
        if (useDevelopBase) {
            if (!ensureDevelopBranch())
                return;
            baseBranch = QStringLiteral("develop");
        } else {
            baseBranch = detectMainBranch();
            if (baseBranch.isEmpty()) {
                QMessageBox::warning(this, tr("Git Flow"),
                                     tr("Cannot start %1 in an empty repository. Please make at least one commit first.").arg(kindLabel.toLower()));
                return;
            }
        }

        m_git.createLocalBranchInternal(branchName, baseBranch, QGit::BranchCreateOption::Checkout);
        nameEdit->clear();
        refreshAll();
        QMessageBox::information(this, tr("%1 Started").arg(kindLabel),
                                 tr("%1 branch '%2' created and checked out from '%3'.").arg(kindLabel, branchName, baseBranch));
    } catch (const QGitError &err) {
        refreshAll();
        QMessageBox::critical(this, tr("Error"), tr("Failed to start %1:\n%2").arg(kindLabel.toLower(), err.errorString()));
    }
}

void QGitFlowDialog::finishReleaseOrHotfix(const QString &branchName, const QString &prefix, const QString &kindLabel)
{
    if (branchName.isEmpty())
        return;

    const QString tagName = branchName.startsWith(prefix) ? branchName.mid(prefix.length()) : branchName;
    const QString mainBranch = detectMainBranch();
    if (mainBranch.isEmpty()) {
        QMessageBox::warning(this, tr("Git Flow"), tr("Could not determine the main branch."));
        return;
    }

    try {
        if (!ensureDevelopBranch())
            return;

        // Merge into main
        m_git.checkoutBranchInternal(mainBranch);
        m_git.mergeInternal(branchName);

        // Tag the release/hotfix without silently overwriting an existing tag
        m_git.createTagInternal(tagName, mainBranch, QStringLiteral("%1 %2").arg(kindLabel, tagName), false);

        // Merge into develop
        m_git.checkoutBranchInternal(QStringLiteral("develop"));
        m_git.mergeInternal(branchName);

        // Delete branch
        QList<QGitBranch> toDelete;
        toDelete.append(QGitBranch(branchName, QString(), 0, GIT_BRANCH_LOCAL));
        m_git.deleteBranchesInternal(toDelete, true);

        refreshAll();
        QMessageBox::information(this, tr("%1 Finished").arg(kindLabel),
                                 tr("%1 branch '%2' merged into '%3' and 'develop', and tagged as '%4'.")
                                     .arg(kindLabel, branchName, mainBranch, tagName));
    } catch (const QGitError &err) {
        refreshAll();
        QMessageBox::critical(this, tr("Error"), tr("Failed to finish %1:\n%2").arg(kindLabel.toLower(), err.errorString()));
    }
}

void QGitFlowDialog::on_pushButton_startFeature_clicked()
{
    startFlowBranch(QStringLiteral("feature/"), ui->lineEdit_featureName, true, tr("Feature"));
}

void QGitFlowDialog::on_pushButton_finishFeature_clicked()
{
    QString branchName = ui->comboBox_featureBranch->currentText();
    if (branchName.isEmpty()) return;

    try {
        if (!ensureDevelopBranch()) return;
        m_git.checkoutBranchInternal(QStringLiteral("develop"));
        m_git.mergeInternal(branchName);

        // Delete feature branch
        QList<QGitBranch> toDelete;
        toDelete.append(QGitBranch(branchName, "", 0, GIT_BRANCH_LOCAL));
        m_git.deleteBranchesInternal(toDelete, true);

        refreshAll();
        QMessageBox::information(this, tr("Feature Finished"), tr("Feature branch '%1' merged into develop and deleted.").arg(branchName));
    } catch (const QGitError &err) {
        refreshAll();
        QMessageBox::critical(this, tr("Error"), tr("Failed to finish feature:\n%1").arg(err.errorString()));
    }
}

void QGitFlowDialog::on_pushButton_startRelease_clicked()
{
    startFlowBranch(QStringLiteral("release/"), ui->lineEdit_releaseName, true, tr("Release"));
}

void QGitFlowDialog::on_pushButton_finishRelease_clicked()
{
    finishReleaseOrHotfix(ui->comboBox_releaseBranch->currentText(), QStringLiteral("release/"), tr("Release"));
}

void QGitFlowDialog::on_pushButton_startHotfix_clicked()
{
    startFlowBranch(QStringLiteral("hotfix/"), ui->lineEdit_hotfixName, false, tr("Hotfix"));
}

void QGitFlowDialog::on_pushButton_finishHotfix_clicked()
{
    finishReleaseOrHotfix(ui->comboBox_hotfixBranch->currentText(), QStringLiteral("hotfix/"), tr("Hotfix"));
}

void QGitFlowDialog::on_lineEdit_featureName_textChanged(const QString &text)
{
    Q_UNUSED(text)
    updateStartButtonState(ui->lineEdit_featureName, ui->pushButton_startFeature, QStringLiteral("feature/"));
}

void QGitFlowDialog::on_lineEdit_releaseName_textChanged(const QString &text)
{
    Q_UNUSED(text)
    updateStartButtonState(ui->lineEdit_releaseName, ui->pushButton_startRelease, QStringLiteral("release/"));
}

void QGitFlowDialog::on_lineEdit_hotfixName_textChanged(const QString &text)
{
    Q_UNUSED(text)
    updateStartButtonState(ui->lineEdit_hotfixName, ui->pushButton_startHotfix, QStringLiteral("hotfix/"));
}
