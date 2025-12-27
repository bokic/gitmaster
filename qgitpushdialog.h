#pragma once

#include "qgitrepository.h"

#include <QStringList>
#include <QString>
#include <QDialog>


namespace Ui {
class QGitPushDialog;
}

class QGitPushDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QGitPushDialog(QGitRepository *parent = nullptr);
    ~QGitPushDialog();

    QString remote() const;
    QStringList branches() const;
    bool tags() const;
    bool force() const;

private slots:
    void on_selectAllBranches_checkBox_checkStateChanged(const Qt::CheckState &value);
    void on_remote_comboBox_currentIndexChanged(int index);

private:
    Ui::QGitPushDialog *ui = nullptr;
    QStringList m_savedBranches;
};
