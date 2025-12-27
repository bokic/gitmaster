#include "qadvplaintextedit.h"


QAdvPlainTextEdit::QAdvPlainTextEdit(QWidget *parent) : QPlainTextEdit(parent)
{

}

void QAdvPlainTextEdit::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
    {
        emit abort();
    }
}

void QAdvPlainTextEdit::focusInEvent(QFocusEvent *event)
{
    if ((event->gotFocus())&&(event->reason() == Qt::MouseFocusReason))
    {
        emit focus();
    }

    QPlainTextEdit::focusInEvent(event);
}
