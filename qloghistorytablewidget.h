#pragma once

#include "qgitcommit.h"

#include <QTableWidget>
#include <QString>
#include <QList>


class QLogHistoryTableWidget : public QTableWidget
{
public:
    QLogHistoryTableWidget(QWidget *parent = nullptr);

    void addCommit(const QGitCommit &commit);

private:
    QList<QString> m_hashIndex;
};
