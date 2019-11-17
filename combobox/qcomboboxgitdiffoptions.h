#pragma once

#include "qcustomcombobox.h"
#include <QWidget>


class QComboBoxGitDiffOptions : public QCustomComboBox
{
    Q_OBJECT
public:
    explicit QComboBoxGitDiffOptions(QWidget *parent = nullptr);

protected:
    void clicked(QListWidgetItem *item) override;
};
