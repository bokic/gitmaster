#pragma once

#include "qgit.h"

#include <QWidget>


class QGitDiffWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY( bool readonly READ readonly WRITE setReadonly DESIGNABLE true )
public:
    explicit QGitDiffWidget(QWidget *parent = 0);

    void setGitDiff(const QGitCommitDiffParent &diff);
    void setReadonly(bool readonly = true);
    bool readonly() const;

signals:

public slots:

private:
    QGitCommitDiffParent m_diff;
    bool m_readonly;
};
