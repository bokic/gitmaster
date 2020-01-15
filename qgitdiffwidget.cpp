#include "qgitdiffwidget.h"
#include <QPaintEvent>
#include <QScrollBar>
#include <QPainter>
#include <QObject>
#include <QDebug>


class QGitDiffWidgetPrivateLine
{
public:
    QByteArray content;
    git_off_t offset = 0;
    int new_lineno = 0;
    int num_lines = 0;
    int old_lineno = 0;
    char origin = '\0';
    QRect rect;
};

class QGitDiffWidgetPrivateHunk
{
public:
    QString header;
    int new_lines = -1;
    int new_start = -1;
    int old_lines = -1;
    int old_start = -1;

    QVector<QGitDiffWidgetPrivateLine> lines;
    QRect rect;
};

class QGitDiffWidgetPrivateFile
{
public:
    QGitDiffFileItem new_file;
    QGitDiffFileItem old_file;
    uint32_t flags = 0;
    int nfiles = 0;
    int simularity = 0;
    int status = 0;

    QVector<QGitDiffWidgetPrivateHunk> hunks;
    QRect rect;
};

class QGitDiffWidgetPrivate : private QObject
{
public:
    QGitDiffWidgetPrivate(QObject *parent = nullptr): QObject(parent)
    {

    }

    void clear()
    {
        files.clear();
        rect = QRect();
    }

    QVector<QGitDiffWidgetPrivateFile> files;
    QRect rect;
};

QGitDiffWidget::QGitDiffWidget(QWidget *parent)
    : QWidget(parent)
{
    m_private = new QGitDiffWidgetPrivate(this);

    m_font = font();

    setMouseTracking(true);
}

void QGitDiffWidget::setGitDiff(const QString &first, const QString &second, const QList<QString> &files)
{
    m_private->clear();

    emit requestGitDiff(first, second, files);

    update();
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
    const int MARGIN = 10;
    int y = 0, h = 0, lineMax = 0;
    QSize newSize;

    QFontMetrics fm(m_font);

    Q_UNUSED(first)
    Q_UNUSED(second)
    Q_UNUSED(error)

    m_private->clear();

    m_fontHeight = fm.height();
    m_fontAscent = fm.ascent();

    for(const QGitDiffFile &diffFile: diff)
    {
        for(const QGitDiffHunk &diffHunk: diffFile.hunks())
        {
            for(const QGitDiffLine &diffLine: diffHunk.lines())
            {
                auto line = diffLine.content();

                int currentLine = fm.horizontalAdvance(QString::fromUtf8(line));

                if (currentLine > lineMax)
                    lineMax = currentLine;
            }
        }
    }

    for(int c = 0; c < diff.count(); c++)
    {
        const auto &file_item = diff.at(c);
        QGitDiffWidgetPrivateFile file;

        y += MARGIN;

        file.new_file = file_item.new_file();
        file.old_file = file_item.old_file();
        file.flags = file_item.flags();
        file.nfiles = file_item.nfiles();
        file.simularity = file_item.simularity();
        file.status = file_item.status();

        file.rect.setTop(y);
        file.rect.setLeft(MARGIN);
        file.rect.setWidth(100 + lineMax); // TODO: Unhardcode 100

        h = m_fontHeight * 2;

        for(int pos_hunk = 0; pos_hunk < file_item.hunks().count(); pos_hunk++)
        {            
            const auto &hunk_item = file_item.hunks().at(pos_hunk);
            QGitDiffWidgetPrivateHunk hunk;

            hunk.header = hunk_item.header();
            hunk.new_lines = hunk_item.new_lines();
            hunk.new_start = hunk_item.new_start();
            hunk.old_lines = hunk_item.old_lines();
            hunk.old_start = hunk_item.old_start();

            if (pos_hunk > 0)
                h += m_fontHeight + 1;

            hunk.rect.setTop(y + h);
            hunk.rect.setLeft(MARGIN);
            hunk.rect.setWidth(100 + lineMax); // TODO: Unhardcode 100

            for(int pos_line = 0; pos_line < hunk_item.lines().count(); pos_line++)
            {
                const auto &line_item = hunk_item.lines().at(pos_line);
                QGitDiffWidgetPrivateLine line;

                line.content = line_item.content();
                line.offset = line_item.offset();
                line.new_lineno = line_item.new_lineno();
                line.num_lines = line_item.num_lines();
                line.old_lineno = line_item.old_lineno();
                line.origin = line_item.origin();

                line.rect.setTop(y + h);
                line.rect.setLeft(MARGIN);
                line.rect.setWidth(100 + lineMax); // TODO: Unhardcode 100

                h += m_fontHeight + 1;

                line.rect.setHeight(m_fontHeight + 1);

                hunk.lines.push_back(line);
            }

            hunk.rect.setHeight(h);

            file.hunks.push_back(hunk);
        }

        file.rect.setHeight(h);

        y += h;

        m_private->files.push_back(file);
    }

    newSize = QSize(MARGIN + 100 + lineMax + (MARGIN * 2), y + (MARGIN * 2)); // TODO: Unhardcode 100

    setMinimumSize(newSize);
    setMaximumSize(newSize);

    update();
}

void QGitDiffWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    int fileIndex = 0, hunkIndex = 0, lineIndex = 0;

    fileIndex = 0;
    for(const auto &file: m_private->files)
    {
        if (!event->region().intersected(file.rect).isEmpty())
        {
            painter.fillRect(file.rect, QColor(220,220,220));

            painter.drawText(file.rect.left() + 10, file.rect.top() + 20, file.new_file.path());

            hunkIndex = 0;
            for(const auto &hunk: file.hunks)
            {
                if (!event->region().intersected(hunk.rect).isEmpty())
                {
                    lineIndex = 0;
                    for(const auto &line: hunk.lines)
                    {
                        if (!event->region().intersected(line.rect).isEmpty())
                        {
                            QString old_lineNo, new_lineNo;
                            if (line.old_lineno >= 0) old_lineNo = QString::number(line.old_lineno);
                            if (line.new_lineno >= 0) new_lineNo = QString::number(line.new_lineno);

                            int yFont = line.rect.top() + m_fontAscent;

                            if (line.origin == '-')
                            {
                                painter.setPen(Qt::NoPen);
                                painter.setBrush(QBrush(QColor(235, 204, 204)));
                                painter.drawRect(line.rect);
                                painter.setBrush(QBrush(QColor(Qt::darkRed)));
                                painter.setPen(Qt::darkRed);
                            } else if (line.origin == '+') {
                                painter.setPen(Qt::NoPen);
                                painter.setBrush(QBrush(QColor(204, 230, 194)));
                                painter.drawRect(line.rect);
                                painter.setPen(Qt::darkGreen);
                            } else {
                                painter.setPen(Qt::SolidLine);
                                painter.setBrush(QBrush(QColor(Qt::black)));
                            }

                            painter.drawText(10, yFont, old_lineNo);
                            painter.drawText(40, yFont, new_lineNo);

                            painter.drawText(100, yFont, line.content);

                            if ((fileIndex == m_hoverFile)&&(hunkIndex == m_hoverHunk)&&(lineIndex == m_hoverLine))
                            {
                                painter.setPen(Qt::SolidLine);
                                painter.setBrush(QBrush());

                                painter.drawRect(line.rect.adjusted(100, 0, -1, -1));
                            }
                        }

                        lineIndex++;
                    }

                    if ((fileIndex == m_hoverFile)&&(hunkIndex == m_hoverHunk)&&(lineIndex == -1))
                    {
                        painter.setPen(Qt::SolidLine);
                        painter.setBrush(QBrush());

                        painter.drawRect(hunk.rect);

                        qDebug() << "hunk";
                    }
                }

                hunkIndex++;
            }
        }

        fileIndex++;
    }
}

void QGitDiffWidget::mousePressEvent(QMouseEvent *event)
{
    QVector<QGitDiffWidgetLine> lines;

    Q_UNUSED(event)

    if ((!m_readonly)&&(m_hoverFile >= 0)&&(m_hoverHunk >= 0)&&(m_hoverLine >= -1))
    {
        auto file = m_private->files.at(m_hoverFile);

        if (m_hoverLine < 0)
        {
            for(const auto &line: file.hunks.at(m_hoverHunk).lines)
            {
                QGitDiffWidgetLine newLine;

                newLine.content = line.content;
                newLine.new_lineno = line.new_lineno;
                newLine.old_lineno = line.old_lineno;
                newLine.origin = line.origin;

                lines.push_back(newLine);
            }
        }
        else
        {
            QGitDiffWidgetLine newLine;

            const auto &line = file.hunks.at(m_hoverHunk).lines.at(m_hoverLine);

            newLine.content = line.content;
            newLine.new_lineno = line.new_lineno;
            newLine.old_lineno = line.old_lineno;
            newLine.origin = line.origin;

            lines.push_back(newLine);
        }

        emit select(file.new_file.path(), lines);
    }
}

void QGitDiffWidget::mouseMoveEvent(QMouseEvent *event)
{
    int l_hoverFile = -1;
    int l_hoverHunk = -1;
    int l_hoverLine = -1;

    if (m_readonly)
        return;

    int file_index = 0;
    for(const auto &file : m_private->files)
    {
        if (file.rect.contains(event->localPos().toPoint()))
        {
            l_hoverFile = file_index;

            int hunk_index = 0;
            for(const auto &hunk : file.hunks)
            {
                if (hunk.rect.contains(event->localPos().toPoint()))
                {
                    l_hoverHunk = hunk_index;

                    if (event->x() >= 100)
                    {
                        int line_index = 0;
                        for(const auto &line : hunk.lines)
                        {
                            if (line.rect.contains(event->localPos().toPoint()))
                            {
                                if ((line.origin == '-')||(line.origin == '+'))
                                {
                                    l_hoverLine = line_index;
                                }
                                else
                                {
                                    l_hoverFile = -1;
                                    l_hoverHunk = -1;
                                    l_hoverLine = -1;
                                }

                                break;
                            }

                            line_index++;
                        }
                    }

                    break;
                }

                hunk_index++;
            }

            break;
        }

        file_index++;
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
