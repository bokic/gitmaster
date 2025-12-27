#pragma once

#include "qgitrepository.h"
#include "qgit.h"

#include <QDialog>


namespace Ui {
class QGitPullDialog;
}

class QGitPullDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QGitPullDialog(QGitRepository *parent = nullptr);
    ~QGitPullDialog();

    QGitRemote remote() const;
    QString branch() const;
    bool commitMergedChanges() const;
    bool includeMessages() const;
    bool createNewCommit() const;
    bool rebase() const;

private slots:
    void on_remote_comboBox_currentIndexChanged(int index);

private:
    Ui::QGitPullDialog *ui = nullptr;
};
