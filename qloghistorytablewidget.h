#pragma once

#include "qgitcommit.h"

#include <QTableWidget>
#include <QString>
#include <QList>


class QLogHistoryTableWidget : public QTableWidget
{
public:
    explicit QLogHistoryTableWidget(QWidget *parent = nullptr);

    void addCommit(const QGitCommit &commit);
    bool selectCommit(const QString &hash);

private:
    QList<QString> m_hashIndex;
};
