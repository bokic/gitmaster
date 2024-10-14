#pragma once

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
    explicit QGitCloneRepositoryDialog(const QString &url, const QString &path, QWidget *parent = nullptr);
    ~QGitCloneRepositoryDialog();

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void on_pushButton_close_clicked();
	void cloneReply(QGitError error);
	void cloneTransferReply(unsigned int total_objects, unsigned int indexed_objects, unsigned int received_objects, unsigned int local_objects, unsigned int total_deltas, unsigned int indexed_deltas, size_t received_bytes);
	void cloneProgressReply(QString path, size_t completed_steps, size_t total);

private:
    Ui::QGitCloneRepositoryDialog *ui;
    QThread m_thread;
    QString m_url;
    QString m_path;
    QGit *m_git = nullptr;

signals:
	void clone(QUrl url);
};
