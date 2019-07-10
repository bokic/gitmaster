#pragma once

#include "qcustomcombobox.h"
#include <QWidget>


class QComboBoxGitStatusFiles : public QCustomComboBox
{
    Q_OBJECT
public:
    explicit QComboBoxGitStatusFiles(QWidget *parent = nullptr);
    void updateText();
    int showFiles() const;
    int showSortBy() const;

protected:
    virtual void clicked(QListWidgetItem *item) override;
};
