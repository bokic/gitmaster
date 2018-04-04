#pragma once

#include <QWidget>


class QGitDiffViewer : public QWidget
{
    Q_OBJECT
public:
    explicit QGitDiffViewer(QWidget *parent = nullptr);

signals:

public slots:

protected:
    virtual void paintEvent(QPaintEvent *event) override;

};
