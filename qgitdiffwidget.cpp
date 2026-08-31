#include "qgitdiffwidget.h"

#include <QStyleOptionFocusRect>
#include <QStylePainter>
#include <QPaintEvent>
#include <QScrollBar>
#include <QObject>


// A single highlighted segment within a diff line (character or word range)
struct QGitDiffInlineRange
{
    int start = 0; // byte offset in the UTF-8 content
    int length = 0;
};

class QGitDiffWidgetPrivateLine
{
public:
    QByteArray content;
    git_off_t offset = 0;
    int new_lineno = 0;
    int old_lineno = 0;
    char origin = '\0';
    QRect rect;
    // Inline diff highlight ranges (in the displayed text, character positions)
    QVector<QGitDiffInlineRange> inlineRanges;
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

class QGitDiffWidgetPrivate : public QObject
{
public:
    explicit QGitDiffWidgetPrivate(QObject *parent = nullptr): QObject(parent)
    {

    }

    void clear()
    {
        files.clear();
        rect = QRect();
        maxLineNum = 1;
        lineNumWidth = 0;
        contentOffset = 0;
    }

    QVector<QGitDiffWidgetPrivateFile> files;
    QRect rect;
    int maxLineNum = 1;
    int lineNumWidth = 0;
    int contentOffset = 0;
};

QGitDiffWidget::QGitDiffWidget(QWidget *parent)
    : QWidget(parent)
{
    m_private = new QGitDiffWidgetPrivate(this);

    m_font = font();

    setMouseTracking(true);
}

void QGitDiffWidget::setGitDiff(const QString &first, const QString &second, const QList<QString> &files, const QMap<QString, git_status_t> &statuses)
{
    m_private->clear();

    m_requestedFirst = first;
    m_requestedSecond = second;
    m_requestedFiles = files;
    m_fileStatuses = statuses;

    emit requestGitDiff(first, second, files, m_linesOfContent, m_ignoreWhitespace);

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

void QGitDiffWidget::setIgnoreWhitespace(bool ignore)
{
    m_ignoreWhitespace = ignore;
}

void QGitDiffWidget::setLinesOfContent(int lines)
{
    m_linesOfContent = lines;
}

void QGitDiffWidget::setInlineDiffMode(InlineDiffMode mode)
{
    if (m_inlineDiffMode != mode)
    {
        m_inlineDiffMode = mode;
        // Re-compute inline diff on existing data
        if (!m_requestedFiles.isEmpty())
            refresh();
    }
}

void QGitDiffWidget::setShowWhitespaceChars(bool show)
{
    if (m_showWhitespaceChars != show)
    {
        m_showWhitespaceChars = show;
        update();
    }
}

// ---------------------------------------------------------------------------
// Whitespace visualisation helper
// ---------------------------------------------------------------------------

// Returns true if byteOffset falls inside any of the inline diff ranges.
static bool byteIsInInlineRange(int byteOffset, const QVector<QGitDiffInlineRange> &ranges)
{
    for (const auto &r : ranges)
        if (byteOffset >= r.start && byteOffset < r.start + r.length)
            return true;
    return false;
}

// Renders a line's text content to the painter at (x, yBaseline).
//
// Whitespace substitution rules:
//   - spaces  → '·'  (middle dot)
//   - tabs    → '→'  (rightward arrow)
//   - newline → '↵'  (return symbol, shown at end)
//
// A whitespace character is ALWAYS substituted when it falls inside an inline
// diff range (so empty-box artifacts on space↔tab changes are impossible),
// even when showWhitespaceChars is false.  When it falls outside any range,
// substitution only happens when showWhitespaceChars is true.
//
// Substitutes inside inline ranges use the full normalColor (clearly visible);
// substitutes outside ranges use a muted 45%-opacity colour.
static void drawLineContent(
    QPainter &painter,
    const QFont &font,
    const QByteArray &rawContent,
    int x,
    int yBaseline,
    bool showWhitespaceChars,
    const QColor &normalColor,
    const QVector<QGitDiffInlineRange> &inlineRanges)
{
    QFontMetrics fm(font);

    // Decode UTF-8, recording the byte offset of every character so we can
    // check it against the inline ranges (which are in byte coordinates).
    struct CharEntry {
        QChar ch;
        int   byteOffset;
    };

    QVector<CharEntry> chars;
    {
        const QByteArray &raw = rawContent;
        QString decoded = QString::fromUtf8(raw);
        int bOff = 0;
        for (const QChar &qch : decoded)
        {
            chars.append({qch, bOff});
            bOff += QString(qch).toUtf8().size();
        }
    }

    // Strip trailing CR/LF, recording the byte offset of the first stripped
    // byte so we can check whether the newline itself is in an inline range.
    bool hasTrailingNewline = false;
    int  newlineByteOffset  = rawContent.size(); // safe default: past the end
    while (!chars.isEmpty()
           && (chars.back().ch == u'\n' || chars.back().ch == u'\r'))
    {
        hasTrailingNewline  = true;
        newlineByteOffset   = chars.back().byteOffset;
        chars.pop_back();
    }

    QColor wsColor = normalColor;
    wsColor.setAlphaF(0.45f);   // muted — visible but clearly secondary

    int cx = x;
    int n  = chars.size();
    int i  = 0;

    while (i < n)
    {
        bool isWs   = (chars[i].ch == u' ' || chars[i].ch == u'\t');
        bool inRange = byteIsInInlineRange(chars[i].byteOffset, inlineRanges);

        // Collect a run of characters sharing the same (isWs, inRange) pair
        int start = i;
        while (i < n
               && (chars[i].ch == u' ' || chars[i].ch == u'\t') == isWs
               && byteIsInInlineRange(chars[i].byteOffset, inlineRanges) == inRange)
        {
            ++i;
        }

        // Build the original string (for width measurement) and the display
        // string (potentially with whitespace substitutes).
        bool substitute = isWs && (showWhitespaceChars || inRange);
        QString orig, display;
        for (int k = start; k < i; ++k)
        {
            orig += chars[k].ch;
            if (substitute)
                display += (chars[k].ch == u'\t') ? QChar(u'→') : QChar(u'·');
            else
                display += chars[k].ch;
        }

        int segWidth = fm.horizontalAdvance(orig);

        if (substitute)
        {
            // Substitutes inside an inline range: full colour (it's a real change).
            // Substitutes outside (global whitespace mode): muted colour.
            painter.setPen(inRange ? normalColor : wsColor);
            painter.drawText(cx, yBaseline, display);
        }
        else
        {
            painter.setPen(normalColor);
            painter.drawText(cx, yBaseline, display);
        }

        cx += segWidth;
    }

    // Trailing newline symbol — only shown when showWhitespaceChars is on or
    // the newline itself is inside an inline range.
    bool newlineInRange = byteIsInInlineRange(newlineByteOffset, inlineRanges);
    if (hasTrailingNewline && (showWhitespaceChars || newlineInRange))
    {
        painter.setPen(newlineInRange ? normalColor : wsColor);
        painter.drawText(cx, yBaseline, QStringLiteral("↵"));
    }
}


// ---------------------------------------------------------------------------
// Inline diff helpers
// ---------------------------------------------------------------------------

// Split a UTF-8 byte array into tokens.
// In CharacterLevel mode, each token is a single Unicode character (QChar).
// In WordLevel mode, tokens are whitespace-separated words, with whitespace
// runs kept as their own tokens so positions map back to source indices.
static QVector<QByteArray> splitTokens(const QByteArray &text, bool wordLevel)
{
    QVector<QByteArray> tokens;
    if (wordLevel)
    {
        int i = 0;
        while (i < text.size())
        {
            bool ws = (text[i] == ' ' || text[i] == '\t');
            int start = i;
            while (i < text.size() && (text[i] == ' ' || text[i] == '\t') == ws)
                ++i;
            tokens.append(text.mid(start, i - start));
        }
    }
    else
    {
        // Character level: split by Unicode code points
        QString str = QString::fromUtf8(text);
        for (const QChar &ch : str)
        {
            tokens.append(QString(ch).toUtf8());
        }
    }
    return tokens;
}

// Compute LCS length table and back-trace the diff.
// Returns ranges in `b` that are *changed* (not in common with `a`).
static QVector<QGitDiffInlineRange> computeChangedRanges(const QByteArray &a, const QByteArray &b, bool wordLevel)
{
    QVector<QByteArray> ta = splitTokens(a, wordLevel);
    QVector<QByteArray> tb = splitTokens(b, wordLevel);

    int m = ta.size();
    int n = tb.size();

    // Limit to prevent O(m*n) blowup on huge lines
    if (m > 200 || n > 200)
    {
        // Highlight entire line as changed
        QGitDiffInlineRange r;
        r.start = 0;
        r.length = b.size();
        return {r};
    }

    // Build LCS table
    QVector<QVector<int>> dp(m + 1, QVector<int>(n + 1, 0));
    for (int i = 1; i <= m; ++i)
        for (int j = 1; j <= n; ++j)
            dp[i][j] = (ta[i-1] == tb[j-1]) ? dp[i-1][j-1] + 1
                                              : qMax(dp[i-1][j], dp[i][j-1]);

    // Back-trace to find which tokens in b are changed (i.e., not matched)
    QVector<bool> matched(n, false);
    int i = m, j = n;
    while (i > 0 && j > 0)
    {
        if (ta[i-1] == tb[j-1]) { matched[j-1] = true; --i; --j; }
        else if (dp[i-1][j] >= dp[i][j-1]) --i;
        else --j;
    }

    // Convert token indices to byte offsets in b
    QVector<QGitDiffInlineRange> ranges;
    int bytePos = 0;
    for (int k = 0; k < n; ++k)
    {
        int tokenLen = tb[k].size();
        if (!matched[k])
        {
            // Merge with previous range if adjacent
            if (!ranges.isEmpty() && ranges.last().start + ranges.last().length == bytePos)
                ranges.last().length += tokenLen;
            else
            {
                QGitDiffInlineRange r;
                r.start = bytePos;
                r.length = tokenLen;
                ranges.append(r);
            }
        }
        bytePos += tokenLen;
    }
    return ranges;
}

void QGitDiffWidget::refresh()
{
    if (!m_requestedFiles.isEmpty())
    {
        emit requestGitDiff(m_requestedFirst, m_requestedSecond, m_requestedFiles, m_linesOfContent, m_ignoreWhitespace);
    }
}

bool QGitDiffWidget::readonly() const
{
    return m_readonly;
}

int QGitDiffWidget::hoverFile() const
{
    return m_hoverFile;
}

int QGitDiffWidget::hoverHunk() const
{
    return m_hoverHunk;
}

int QGitDiffWidget::hoverLine() const
{
    return m_hoverLine;
}

QVector<QGitDiffWidgetLine> QGitDiffWidget::linesAt(int fileIdx, int hunkIdx, int lineIdx, QString &fileName) const
{
    QVector<QGitDiffWidgetLine> lines;

    if (fileIdx < 0 || fileIdx >= m_private->files.count()) return lines;
    const auto &file = m_private->files.at(fileIdx);
    fileName = file.new_file.path();

    if (hunkIdx < 0 || hunkIdx >= file.hunks.count()) return lines;
    const auto &hunk = file.hunks.at(hunkIdx);
    if (hunk.lines.isEmpty()) return lines;

    if (lineIdx < 0) {
        // Collect whole hunk
        int v_old = hunk.lines.first().old_lineno - 1;
        int v_new = hunk.lines.first().new_lineno - 1;
        if (v_old < 0) v_old = hunk.old_start - 1;
        if (v_new < 0) v_new = hunk.new_start - 1;

        for (const auto &line : hunk.lines) {
            QGitDiffWidgetLine newLine;
            newLine.content = line.content;
            newLine.hunk_new_start = hunk.new_start;
            newLine.hunk_old_start = hunk.old_start;
            newLine.origin = line.origin;

            if (line.origin == ' ') {
                v_old = line.old_lineno;
                v_new = line.new_lineno;
                newLine.old_lineno = v_old;
                newLine.new_lineno = v_new;
            } else if (line.origin == '-') {
                v_old = line.old_lineno;
                newLine.old_lineno = v_old;
                newLine.new_lineno = v_new + 1;
            } else if (line.origin == '+') {
                v_new = line.new_lineno;
                newLine.new_lineno = v_new;
                newLine.old_lineno = v_old + 1;
            }

            lines.push_back(newLine);
        }
    } else {
        // Collect single line
        if (lineIdx >= hunk.lines.count()) return lines;
        
        int v_old = hunk.lines.first().old_lineno - 1;
        int v_new = hunk.lines.first().new_lineno - 1;
        if (v_old < 0) v_old = hunk.old_start - 1;
        if (v_new < 0) v_new = hunk.new_start - 1;
        
        int target_v_old = 0, target_v_new = 0;
        for (int c = 0; c <= lineIdx; c++) {
            const auto &line = hunk.lines.at(c);
            if (line.origin == ' ') {
                v_old = line.old_lineno;
                v_new = line.new_lineno;
                target_v_old = v_old;
                target_v_new = v_new;
            } else if (line.origin == '-') {
                v_old = line.old_lineno;
                target_v_old = v_old;
                target_v_new = v_new + 1;
            } else if (line.origin == '+') {
                v_new = line.new_lineno;
                target_v_old = v_old + 1;
                target_v_new = v_new;
            }
        }
        
        const auto &line = hunk.lines.at(lineIdx);
        if (line.origin == '+' || line.origin == '-') {
            QGitDiffWidgetLine newLine;
            newLine.content = line.content;
            newLine.new_lineno = target_v_new;
            newLine.old_lineno = target_v_old;
            newLine.hunk_new_start = hunk.new_start;
            newLine.hunk_old_start = hunk.old_start;
            newLine.origin = line.origin;
            lines.push_back(newLine);
        }
    }
    return lines;
}

void QGitDiffWidget::responseGitDiff(const QString &first, const QString &second, const QList<QGitDiffFile> &diff, const QGitError &error)
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

    // Dynamically calculate line number gutter width & content offset
    int maxLineNum = 1;
    for (const QGitDiffFile &diffFile : diff) {
        for (const QGitDiffHunk &diffHunk : diffFile.hunks()) {
            for (const QGitDiffLine &diffLine : diffHunk.lines()) {
                maxLineNum = qMax(maxLineNum, qMax(diffLine.old_lineno(), diffLine.new_lineno()));
            }
        }
    }
    m_private->maxLineNum = maxLineNum;
    m_private->lineNumWidth = fm.horizontalAdvance(QString(QString::number(maxLineNum).length(), '9')) + 10;
    m_private->contentOffset = m_private->lineNumWidth * 2 + 20;

    const int contentOffset = m_private->contentOffset;

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
        file.rect.setWidth(contentOffset + lineMax);

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
            hunk.rect.setWidth(contentOffset + lineMax);

            for(int pos_line = 0; pos_line < hunk_item.lines().count(); pos_line++)
            {
                const auto &line_item = hunk_item.lines().at(pos_line);
                QGitDiffWidgetPrivateLine line;

                line.content = line_item.content();
                line.offset = line_item.offset();
                line.new_lineno = line_item.new_lineno();
                line.old_lineno = line_item.old_lineno();
                line.origin = line_item.origin();

                line.rect.setTop(y + file_h + hunk_h);
                line.rect.setLeft(contentsMargins().left());
                line.rect.setWidth(contentOffset + lineMax);

                hunk_h += m_fontHeight + 1;

                line.rect.setHeight(m_fontHeight + 1);

                hunk.lines.push_back(line);
            }

            hunk.rect.setHeight(hunk_h);
            file_h += hunk_h;

            // ---------------------------------------------------------------
            // Inline diff: pair adjacent deleted/added lines and compute
            // character- or word-level diff ranges.
            // ---------------------------------------------------------------
            if (m_inlineDiffMode != InlineDiffMode::Off)
            {
                bool wordLevel = (m_inlineDiffMode == InlineDiffMode::WordLevel);
                int nLines = hunk.lines.size();
                int li = 0;
                while (li < nLines)
                {
                    // Collect a run of '-' lines followed immediately by '+' lines
                    int delStart = li;
                    while (li < nLines && hunk.lines[li].origin == '-') ++li;
                    int delEnd = li; // exclusive

                    int addStart = li;
                    while (li < nLines && hunk.lines[li].origin == '+') ++li;
                    int addEnd = li; // exclusive

                    int delCount = delEnd - delStart;
                    int addCount = addEnd - addStart;

                    if (delCount > 0 && addCount > 0)
                    {
                        // Pair them up 1-to-1; remaining unpaired lines get full-line highlight
                        int pairs = qMin(delCount, addCount);
                        for (int p = 0; p < pairs; ++p)
                        {
                            auto &delLine = hunk.lines[delStart + p];
                            auto &addLine = hunk.lines[addStart + p];

                            // Strip trailing newline for comparison
                            QByteArray delContent = delLine.content;
                            QByteArray addContent = addLine.content;
                            while (!delContent.isEmpty() && (delContent.back() == '\n' || delContent.back() == '\r'))
                                delContent.chop(1);
                            while (!addContent.isEmpty() && (addContent.back() == '\n' || addContent.back() == '\r'))
                                addContent.chop(1);

                            delLine.inlineRanges = computeChangedRanges(addContent, delContent, wordLevel);
                            addLine.inlineRanges = computeChangedRanges(delContent, addContent, wordLevel);
                        }
                    }

                    if (delEnd == addEnd && delEnd == li) ++li; // context line, skip
                }
            }

            file.hunks.push_back(hunk);
        }

        file.rect.setHeight(file_h);

        y += file_h;

        m_private->files.push_back(file);
    }

    newSize = QSize(contentsMargins().left() + contentOffset + lineMax + contentsMargins().right(), contentsMargins().top() + y + contentsMargins().bottom());

    setMinimumSize(newSize);
    setMaximumSize(newSize);

    update();
}

void QGitDiffWidget::paintEvent(QPaintEvent *event)
{
    QStylePainter painter(this);
    int fileIndex = 0, hunkIndex = 0, lineIndex = 0;

    const auto &files = m_private->files;
    const int lineNumWidth = m_private->lineNumWidth;

    for(const auto &file: files)
    {
        if (!event->region().intersected(file.rect).isEmpty())
        {
            painter.fillRect(file.rect, QColor(220,220,220));

            if (m_hoverLine == -1)
            {
                hunkIndex = 0;
                for(const auto &hunk: file.hunks)
                {
                    if ((fileIndex == m_hoverFile)&&(hunkIndex == m_hoverHunk))
                    {
                        QStyleOptionFocusRect option;
                        option.initFrom(this);
                        option.rect = hunk.rect.adjusted(0, 0, 0, -1);
                        painter.setBrush(Qt::NoBrush);
                        painter.fillRect(option.rect, QColor(192,192,192));
                    }

                    hunkIndex++;
                }
            }

            painter.setPen(Qt::black);
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

                            int oldColX = 10;
                            int newColX = 10 + lineNumWidth + 5;
                            int contentX = 10 + (lineNumWidth + 5) * 2;

                            painter.drawText(oldColX, yFont, old_lineNo);
                            painter.drawText(newColX, yFont, new_lineNo);

                            // -----------------------------------------------
                            // Inline diff highlighting
                            // -----------------------------------------------
                            if (!line.inlineRanges.isEmpty())
                            {
                                QFontMetrics fm(m_font);
                                QString fullText = QString::fromUtf8(line.content);

                                // Choose highlight colour: deeper shade of line background
                                QColor hlColor = (line.origin == '-')
                                    ? QColor(200, 100, 100)   // deeper red for deleted parts
                                    : QColor(100, 185, 80);   // deeper green for added parts

                                for (const auto &range : line.inlineRanges)
                                {
                                    // Convert byte offsets to QString character positions
                                    // because the content might be multi-byte UTF-8
                                    QByteArray rawContent = line.content;
                                    int charStart = QString::fromUtf8(rawContent.left(range.start)).length();
                                    int charLength = QString::fromUtf8(rawContent.mid(range.start, range.length)).length();

                                    if (charLength <= 0) continue;

                                    // Pixel position of the range start/end inside the content area
                                    int xRangeStart = contentX + fm.horizontalAdvance(fullText, charStart);
                                    int xRangeEnd   = contentX + fm.horizontalAdvance(fullText, charStart + charLength);

                                    painter.setPen(Qt::NoPen);
                                    painter.setBrush(QBrush(hlColor));
                                    painter.drawRect(QRect(xRangeStart, line.rect.top(),
                                                           xRangeEnd - xRangeStart, line.rect.height()));
                                }

                                // Restore pen color for text
                                painter.setPen((line.origin == '-') ? Qt::darkRed : Qt::darkGreen);
                            }

                            // Determine normal text colour for this line type
                            QColor normalColor;
                            if (line.origin == '-')      normalColor = Qt::darkRed;
                            else if (line.origin == '+') normalColor = Qt::darkGreen;
                            else                         normalColor = Qt::black;

                            drawLineContent(painter, m_font, line.content,
                                            contentX, yFont,
                                            m_showWhitespaceChars, normalColor,
                                            line.inlineRanges);

                            if ((fileIndex == m_hoverFile)&&(hunkIndex == m_hoverHunk)&&(lineIndex == m_hoverLine))
                            {
                                QStyleOptionFocusRect option;
                                option.initFrom(this);
                                option.rect = line.rect.adjusted(contentX - contentsMargins().left(), 0, -1, -1);
                                painter.setPen(Qt::SolidLine);
                                painter.setBrush(Qt::NoBrush);
                                painter.drawRect(option.rect);
                                //painter.drawPrimitive(QStyle::PE_FrameFocusRect, option);
                            }

                        }

                        lineIndex++;
                    }

                    if ((fileIndex == m_hoverFile)&&(hunkIndex == m_hoverHunk)&&(m_hoverLine == -1))
                    {
                        QStyleOptionFocusRect option;
                        option.initFrom(this);
                        option.rect = hunk.rect.adjusted(0, 0, -1, -1);
                        painter.setPen(Qt::SolidLine);
                        painter.setBrush(Qt::NoBrush);
                        painter.drawRect(option.rect);
                        //painter.drawPrimitive(QStyle::PE_FrameFocusRect, option);
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
    if ((m_readonly)||(event->buttons() != Qt::LeftButton))
    {
        return;
    }

    QString fileName;
    QVector<QGitDiffWidgetLine> lines = linesAt(m_hoverFile, m_hoverHunk, m_hoverLine, fileName);

    if (!lines.isEmpty())
    {
        emit select(fileName, lines);
    }
}

void QGitDiffWidget::mouseMoveEvent(QMouseEvent *event)
{
    QWidget::mouseMoveEvent(event);

    updatePosition();
}

void QGitDiffWidget::moveEvent(QMoveEvent *event)
{
    QWidget::moveEvent(event);
    updatePosition();
}

void QGitDiffWidget::updatePosition()
{
    int l_hoverFile = -1;
    int l_hoverHunk = -1;
    int l_hoverLine = -1;

    if (m_readonly)
        return;

    QPoint point = this->mapFromGlobal(QCursor::pos());

    int file_index = 0;
    const auto &files = m_private->files;
    const int contentX = 10 + (m_private->lineNumWidth + 5) * 2;

    for(const auto &file : files)
    {
        if (file.rect.contains(point))
        {
            QString filePath = file.new_file.path();
            if (filePath.isEmpty()) {
                filePath = file.old_file.path();
            }

            if (m_fileStatuses.contains(filePath))
            {
                git_status_t status = m_fileStatuses.value(filePath);
                if (status & (GIT_STATUS_WT_NEW | GIT_STATUS_INDEX_NEW | GIT_STATUS_WT_DELETED | GIT_STATUS_INDEX_DELETED))
                {
                    break;
                }
            }

            l_hoverFile = file_index;

            int hunk_index = 0;
            for(const auto &hunk : file.hunks)
            {
                if (hunk.rect.contains(point))
                {
                    l_hoverHunk = hunk_index;

                    if (point.x() >= contentX)
                    {
                        int line_index = 0;
                        for(const auto &line : hunk.lines)
                        {
                            if (line.rect.contains(point))
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
