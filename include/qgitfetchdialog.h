#pragma once

#include <QDialog>
#include <QWidget>


namespace Ui {
class QGitFetchDialog;
}

class QGitFetchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QGitFetchDialog(QWidget *parent = nullptr);
    ~QGitFetchDialog();

    QString remote() const;
    bool fetchFromAllRemotes() const;
    bool purgeDeletedBranches() const;
    bool fetchAllTags() const;
    bool recurseSubmodules() const;

private slots:
    void on_checkBox_fetchFromAllRemotes_toggled(bool checked);

private:
    Ui::QGitFetchDialog *ui = nullptr;
};
