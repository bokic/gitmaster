#pragma once

#include "qcomboboxgitbase.h"

class QComboBoxGitViewOptions : public QComboBoxGitBase
{
    Q_OBJECT
public:
    explicit QComboBoxGitViewOptions(QWidget *parent = nullptr);
    ~QComboBoxGitViewOptions() override = default;

protected:
    void updateIconColor() override;

private Q_SLOTS:
    void activated(int index);

private:
    void setIcon(int index);

    QIcon m_icon;
    QIcon m_iconFlatListSingleColumn;
    QIcon m_iconFlatListMultipleColumn;
    QIcon m_iconTreeView;
    QIcon m_iconNoStaging;
    QIcon m_iconSplitViewStaging;
    int m_currentView = 0;
    bool m_showIcons = false;
};
