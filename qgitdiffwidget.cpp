#include "qgitdiffwidget.h"
#include <QScrollBar>
#include <QPainter>
#include <QDebug>


QGitDiffWidget::QGitDiffWidget(QWidget *parent)
    : QAbstractScrollArea(parent)
{
    int distance = 0;

    m_font = font();

    remeasureItems();

    horizontalScrollBar()->setSingleStep(m_fontHeight);
    verticalScrollBar()->setSingleStep(m_fontHeight);

    distance = m_fontHeight * 3;
    horizontalScrollBar()->setPageStep(distance);
    verticalScrollBar()->setPageStep(distance);
}

void QGitDiffWidget::setGitDiff(const QList<QGitDiffFile> &diff)
{
    m_diff = diff;

    remeasureItems();

    verticalScrollBar()->setMinimum(0);   verticalScrollBar()->setMaximum(100);
    horizontalScrollBar()->setMinimum(0); horizontalScrollBar()->setMaximum(100);
    viewport()->update();
}

void QGitDiffWidget::setReadonly(bool readonly)
{
    if (m_readonly != readonly)
    {
        m_readonly = readonly;

        viewport()->update();
    }
}

bool QGitDiffWidget::readonly() const
{
    return m_readonly;
}

void QGitDiffWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(viewport());

    int y = 0;

    painter.translate(-horizontalScrollBar()->value(), -verticalScrollBar()->value());

    for(int c = 0; c < m_fileRects.count(); c++)
    {
        const QRect &rect = m_fileRects.at(c);

        if (!event->region().intersected(QRegion(rect)).isEmpty())
        {
            const auto &file = m_diff.at(c);
            const auto &hunks = file.hunks();

            painter.fillRect(rect, QColor(220,220,220));

            painter.drawText(rect.left() + 10, rect.top() + 20, file.new_file().path());

            y = rect.top() + (m_fontHeight * 2);

            for(int h = 0; h < hunks.count(); h++)
            {
                const auto hunk = hunks.at(h);
                const auto lines = hunk.lines();

                for(int l = 0; l < lines.count(); l++)
                {
                    QString old_lineNo, new_lineNo;
                    const auto line = lines.at(l);

                    if (line.old_lineno() >= 0) old_lineNo = QString::number(line.old_lineno());
                    if (line.new_lineno() >= 0) new_lineNo = QString::number(line.new_lineno());

                    int yFont = y + m_fontAscent;

                    if (line.origin() == '-')
                    {
                        painter.setPen(Qt::NoPen);
                        painter.setBrush(QBrush(QColor(235, 204, 204)));
                        painter.drawRect(10, y, rect.width(), m_fontHeight + 1);
                        painter.setBrush(QBrush(QColor(Qt::darkRed)));
                        painter.setPen(Qt::darkRed);
                    } else if (line.origin() == '+') {
                        painter.setPen(Qt::NoPen);
                        painter.setBrush(QBrush(QColor(204, 230, 194)));
                        painter.drawRect(10, y, rect.width(), m_fontHeight + 1);
                        painter.setPen(Qt::darkGreen);
                    } else {
                        painter.setPen(Qt::SolidLine);
                        painter.setBrush(QBrush(QColor(Qt::black)));
                    }

                    painter.drawText(10, yFont, old_lineNo);
                    painter.drawText(40, yFont, new_lineNo);

                    painter.drawText(70, yFont, QString(QChar(line.origin())));

                    painter.drawText(100, yFont, line.content());

                    y += m_fontHeight + 1;
                }
            }
        }
    }
}

void QGitDiffWidget::remeasureItems()
{
    const int MARGIN = 10;
    int y = 0, h = 0, lineMax = 0;

    QFontMetrics fm(m_font);

    m_fontHeight = fm.height();
    m_fontAscent = fm.ascent();

    m_fileRects.resize(m_diff.count());

    for(const QGitDiffFile &diffFile: m_diff)
    {
        for(const QGitDiffHunk &diffHunk: diffFile.hunks())
        {
            for(const QGitDiffLine &diffLine: diffHunk.lines())
            {
                auto line = diffLine.content();

                int currentLine = fm.width(QString::fromUtf8(line));

                if (currentLine > lineMax)
                    lineMax = currentLine;
            }
        }
    }

    for(int c = 0; c < m_diff.count(); c++)
    {
        QRect item;

        y += MARGIN;

        item.setTop(y);
        item.setLeft(MARGIN);
        item.setWidth(100 + lineMax);

        h = 200;
        item.setHeight(h);

        y += h;

        m_fileRects[c] = item;
    }
}
