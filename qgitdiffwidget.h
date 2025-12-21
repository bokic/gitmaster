#pragma once

#include "qgit.h"

#include <QWidget>
#include <QVector>
#include <QString>
#include <QRect>
#include <QList>


class QGitDiffWidgetPrivate;
class QGitDiffWidgetPrivateLine;

class QGitDiffWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY( bool readonly READ readonly WRITE setReadonly DESIGNABLE true )
public:
    explicit QGitDiffWidget(QWidget *parent = nullptr);

    void setGitDiff(const QString &first, const QString &second, const QList<QString> &files);
    void setReadonly(bool readonly = true);
    void refresh();
    bool readonly() const;

signals:
    void requestGitDiff(QString first, QString second, QList<QString> files, uint32_t context_lines);
    void select(QString file, QVector<QGitDiffWidgetLine> lines);

public slots:
    void responseGitDiff(QString first, QString second, QList<QGitDiffFile> diff, QGitError error);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    QGitDiffWidgetPrivate *m_private = nullptr;
    QString m_requestedFirst;
    QString m_requestedSecond;
    QList<QString> m_requestedFiles;
    QFont m_font;
    int m_fontHeight = 0;
    int m_fontAscent = 0;
    int m_hoverFile = -1;
    int m_hoverHunk = -1;
    int m_hoverLine = -1;
    bool m_readonly = false;
};
