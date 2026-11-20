#pragma once

#include <QTreeWidget>
#include <QThread>
#include <QDir>
#include "qgit.h"

class QGitRepoTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    explicit QGitRepoTreeWidget(QWidget *parent = nullptr);
    ~QGitRepoTreeWidget();

    void refreshItems(bool notifyOnChange = false);

signals:
    void repositoryStatus();
    void repositoryChanged(const QString &path);
    void repositoriesDropped(const QStringList &paths);

public slots:

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    void refreshItem();
    QThread m_thread;
    QGit *m_git = nullptr;
    int m_refreshIndex = 0;
    bool m_isRefreshing = false;
    bool m_refreshPending = false;
    bool m_notifyOnChange = false;
    bool m_pendingNotifyOnChange = false;
    QString m_currentRefreshingPath;

private slots:
    void repositoryStatusReply(QMap<git_status_t, int> items, QGitError error);
};
