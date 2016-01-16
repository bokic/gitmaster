#ifndef QGITREPOSITORY_H
#define QGITREPOSITORY_H

#include <QNetworkAccessManager>
#include <QListWidgetItem>
#include <QWidget>
#include "qgit.h"

namespace Ui {
class QGitRepository;
}

class QGitRepository : public QWidget
{
    Q_OBJECT

public:
    explicit QGitRepository(const QString &path, QWidget *parent);
    ~QGitRepository();

signals:
    void repositoryBranches(QDir path);
    void repositoryChangedFiles(QDir path);
    void repositoryStageFiles(QDir path, QStringList items);
    void repositoryUnstageFiles(QDir path, QStringList items);
    void repositoryCommit(QDir path, QString message);

private slots:
    void gravatarImageDownloadFinished();
    void repositoryBranchesReply(QList<QGitBranch> branches);
    void repositoryChangedFilesReply(QDir path, QMap<QString, git_status_t> files);
    void repositoryStageFilesReply(QDir path);
    void repositoryUnstageFilesReply(QDir path);
    void repositoryCommitReply(QDir path, QString commit_id);
	void repositoryError(QString qgit_function, QString git_function, int code);
    void on_repositoryDetail_currentChanged(int index);
    void on_checkBox_StagedFiles_clicked();
    void on_checkBox_UnstagedFiles_clicked();
    void on_listWidget_staged_itemChanged(QListWidgetItem *item);
    void on_listWidget_unstaged_itemChanged(QListWidgetItem *item);
    void on_pushButton_commit_clicked();

private:
    Ui::QGitRepository *ui;
    QNetworkAccessManager m_networkManager;
    QString m_path;
    QGit *m_git;
};

#endif // QGITREPOSITORY_H
