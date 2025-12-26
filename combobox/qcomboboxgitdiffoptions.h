#pragma once

#include <QComboBox>
#include <QWidget>
#include <QIcon>


class QComboBoxGitDiffOptions : public QComboBox
{
    Q_OBJECT
public:
    explicit QComboBoxGitDiffOptions(QWidget *parent = nullptr);
    ~QComboBoxGitDiffOptions() = default;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void showPopup() override;

private Q_SLOTS:
    void activated(int index);

private:
    QIcon m_icon;
    QIcon m_iconChecked;
    QIcon m_iconUnchecked;
};
