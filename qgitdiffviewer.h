#pragma once

#include <QWidget>


class QGitDiffViewer : public QWidget
{
    Q_OBJECT
public:
    explicit QGitDiffViewer(QWidget *parent = 0);

signals:

public slots:

protected:
    virtual void paintEvent(QPaintEvent *event) override;

};
