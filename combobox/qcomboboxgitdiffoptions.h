#pragma once

#include <QComboBox>
#include <QWidget>
#include <QIcon>


class QComboBoxGitDiffOptions : public QComboBox
{
    Q_OBJECT
public:
    explicit QComboBoxGitDiffOptions(QWidget *parent = nullptr);
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private Q_SLOTS:
    void activated(int index);

private:
    QIcon m_icon;
};
