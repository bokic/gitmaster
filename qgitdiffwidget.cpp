#include "qgitdiffwidget.h"
#include <QScrollBar>
#include <QPainter>


QGitDiffWidget::QGitDiffWidget(QWidget *parent)
    : QWidget(parent)
{
    m_font = font();

    setMouseTracking(true);

    remeasureItems();
}

void QGitDiffWidget::setGitDiff(const QString &first, const QString &second, const QList<QString> &files)
{
    emit requestGitDiff(first, second, files);
}

void QGitDiffWidget::setReadonly(bool readonly)
{
    if (m_readonly != readonly)
    {
        m_readonly = readonly;

        setMouseTracking(!m_readonly);

        update();
    }
}

bool QGitDiffWidget::readonly() const
{
    return m_readonly;
}

void QGitDiffWidget::responseGitDiff(QString first, QString second, QList<QGitDiffFile> diff, QGitError error)
{
    Q_UNUSED(first)
    Q_UNUSED(second)
    Q_UNUSED(error)

    m_diff = diff;

    remeasureItems();

    update();
}

void QGitDiffWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    int y = 0;

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
                const auto &hunk = hunks.at(h);
                const auto lines = hunk.lines();

                if (h > 0) y += m_fontHeight + 1;

                for(int l = 0; l < lines.count(); l++)
                {
                    QString old_lineNo, new_lineNo;
                    const auto &line = lines.at(l);

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

                    if ((!m_readonly)&&(c == m_hoverFile)&&(h == m_hoverHunk)&&(m_hoverLine == l))
                    {
                        painter.setPen(Qt::SolidLine);
                        painter.setBrush(Qt::NoBrush);
                        painter.setPen(Qt::darkRed);
                        painter.drawRect(70, y, m_fontHeight, m_fontHeight);
                    }

                    painter.drawText(100, yFont, line.content());

                    y += m_fontHeight + 1;
                }

                if ((!m_readonly)&&(c == m_hoverFile)&&(h == m_hoverHunk)&&(m_hoverLine == -1))
                {
                    painter.setPen(Qt::SolidLine);
                    painter.setBrush(Qt::NoBrush);
                    painter.setPen(Qt::darkRed);
                    painter.drawRect(10, y, 60, -lines.count()*(m_fontHeight + 1));
                }
            }
        }
    }
}

void QGitDiffWidget::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event)

    if ((!m_readonly)&&(m_hoverFile >= 0)&&(m_hoverHunk >= 0)&&(m_hoverLine >= -1))
    {
        emit select(m_hoverFile, m_hoverHunk, m_hoverLine);
    }
}

void QGitDiffWidget::mouseMoveEvent(QMouseEvent *event)
{
    int l_hoverFile = -1;
    int l_hoverHunk = -1;
    int l_hoverLine = -1;

    if (m_readonly)
        return;

    for(const auto &rect : m_fileRects)
    {
        if (rect.contains(event->localPos().toPoint()))
        {
            l_hoverFile = m_fileRects.indexOf(rect);
            l_hoverHunk = 0;
            l_hoverLine = -1;

            break;
        }
    }

    if ((m_hoverFile != l_hoverFile)||(m_hoverHunk != l_hoverHunk)||(m_hoverLine != l_hoverLine))
    {
        m_hoverFile = l_hoverFile;
        m_hoverHunk = l_hoverHunk;
        m_hoverLine = l_hoverLine;

        if (m_hoverHunk >= 0) {
            setCursor(Qt::PointingHandCursor);
        } else {
            setCursor(Qt::ArrowCursor);
        }

        update();
    }
}

void QGitDiffWidget::remeasureItems()
{
    const int MARGIN = 10;
    int y = 0, h = 0, lineMax = 0;
    QSize newSize;

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

        h = m_fontHeight * 2;

        for(int hunk = 0; hunk < m_diff.at(c).hunks().count(); hunk++)
        {
            if (hunk > 0)
                h += m_fontHeight + 1;

            h += m_diff.at(c).hunks().at(hunk).lines().count() * (m_fontHeight + 1);
        }

        item.setHeight(h);

        y += h;

        m_fileRects[c] = item;
    }

    newSize = QSize(MARGIN + 100 + lineMax + (MARGIN * 2), y + (MARGIN * 2));

    setMinimumSize(newSize);
    setMaximumSize(newSize);
}
