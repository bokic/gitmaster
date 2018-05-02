#pragma once

#include "qgit.h"

#include <QAbstractScrollArea>
#include <QVector>
#include <QRect>
#include <QList>


class QGitDiffWidget : public QAbstractScrollArea
{
    Q_OBJECT
    Q_PROPERTY( bool readonly READ readonly WRITE setReadonly DESIGNABLE true )
public:
    explicit QGitDiffWidget(QWidget *parent = nullptr);

    void setGitDiff(const QList<QGitDiffFile> &diff);
    void setReadonly(bool readonly = true);
    bool readonly() const;

signals:

public slots:

protected:
    virtual void paintEvent(QPaintEvent *event) override;

private:
    void remeasureItems();
    QList<QGitDiffFile> m_diff;
    QVector<QRect> m_fileRects;
    QFont m_font;
    int m_fontHeight = 0;
    bool m_readonly = false;
};
