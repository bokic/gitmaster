#include "qgitflowdialog.h"
#include "ui_qgitflowdialog.h"
#include "qgitrepository.h"
#include <QMessageBox>

QGitFlowDialog::QGitFlowDialog(QGitRepository *parent)
    : QDialog(parent)
    , ui(new Ui::QGitFlowDialog)
    , m_repo(parent)
{
    ui->setupUi(this);
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
            m_git.createLocalBranch(QStringLiteral("develop"), mainBranch, false);
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

void QGitFlowDialog::on_pushButton_startFeature_clicked()
{
    QString name = ui->lineEdit_featureName->text().trimmed();
    if (name.isEmpty()) return;

    QString branchName = QStringLiteral("feature/") + name;
    if (hasBranch(branchName)) {
        QMessageBox::warning(this, tr("Branch Exists"), tr("Branch '%1' already exists.").arg(branchName));
        return;
    }

    try {
        if (!ensureDevelopBranch()) return;
        m_git.createLocalBranch(branchName, QStringLiteral("develop"), true);
        ui->lineEdit_featureName->clear();
        refreshBranches();
        m_repo->refreshData();
        QMessageBox::information(this, tr("Feature Started"), tr("Feature branch '%1' created and checked out.").arg(branchName));
    } catch (const QGitError &err) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to start feature:\n%1").arg(err.errorString()));
    }
}

void QGitFlowDialog::on_pushButton_finishFeature_clicked()
{
    QString branchName = ui->comboBox_featureBranch->currentText();
    if (branchName.isEmpty()) return;

    try {
        if (!ensureDevelopBranch()) return;
        m_git.checkoutBranch(QStringLiteral("develop"));
        m_git.merge(branchName);

        // Delete feature branch
        QList<QGitBranch> toDelete;
        toDelete.append(QGitBranch(branchName, "", 0, GIT_BRANCH_LOCAL));
        m_git.deleteBranches(toDelete, true);

        refreshBranches();
        m_repo->refreshData();
        QMessageBox::information(this, tr("Feature Finished"), tr("Feature branch '%1' merged into develop and deleted.").arg(branchName));
    } catch (const QGitError &err) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to finish feature:\n%1").arg(err.errorString()));
    }
}

void QGitFlowDialog::on_pushButton_startRelease_clicked()
{
    QString name = ui->lineEdit_releaseName->text().trimmed();
    if (name.isEmpty()) return;

    QString branchName = QStringLiteral("release/") + name;
    if (hasBranch(branchName)) {
        QMessageBox::warning(this, tr("Branch Exists"), tr("Branch '%1' already exists.").arg(branchName));
        return;
    }

    try {
        if (!ensureDevelopBranch()) return;
        m_git.createLocalBranch(branchName, QStringLiteral("develop"), true);
        ui->lineEdit_releaseName->clear();
        refreshBranches();
        m_repo->refreshData();
        QMessageBox::information(this, tr("Release Started"), tr("Release branch '%1' created and checked out.").arg(branchName));
    } catch (const QGitError &err) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to start release:\n%1").arg(err.errorString()));
    }
}

void QGitFlowDialog::on_pushButton_finishRelease_clicked()
{
    QString branchName = ui->comboBox_releaseBranch->currentText();
    if (branchName.isEmpty()) return;

    QString releaseName = branchName.mid(8); // remove "release/"
    QString mainBranch = detectMainBranch();

    try {
        // Merge into main
        m_git.checkoutBranch(mainBranch);
        m_git.merge(branchName);

        // Tag the release
        m_git.createTag(releaseName, mainBranch, QString("Release %1").arg(releaseName), true);

        // Merge into develop
        if (!ensureDevelopBranch()) return;
        m_git.checkoutBranch(QStringLiteral("develop"));
        m_git.merge(branchName);

        // Delete release branch
        QList<QGitBranch> toDelete;
        toDelete.append(QGitBranch(branchName, "", 0, GIT_BRANCH_LOCAL));
        m_git.deleteBranches(toDelete, true);

        refreshBranches();
        m_repo->refreshData();
        QMessageBox::information(this, tr("Release Finished"), tr("Release branch '%1' merged into '%2' and 'develop', and tagged as '%3'.").arg(branchName, mainBranch, releaseName));
    } catch (const QGitError &err) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to finish release:\n%1").arg(err.errorString()));
    }
}

void QGitFlowDialog::on_pushButton_startHotfix_clicked()
{
    QString name = ui->lineEdit_hotfixName->text().trimmed();
    if (name.isEmpty()) return;

    QString branchName = QStringLiteral("hotfix/") + name;
    if (hasBranch(branchName)) {
        QMessageBox::warning(this, tr("Branch Exists"), tr("Branch '%1' already exists.").arg(branchName));
        return;
    }

    try {
        QString mainBranch = detectMainBranch();
        m_git.createLocalBranch(branchName, mainBranch, true);
        ui->lineEdit_hotfixName->clear();
        refreshBranches();
        m_repo->refreshData();
        QMessageBox::information(this, tr("Hotfix Started"), tr("Hotfix branch '%1' created and checked out from '%2'.").arg(branchName, mainBranch));
    } catch (const QGitError &err) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to start hotfix:\n%1").arg(err.errorString()));
    }
}

void QGitFlowDialog::on_pushButton_finishHotfix_clicked()
{
    QString branchName = ui->comboBox_hotfixBranch->currentText();
    if (branchName.isEmpty()) return;

    QString hotfixName = branchName.mid(7); // remove "hotfix/"
    QString mainBranch = detectMainBranch();

    try {
        // Merge into main
        m_git.checkoutBranch(mainBranch);
        m_git.merge(branchName);

        // Tag the hotfix
        m_git.createTag(hotfixName, mainBranch, QString("Hotfix %1").arg(hotfixName), true);

        // Merge into develop
        if (!ensureDevelopBranch()) return;
        m_git.checkoutBranch(QStringLiteral("develop"));
        m_git.merge(branchName);

        // Delete hotfix branch
        QList<QGitBranch> toDelete;
        toDelete.append(QGitBranch(branchName, "", 0, GIT_BRANCH_LOCAL));
        m_git.deleteBranches(toDelete, true);

        refreshBranches();
        m_repo->refreshData();
        QMessageBox::information(this, tr("Hotfix Finished"), tr("Hotfix branch '%1' merged into '%2' and 'develop', and tagged as '%3'.").arg(branchName, mainBranch, hotfixName));
    } catch (const QGitError &err) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to finish hotfix:\n%1").arg(err.errorString()));
    }
}

void QGitFlowDialog::on_lineEdit_featureName_textChanged(const QString &text)
{
    ui->pushButton_startFeature->setEnabled(!text.trimmed().isEmpty());
}

void QGitFlowDialog::on_lineEdit_releaseName_textChanged(const QString &text)
{
    ui->pushButton_startRelease->setEnabled(!text.trimmed().isEmpty());
}

void QGitFlowDialog::on_lineEdit_hotfixName_textChanged(const QString &text)
{
    ui->pushButton_startHotfix->setEnabled(!text.trimmed().isEmpty());
}
