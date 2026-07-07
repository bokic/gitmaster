#ifndef QGITCLEANDIALOG_H
#define QGITCLEANDIALOG_H

#include <QDialog>

namespace Ui {
class QGitCleanDialog;
}

class QGitCleanDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QGitCleanDialog(QWidget *parent = nullptr);
    ~QGitCleanDialog();

    bool removeDirectories() const;
    bool removeIgnored() const;

private:
    Ui::QGitCleanDialog *ui = nullptr;
};

#endif // QGITCLEANDIALOG_H
