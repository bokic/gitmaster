#include "qgitdiffwidget.h"
#include <QPainter>


QGitDiffWidget::QGitDiffWidget(QWidget *parent)
    : QWidget(parent)
    , m_readonly(false)
{

}

void QGitDiffWidget::setGitDiff(const QList<QGitDiffFile> &diff)
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

    int y = 0;

    for(int c = 0; c < m_fileRects.count(); c++)
    {
        QRect rect = m_fileRects.at(c);

        if (!event->region().intersected(QRegion(rect)).isEmpty())
        {
            const auto file = m_diff.at(c);
            const auto hunks = file.hunks();

            painter.fillRect(rect, QColor(220,220,220));

            painter.drawText(rect.left() + 10, rect.top() + 20, file.new_file().path());

            for(int h = 0; h < hunks.count(); h++)
            {
                y += 12;


                const auto hunk = hunks.at(h);
                const auto lines = hunk.lines();

                for(int l = 0; l < lines.count(); l++)
                {
                    QString old_lineNo, new_lineNo;
                    const auto line = lines.at(l);

                    if (line.old_lineno() >= 0) old_lineNo = QString::number(line.old_lineno());
                    if (line.new_lineno() >= 0) new_lineNo = QString::number(line.new_lineno());

                    painter.drawText(10, y, old_lineNo);
                    painter.drawText(40, y, new_lineNo);

                    painter.drawText(70, y, QString(QChar(line.origin())));

                    painter.drawText(100, y, line.content());

                    y += 13;
                }
            }
        }
    }
}

void QGitDiffWidget::remeasureItems()
{
    const int MARGIN = 10;
    int y = 0, h = 0;

    m_fileRects.resize(m_diff.count());

    for(int c = 0; c < m_diff.count(); c++)
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
