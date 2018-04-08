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

    remeasureItems();

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
    QPainter painter(this);

    for(int c = 0; c < m_fileRects.count(); c++)
    {
        QRect rect = m_fileRects.at(c);

        if (!event->region().intersected(QRegion(rect)).isEmpty())
        {
            const QGitDiffFile &file = m_diff.files().at(c);

            painter.fillRect(rect, QColor(220,220,220));

            painter.drawText(rect.left() + 10, rect.top() + 20, file.new_file().path());
        }
    }
}

void QGitDiffWidget::remeasureItems()
{
    const int MARGIN = 10;
    int y = 0, h = 0;

    m_fileRects.resize(m_diff.files().count());

    for(int c = 0; c < m_diff.files().count(); c++)
    {
        QRect item;

        y += MARGIN;

        item.setTop(y);
        item.setLeft(MARGIN);
        item.setWidth(300);

        h = 200;
        item.setHeight(h);

        y += h;

        m_fileRects[c] = item;
    }
}
