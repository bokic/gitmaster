#pragma once

#include "qgit.h"

#include <QWidget>
#include <QVector>
#include <QRect>
#include <QList>


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

public slots:
    void responseGitDiff(QString first, QString second, QList<QGitDiffFile> diff, QGitError error);

protected:
    virtual void paintEvent(QPaintEvent *event) override;

private:
    void remeasureItems();
    QList<QGitDiffFile> m_diff;
    QVector<QRect> m_fileRects;
    QFont m_font;
    int m_fontHeight = 0;
    int m_fontAscent = 0;
    bool m_readonly = false;
};
