#pragma once

#include <QComboBox>
#include <QWidget>
#include <QIcon>


class QComboBoxGitViewOptions : public QComboBox
{
    Q_OBJECT
public:
    explicit QComboBoxGitViewOptions(QWidget *parent = nullptr);
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private Q_SLOTS:
    void activated(int index);

private:
    QIcon m_icon;
    QIcon m_iconFlatListSingleColumn;
    QIcon m_iconFlatListMultipleColumn;
    QIcon m_iconTreeView;
    QIcon m_iconNoStaging;
    QIcon m_iconFluidStaging;
    QIcon m_iconSplitViewStaging;
};
