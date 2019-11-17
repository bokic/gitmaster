#pragma once

#include "qcustomcombobox.h"
#include <QWidget>


class QComboBoxGitViewOptions : public QCustomComboBox
{
    Q_OBJECT
public:
    explicit QComboBoxGitViewOptions(QWidget *parent = nullptr);
    void updateIcon();
    int viewType() const;
    int stagingType() const;

protected:
    void clicked(QListWidgetItem *item) override;
};
