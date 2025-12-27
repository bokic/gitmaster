#pragma once

#include <QPlainTextEdit>

class QAdvPlainTextEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit QAdvPlainTextEdit(QWidget *parent = nullptr);
    ~QAdvPlainTextEdit() = default;

signals:
    void abort();
    void focus();

public slots:

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
};
