#pragma once

#include "qgit.h"

#include <QWidget>
#include <QVector>
#include <QRect>
#include <QList>


class QGitDiffWidgetPrivate;

class QGitDiffWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY( bool readonly READ readonly WRITE setReadonly DESIGNABLE true )
public:
    explicit QGitDiffWidget(QWidget *parent = nullptr);

    void setGitDiff(const QString &first, const QString &second, const QList<QString> &files);
    void setReadonly(bool readonly = true);
    bool readonly() const;

signals:
    void requestGitDiff(QString first, QString second, QList<QString> files);
    void select(int file, int hunk, int line);

public slots:
    void responseGitDiff(QString first, QString second, QList<QGitDiffFile> diff, QGitError error);

protected:
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;

private:
    QGitDiffWidgetPrivate *m_private = nullptr;
    QFont m_font;
    int m_fontHeight = 0;
    int m_fontAscent = 0;
    int m_hoverFile = -1;
    int m_hoverHunk = -1;
    int m_hoverLine = -1;
    bool m_readonly = false;
};
