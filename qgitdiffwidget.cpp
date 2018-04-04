#include "qgitdiffwidget.h"
#include <QPainter>


QGitDiffWidget::QGitDiffWidget(QWidget *parent)
    : QWidget(parent)
    , m_readonly(false)
{

}

void QGitDiffWidget::setGitDiff(const QGitCommitDiffParent &diff)
{
    m_diff = diff;

    //  TODO: Remeasure!

    update();
}

void QGitDiffWidget::setReadonly(bool readonly)
{
    if (m_readonly != readonly)
    {
        m_readonly = readonly;

        update();
    }
}

bool QGitDiffWidget::readonly() const
{
    return m_readonly;
}

void QGitDiffWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
}
