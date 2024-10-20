#include "qgitdiffwidget.h"

#include <QStylePainter>
#include <QPaintEvent>
#include <QScrollBar>
#include <QObject>


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
    explicit QGitDiffWidgetPrivate(QObject *parent = nullptr): QObject(parent)
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

    m_requestedFirst = first;
    m_requestedSecond = second;
    m_requestedFiles = files;

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

void QGitDiffWidget::refresh()
{
    if (!m_requestedFiles.isEmpty())
    {
        emit requestGitDiff(m_requestedFirst, m_requestedSecond, m_requestedFiles);
    }
}

bool QGitDiffWidget::readonly() const
{
    return m_readonly;
}

void QGitDiffWidget::responseGitDiff(QString first, QString second, QList<QGitDiffFile> diff, QGitError error)
{
    int y = 0, file_h = 0, lineMax = 0;
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
        const auto &hunks = diffFile.hunks();
        for(const QGitDiffHunk &diffHunk: hunks)
        {
            const auto &lines = diffHunk.lines();
            for(const QGitDiffLine &diffLine: lines)
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

        y += contentsMargins().top();

        file.new_file = file_item.new_file();
        file.old_file = file_item.old_file();
        file.flags = file_item.flags();
        file.nfiles = file_item.nfiles();
        file.simularity = file_item.simularity();
        file.status = file_item.status();

        file.rect.setTop(y);
        file.rect.setLeft(contentsMargins().left());
        file.rect.setWidth(100 + lineMax); // TODO: Unhardcode 100

        file_h = m_fontHeight * 2;

        for(int pos_hunk = 0; pos_hunk < file_item.hunks().count(); pos_hunk++)
        {
            const auto &hunk_item = file_item.hunks().at(pos_hunk);
            QGitDiffWidgetPrivateHunk hunk;
            int hunk_h = 0;

            hunk.header = hunk_item.header();
            hunk.new_lines = hunk_item.new_lines();
            hunk.new_start = hunk_item.new_start();
            hunk.old_lines = hunk_item.old_lines();
            hunk.old_start = hunk_item.old_start();

            if (pos_hunk > 0)
                file_h += m_fontHeight + 1;

            hunk.rect.setTop(y + file_h);
            hunk.rect.setLeft(contentsMargins().left());
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

                line.rect.setTop(y + file_h + hunk_h);
                line.rect.setLeft(contentsMargins().left());
                line.rect.setWidth(100 + lineMax); // TODO: Unhardcode 100

                hunk_h += m_fontHeight + 1;

                line.rect.setHeight(m_fontHeight + 1);

                hunk.lines.push_back(line);
            }

            hunk.rect.setHeight(hunk_h);
            file_h += hunk_h;

            file.hunks.push_back(hunk);
        }

        file.rect.setHeight(file_h);

        y += file_h;

        m_private->files.push_back(file);
    }

    newSize = QSize(contentsMargins().left() + 100 + lineMax + contentsMargins().right(), contentsMargins().top() + y + contentsMargins().bottom()); // TODO: Unhardcode 100

    setMinimumSize(newSize);
    setMaximumSize(newSize);

    update();
}

void QGitDiffWidget::paintEvent(QPaintEvent *event)
{
    QStylePainter painter(this);
    int fileIndex = 0, hunkIndex = 0, lineIndex = 0;

    fileIndex = 0;

    const auto &files = m_private->files;
    for(const auto &file: files)
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

                            painter.drawText(10, yFont, old_lineNo); // TODO: Unhardcode 10
                            painter.drawText(40, yFont, new_lineNo); // TODO: Unhardcode 40
                            painter.drawText(100, yFont, line.content); // TODO: Unhardcode 100

                            if ((fileIndex == m_hoverFile)&&(hunkIndex == m_hoverHunk)&&(lineIndex == m_hoverLine))
                            {
                                QStyleOptionFocusRect option;
                                option.initFrom(this);
                                option.rect = line.rect.adjusted(100 - contentsMargins().left(), 0, 0, 0);
                                painter.setBrush(Qt::NoBrush);
                                painter.drawPrimitive(QStyle::PE_FrameFocusRect, option);
                            }
                        }

                        lineIndex++;
                    }

                    if ((fileIndex == m_hoverFile)&&(hunkIndex == m_hoverHunk)&&(m_hoverLine == -1))
                    {
                        QStyleOptionFocusRect option;
                        option.initFrom(this);
                        option.rect = hunk.rect;
                        painter.setBrush(Qt::NoBrush);
                        painter.drawPrimitive(QStyle::PE_FrameFocusRect, option);
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

    if ((!m_readonly)&&(m_hoverFile >= 0)&&(m_hoverHunk >= 0))
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
    const auto &files = m_private->files;

    for(const auto &file : files)
    {
        if (file.rect.contains(event->position().toPoint()))
        {
            l_hoverFile = file_index;

            int hunk_index = 0;
            for(const auto &hunk : file.hunks)
            {
                if (hunk.rect.contains(event->position().toPoint()))
                {
                    l_hoverHunk = hunk_index;

                    if (event->position().x() >= 100) // TODO Unhardcode magic number 100
                    {
                        int line_index = 0;
                        for(const auto &line : hunk.lines)
                        {
                            if (line.rect.contains(event->position().toPoint()))
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
