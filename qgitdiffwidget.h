#pragma once

#include "qgit.h"

#include <QWidget>
#include <QVector>
#include <QRect>


class QGitDiffWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY( bool readonly READ readonly WRITE setReadonly DESIGNABLE true )
public:
    explicit QGitDiffWidget(QWidget *parent = nullptr);

    void setGitDiff(const QGitCommitDiffParent &diff);
    void setReadonly(bool readonly = true);
    bool readonly() const;

signals:

public slots:

protected:
    virtual void paintEvent(QPaintEvent *event) override;

private:
    void remeasureItems();
    QVector<QRect> m_fileRects;
    QGitCommitDiffParent m_diff;
    bool m_readonly;
};
