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
    virtual QSize sizeHint() const override final;
    virtual QSize minimumSizeHint() const override final;
    void setCurrentText(const QString &text);
    void setCurrentIcon(const QIcon &icon);

signals:
    void itemClicked(QListWidgetItem *item);

public slots:

protected:
    virtual void paintEvent(QPaintEvent *event) override final;
    virtual void mousePressEvent(QMouseEvent *event) override final;

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
