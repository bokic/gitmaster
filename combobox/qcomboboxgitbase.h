#pragma once

#include <QComboBox>
#include <QIcon>
#include <QSize>

class QStandardItemModel;
class QStandardItem;

class QComboBoxGitBase : public QComboBox
{
    Q_OBJECT
public:
    explicit QComboBoxGitBase(QWidget *parent = nullptr);
    ~QComboBoxGitBase() override = default;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void changeEvent(QEvent *event) override;
    void showPopup() override;

    virtual void updateIconColor() {}

    QStandardItemModel *initStandardModel();
    QStandardItem *addOptionItem(const QString &text, bool isSelectable = true, Qt::CheckState checkState = Qt::Unchecked, const QIcon &icon = QIcon());
    QStandardItem *addHeaderItem(const QString &text, const QIcon &icon = QIcon());

    QIcon m_icon;
    QString m_displayText;
    bool m_showIcons = false;
};
