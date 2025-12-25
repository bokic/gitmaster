#pragma once

#include <QPlainTextEdit>

class QAdvPlainTextEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit QAdvPlainTextEdit(QWidget *parent = nullptr);
    ~QAdvPlainTextEdit() = default;

signals:
    void focus();

public slots:

protected:
    void focusInEvent(QFocusEvent *event) override;
};
