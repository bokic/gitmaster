#pragma once

#include <QDialog>
#include <QString>
#include <QDir>
#include "qgit.h"

namespace Ui {
class QGitRemoveSubmoduleDialog;
}

class QGitRemoveSubmoduleDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QGitRemoveSubmoduleDialog(
        const QDir &repoPath,
        const QGitSubmodule &submodule,
        QWidget *parent = nullptr);
    ~QGitRemoveSubmoduleDialog() override;

    QString submoduleName() const;
    bool removeWorkingDirectory() const;
    bool removeGitDir() const;
    bool force() const;

private:
    Ui::QGitRemoveSubmoduleDialog *ui;
    QDir m_repoPath;
    QGitSubmodule m_submodule;
};
