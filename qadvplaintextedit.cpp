#include "qadvplaintextedit.h"

QAdvPlainTextEdit::QAdvPlainTextEdit(QWidget *parent) : QPlainTextEdit(parent)
{

}

void QAdvPlainTextEdit::focusInEvent(QFocusEvent *event)
{
    if ((event->gotFocus())&&(event->reason() == Qt::MouseFocusReason))
    {
        emit focus();
    }

    QPlainTextEdit::focusInEvent(event);
}
