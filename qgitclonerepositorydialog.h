#ifndef QGITCLONEREPOSITORYDIALOG_H
#define QGITCLONEREPOSITORYDIALOG_H

#include "qgit.h"

#include <QDialog>
#include <QThread>
#include <QString>

namespace Ui {
class QGitCloneRepositoryDialog;
}

class QGitCloneRepositoryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QGitCloneRepositoryDialog(const QString &url, const QString &path, QWidget *parent = 0);
    ~QGitCloneRepositoryDialog();

private slots:
    void on_pushButton_close_clicked();
    void repositoryCloneReply(QDir path, int error);
    void repositoryCloneTransferReply(unsigned int total_objects, unsigned int indexed_objects, unsigned int received_objects, unsigned int local_objects, unsigned int total_deltas, unsigned int indexed_deltas, size_t received_bytes);
    void repositoryCloneProgressReply(QDir path, int completed_steps, int total);

private:
    Ui::QGitCloneRepositoryDialog *ui;
    QThread m_thread;
    QString m_url;
    QString m_path;
    QGit *m_git;
    bool m_aborted;

signals:
    void repositoryClone(QDir path, QUrl url);
};

#endif // QGITCLONEREPOSITORYDIALOG_H