#pragma once

#include <QComboBox>
#include <QListWidget>
#include <QFrame>
#include <QIcon>


class QCustomComboBox: public QComboBox
{
    Q_OBJECT
public:
    explicit QCustomComboBox(QWidget *parent = nullptr);
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
    void setCurrentText(const QString &text);
    void setCurrentIcon(const QIcon &icon);

signals:
    void itemClicked(QListWidgetItem *item);

public slots:

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void listItemClicked(QListWidgetItem *item);

protected:
    virtual void clicked(QListWidgetItem *item) = 0;
    QFrame *m_popup = nullptr;
    QListWidget *m_list = nullptr;

private:
    QString m_text;
    QIcon m_icon;
};
