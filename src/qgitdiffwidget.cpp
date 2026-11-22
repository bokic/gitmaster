// SPDX-License-Identifier: LGPL-3.0-or-later
#include "qgitdiffwidget.h"

#include <QStyleOptionFocusRect>
#include <QStylePainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QScrollBar>
#include <QFontDatabase>
#include <QPainterPath>
#include <QObject>
#include <algorithm>


namespace {

QColor blendColor(const QColor &base, const QColor &tint, qreal ratio)
{
    const qreal inv = 1.0 - ratio;
    return QColor::fromRgbF(
        base.redF()   * inv + tint.redF()   * ratio,
        base.greenF() * inv + tint.greenF() * ratio,
        base.blueF()  * inv + tint.blueF()  * ratio
    );
}

struct DiffThemeColors
{
    bool isDark = false;

    QColor widgetBg;
    QColor fileCardBorder;
    QColor fileHeaderBg;
    QColor fileHeaderText;
    QColor fileHeaderDirText;

    QColor hunkHeaderBg;
    QColor hunkHeaderText;
    QColor hunkHeaderBorder;

    QColor contextBg;
    QColor contextGutterBg;
    QColor contextText;
    QColor lineNumText;
    QColor gutterBorder;

    QColor addedBg;
    QColor addedGutterBg;
    QColor addedInlineBg;
    QColor addedSignText;
    QColor addedLineNumText;

    QColor deletedBg;
    QColor deletedGutterBg;
    QColor deletedInlineBg;
    QColor deletedSignText;
    QColor deletedLineNumText;

    QColor emptySlotBg;
    QColor emptySlotHatch;
    QColor dividerColor;

    QColor hoverFill;
    QColor hoverBorder;
    QColor hoverAccentBar;
};

DiffThemeColors resolveThemeColors(const QPalette &pal)
{
    DiffThemeColors c;
    const QColor base = pal.color(QPalette::Base);
    const QColor text = pal.color(QPalette::Text);
    const QColor highlight = pal.color(QPalette::Highlight);
    c.isDark = base.lightnessF() < 0.5;

    const QColor addAccent = c.isDark ? QColor(46, 160, 67)  : QColor(45, 164, 78);
    const QColor delAccent = c.isDark ? QColor(248, 81, 73)  : QColor(207, 34, 46);
    const QColor hunkAccent = c.isDark ? QColor(56, 139, 253) : QColor(9, 105, 218);

    c.widgetBg = base;
    c.fileCardBorder = blendColor(base, text, c.isDark ? 0.16 : 0.14);
    c.fileHeaderBg = blendColor(base, text, c.isDark ? 0.07 : 0.045);
    c.fileHeaderText = text;
    c.fileHeaderDirText = blendColor(base, text, 0.55);

    c.hunkHeaderBg = blendColor(base, hunkAccent, c.isDark ? 0.14 : 0.09);
    c.hunkHeaderText = c.isDark ? QColor(121, 192, 255) : QColor(9, 105, 218);
    c.hunkHeaderBorder = blendColor(base, hunkAccent, c.isDark ? 0.24 : 0.18);

    c.contextBg = base;
    c.contextGutterBg = blendColor(base, text, c.isDark ? 0.035 : 0.025);
    c.contextText = text;
    c.lineNumText = blendColor(base, text, c.isDark ? 0.42 : 0.45);
    c.gutterBorder = blendColor(base, text, c.isDark ? 0.12 : 0.10);

    c.addedBg = blendColor(base, addAccent, c.isDark ? 0.15 : 0.11);
    c.addedGutterBg = blendColor(base, addAccent, c.isDark ? 0.24 : 0.19);
    c.addedInlineBg = blendColor(base, addAccent, c.isDark ? 0.38 : 0.30);
    c.addedSignText = c.isDark ? QColor(63, 185, 80) : QColor(26, 127, 55);
    c.addedLineNumText = blendColor(c.addedGutterBg, text, 0.65);

    c.deletedBg = blendColor(base, delAccent, c.isDark ? 0.16 : 0.11);
    c.deletedGutterBg = blendColor(base, delAccent, c.isDark ? 0.25 : 0.19);
    c.deletedInlineBg = blendColor(base, delAccent, c.isDark ? 0.40 : 0.30);
    c.deletedSignText = c.isDark ? QColor(248, 81, 73) : QColor(207, 34, 46);
    c.deletedLineNumText = blendColor(c.deletedGutterBg, text, 0.65);

    c.emptySlotBg = blendColor(base, text, c.isDark ? 0.045 : 0.035);
    c.emptySlotHatch = blendColor(base, text, c.isDark ? 0.09 : 0.07);
    c.dividerColor = blendColor(base, text, c.isDark ? 0.18 : 0.15);

    c.hoverFill = highlight;
    c.hoverFill.setAlpha(c.isDark ? 45 : 30);
    c.hoverBorder = highlight;
    c.hoverBorder.setAlpha(170);
    c.hoverAccentBar = highlight;

    return c;
}

void drawEmptySlotHatch(QPainter &painter, const QRect &rect, const DiffThemeColors &colors)
{
    painter.fillRect(rect, colors.emptySlotBg);
    painter.save();
    painter.setClipRect(rect);
    painter.setPen(QPen(colors.emptySlotHatch, 1));
    const int step = 8;
    const int h = rect.height();
    for (int x = rect.left() - h; x < rect.right() + h; x += step) {
        painter.drawLine(x, rect.bottom(), x + h, rect.top());
    }
    painter.restore();
}

void drawHoverOverlay(QPainter &painter, const QRect &rect, const DiffThemeColors &colors)
{
    painter.fillRect(rect, colors.hoverFill);
    painter.fillRect(QRect(rect.left(), rect.top(), 3, rect.height()), colors.hoverAccentBar);
    painter.setPen(QPen(colors.hoverBorder, 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(rect.adjusted(0, 0, -1, -1));
}

} // namespace


// A single highlighted segment within a diff line (character or word range)
struct QGitDiffInlineRange
{
    int start = 0; // byte offset in the UTF-8 content
    int length = 0;
    int charStart = 0;
    int charLength = 0;
};

class QGitDiffWidgetPrivateLine
{
public:
    struct CharEntry {
        CharEntry() = default;
        CharEntry(char16_t c, int off) : ch(c), byteOffset(off) {}
        char16_t ch = 0;
        int   byteOffset = 0;
    };

    QByteArray content;
    git_off_t offset = 0;
    int new_lineno = 0;
    int old_lineno = 0;
    char origin = '\0';
    QRect rect;
    // Inline diff highlight ranges (in the displayed text, character positions)
    QVector<QGitDiffInlineRange> inlineRanges;

    // Precomputed decoded characters and mappings
    QString displayText;
    QVector<CharEntry> chars;
    bool hasTrailingNewline = false;
    int newlineByteOffset = 0;
};

// A single row in side-by-side diff view (pairing old/left and new/right)
class QGitDiffWidgetPrivateSideRow
{
public:
    int oldLineIdx = -1; // index into hunk.lines, or -1 if empty/inserted
    int newLineIdx = -1; // index into hunk.lines, or -1 if empty/deleted
    QRect rect;
    QRect leftRect;
    QRect rightRect;
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
    QVector<QGitDiffWidgetPrivateSideRow> sideRows;
    QRect headerRect;
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
    int addedCount = 0;
    int deletedCount = 0;

    QVector<QGitDiffWidgetPrivateHunk> hunks;
    QRect headerRect;
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
        signWidth = 0;
        contentOffset = 0;
        sidePanelWidth = 0;
        minContentWidth = 0;
    }

    QVector<QGitDiffWidgetPrivateFile> files;
    QRect rect;
    int maxLineNum = 1;
    int lineNumWidth = 0;
    int signWidth = 0;
    int contentOffset = 0;
    int sidePanelWidth = 0;
    int minContentWidth = 0;
};

QGitDiffWidget::QGitDiffWidget(QWidget *parent)
    : QWidget(parent)
{
    m_private = new QGitDiffWidgetPrivate(this);

    initFonts();
    setMouseTracking(true);
}

void QGitDiffWidget::initFonts()
{
    m_uiFont = font();
    m_font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    if (m_uiFont.pointSize() > 0) {
        m_font.setPointSize(m_uiFont.pointSize());
    } else if (m_uiFont.pixelSize() > 0) {
        m_font.setPixelSize(m_uiFont.pixelSize());
    }
}

void QGitDiffWidget::setGitDiff(const QString &first, const QString &second, const QStringList &files, const QMap<QString, git_status_t> &statuses)
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

void QGitDiffWidget::setDiffViewMode(DiffViewMode mode)
{
    if (m_diffViewMode != mode)
    {
        m_diffViewMode = mode;
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
    const QFontMetrics &fm,
    const QGitDiffWidgetPrivateLine &line,
    int x,
    int yBaseline,
    bool showWhitespaceChars,
    const QColor &normalColor)
{
    const auto &chars = line.chars;
    if (chars.isEmpty() && !line.hasTrailingNewline)
        return;

    QColor wsColor = normalColor;
    wsColor.setAlphaF(0.45f);   // muted — visible but clearly secondary

    int cx = x;
    qsizetype n  = chars.size();
    qsizetype i  = 0;

    while (i < n)
    {
        bool isWs   = (chars[i].ch == u' ' || chars[i].ch == u'\t');
        bool inRange = byteIsInInlineRange(chars[i].byteOffset, line.inlineRanges);

        // Collect a run of characters sharing the same (isWs, inRange) pair
        qsizetype start = i;
        while (i < n
               && (chars[i].ch == u' ' || chars[i].ch == u'\t') == isWs
               && byteIsInInlineRange(chars[i].byteOffset, line.inlineRanges) == inRange)
        {
            ++i;
        }

        // Build the display string (potentially with whitespace substitutes).
        bool substitute = isWs && (showWhitespaceChars || inRange);
        QString display;
        display.reserve(i - start);
        for (qsizetype k = start; k < i; ++k)
        {
            if (substitute)
                display += (chars[k].ch == u'\t') ? QChar(u'→') : QChar(u'·');
            else
                display += chars[k].ch;
        }

        int segWidth = fm.horizontalAdvance(display);

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
    bool newlineInRange = byteIsInInlineRange(line.newlineByteOffset, line.inlineRanges);
    if (line.hasTrailingNewline && (showWhitespaceChars || newlineInRange))
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

    int m = static_cast<int>(ta.size());
    int n = static_cast<int>(tb.size());

    // Limit to prevent O(m*n) blowup on huge lines
    if (m > 200 || n > 200)
    {
        // Highlight entire line as changed
        QGitDiffInlineRange r;
        r.start = 0;
        r.length = static_cast<int>(b.size());
        return {r};
    }

    // Build LCS table
    QVector<QVector<int>> dp(m + 1, QVector<int>(n + 1, 0));
    for (int i = 1; i <= m; ++i)
        for (int j = 1; j <= n; ++j)
            dp[i][j] = (ta[i-1] == tb[j-1]) ? dp[i-1][j-1] + 1
                                              : std::max(dp[i-1][j], dp[i][j-1]);

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
        int tokenLen = static_cast<int>(tb[k].size());
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

    if (fileIdx < 0 || fileIdx >= m_private->files.size()) return lines;
    const auto &file = m_private->files.at(fileIdx);
    fileName = file.new_file.path();
    if (fileName.isEmpty()) fileName = file.old_file.path();

    if (hunkIdx < 0 || hunkIdx >= file.hunks.size()) return lines;
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
        if (lineIdx >= hunk.lines.size()) return lines;
        
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
                maxLineNum = std::max({maxLineNum, diffLine.old_lineno(), diffLine.new_lineno()});
            }
        }
    }
    const QFontMetrics uiFm(m_uiFont);
    const int lineHeight = m_fontHeight + 6;
    const int fileHeaderHeight = std::max(uiFm.height() + 18, m_fontHeight + 14);
    const int hunkHeaderHeight = m_fontHeight + 8;
    const int fileGap = 12;

    m_private->maxLineNum = maxLineNum;
    m_private->lineNumWidth = fm.horizontalAdvance(QString(QString::number(maxLineNum).size(), '9')) + 14;
    m_private->signWidth = fm.horizontalAdvance(QStringLiteral("+")) + 10;
    m_private->contentOffset = m_private->lineNumWidth * 2 + 12 + m_private->signWidth + 6;

    const int contentOffset = m_private->contentOffset;

    // For side-by-side view: calculate panel width for each side
    int sidePanelWidth = m_private->lineNumWidth + 10 + m_private->signWidth + lineMax + 24;
    if (sidePanelWidth < 320) sidePanelWidth = 320;
    m_private->sidePanelWidth = sidePanelWidth;

    const bool isSideBySide = (m_diffViewMode == DiffViewMode::SideBySide);
    const int totalDiffWidth = isSideBySide ? (sidePanelWidth * 2 + 1) : (contentOffset + lineMax + 24);
    m_private->minContentWidth = totalDiffWidth;

    const int leftMargin = contentsMargins().left();

    for(int c = 0; c < diff.size(); c++)
    {
        const auto &file_item = diff.at(c);
        QGitDiffWidgetPrivateFile file;

        if (c == 0)
            y += contentsMargins().top();
        else
            y += fileGap;

        file.new_file = file_item.new_file();
        file.old_file = file_item.old_file();
        file.flags = file_item.flags();
        file.nfiles = file_item.nfiles();
        file.simularity = file_item.simularity();
        file.status = file_item.status();

        file.rect.setTop(y);
        file.rect.setLeft(leftMargin);
        file.rect.setWidth(totalDiffWidth);

        file.headerRect = QRect(leftMargin, y, totalDiffWidth, fileHeaderHeight);
        file_h = fileHeaderHeight;

        for(int pos_hunk = 0; pos_hunk < file_item.hunks().size(); pos_hunk++)
        {
            const auto &hunk_item = file_item.hunks().at(pos_hunk);
            QGitDiffWidgetPrivateHunk hunk;
            int hunk_h = 0;

            hunk.header = hunk_item.header();
            hunk.new_lines = hunk_item.new_lines();
            hunk.new_start = hunk_item.new_start();
            hunk.old_lines = hunk_item.old_lines();
            hunk.old_start = hunk_item.old_start();

            hunk.rect.setTop(y + file_h);
            hunk.rect.setLeft(leftMargin);
            hunk.rect.setWidth(totalDiffWidth);

            hunk.headerRect = QRect(leftMargin, y + file_h, totalDiffWidth, hunkHeaderHeight);
            hunk_h += hunkHeaderHeight;

            for(int pos_line = 0; pos_line < hunk_item.lines().size(); pos_line++)
            {
                const auto &line_item = hunk_item.lines().at(pos_line);
                QGitDiffWidgetPrivateLine line;

                line.content = line_item.content();
                line.offset = line_item.offset();
                line.new_lineno = line_item.new_lineno();
                line.old_lineno = line_item.old_lineno();
                line.origin = line_item.origin();

                if (line.origin == '+')
                    file.addedCount++;
                else if (line.origin == '-')
                    file.deletedCount++;

                // Precompute decoded characters and character-to-byte mapping
                QString decoded = QString::fromUtf8(line.content);
                line.chars.reserve(decoded.size());
                int bOff = 0;
                for (int i = 0; i < decoded.size(); ++i)
                {
                    auto qch = decoded.at(i);
                    line.chars.append({qch.unicode(), bOff});
                    if (qch.isHighSurrogate() && i + 1 < decoded.size() && decoded.at(i + 1).isLowSurrogate()) {
                        line.chars.append({decoded.at(i + 1).unicode(), bOff});
                        bOff += 4;
                        ++i;
                    } else {
                        ushort u = qch.unicode();
                        if (u < 0x80) bOff += 1;
                        else if (u < 0x800) bOff += 2;
                        else bOff += 3;
                    }
                }

                line.hasTrailingNewline = false;
                line.newlineByteOffset = static_cast<int>(line.content.size());
                while (!line.chars.isEmpty() && (line.chars.back().ch == u'\n' || line.chars.back().ch == u'\r'))
                {
                    line.hasTrailingNewline = true;
                    line.newlineByteOffset = line.chars.back().byteOffset;
                    line.chars.pop_back();
                }

                line.displayText.reserve(line.chars.size());
                for (const auto &chInfo : line.chars)
                {
                    line.displayText += (chInfo.ch == u'\t') ? QChar(u'→') : chInfo.ch;
                }

                line.rect.setTop(y + file_h + hunk_h);
                line.rect.setLeft(leftMargin);
                line.rect.setWidth(totalDiffWidth);

                hunk_h += lineHeight;

                line.rect.setHeight(lineHeight);

                hunk.lines.push_back(line);
            }

            // ---------------------------------------------------------------
            // Inline diff: pair adjacent deleted/added lines and compute
            // character- or word-level diff ranges.
            // ---------------------------------------------------------------
            if (m_inlineDiffMode != InlineDiffMode::Off)
            {
                bool wordLevel = (m_inlineDiffMode == InlineDiffMode::WordLevel);
                int nLines = static_cast<int>(hunk.lines.size());
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
                        int pairs = std::min(delCount, addCount);
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

                            auto updateCharOffsets = [](QGitDiffWidgetPrivateLine &l) {
                                for (qsizetype r = 0; r < l.inlineRanges.size(); ++r) {
                                    auto &range = l.inlineRanges[r];
                                    int charStart = 0;
                                    int charLength = 0;
                                    for (qsizetype i = 0; i < l.chars.size(); ++i) {
                                        if (l.chars[i].byteOffset < range.start)
                                            charStart = static_cast<int>(i + 1);
                                        if (l.chars[i].byteOffset >= range.start &&
                                            l.chars[i].byteOffset < range.start + range.length)
                                        {
                                            charLength++;
                                        }
                                    }
                                    range.charStart = charStart;
                                    range.charLength = charLength;
                                }
                            };
                            updateCharOffsets(delLine);
                            updateCharOffsets(addLine);
                        }
                    }

                    if (delEnd == addEnd && delEnd == li) ++li; // context line, skip
                }
            }

            // ---------------------------------------------------------------
            // Side-by-side pairing: group deletion chunks and addition chunks
            // ---------------------------------------------------------------
            if (isSideBySide)
            {
                int nLines = static_cast<int>(hunk.lines.size());
                int li = 0;
                int sideRowY = y + file_h + hunkHeaderHeight;

                while (li < nLines)
                {
                    if (hunk.lines[li].origin == ' ')
                    {
                        QGitDiffWidgetPrivateSideRow sRow;
                        sRow.oldLineIdx = li;
                        sRow.newLineIdx = li;
                        sRow.rect = QRect(leftMargin, sideRowY, totalDiffWidth, lineHeight);
                        sRow.leftRect = QRect(leftMargin, sideRowY, sidePanelWidth, lineHeight);
                        sRow.rightRect = QRect(leftMargin + sidePanelWidth + 1, sideRowY, sidePanelWidth, lineHeight);
                        hunk.sideRows.push_back(sRow);
                        sideRowY += lineHeight;
                        ++li;
                    }
                    else
                    {
                        int delStart = li;
                        while (li < nLines && hunk.lines[li].origin == '-') ++li;
                        int delEnd = li;

                        int addStart = li;
                        while (li < nLines && hunk.lines[li].origin == '+') ++li;
                        int addEnd = li;

                        int delCount = delEnd - delStart;
                        int addCount = addEnd - addStart;
                        int maxCount = std::max(delCount, addCount);

                        for (int k = 0; k < maxCount; ++k)
                        {
                            QGitDiffWidgetPrivateSideRow sRow;
                            sRow.oldLineIdx = (k < delCount) ? (delStart + k) : -1;
                            sRow.newLineIdx = (k < addCount) ? (addStart + k) : -1;
                            sRow.rect = QRect(leftMargin, sideRowY, totalDiffWidth, lineHeight);
                            sRow.leftRect = QRect(leftMargin, sideRowY, sidePanelWidth, lineHeight);
                            sRow.rightRect = QRect(leftMargin + sidePanelWidth + 1, sideRowY, sidePanelWidth, lineHeight);
                            hunk.sideRows.push_back(sRow);
                            sideRowY += lineHeight;
                        }
                    }
                }

                hunk_h = sideRowY - (y + file_h);
            }

            hunk.rect.setHeight(hunk_h);
            file_h += hunk_h;

            file.hunks.push_back(hunk);
        }

        file.rect.setHeight(file_h);

        y += file_h;

        m_private->files.push_back(file);
    }

    newSize = QSize(contentsMargins().left() + totalDiffWidth + contentsMargins().right(), y + contentsMargins().bottom());

    setMinimumSize(newSize);
    setMaximumSize(QWIDGETSIZE_MAX, newSize.height());
    resize(std::max(width(), newSize.width()), newSize.height());

    updateLayoutWidths();
    update();
}

void QGitDiffWidget::updateLayoutWidths()
{
    if (m_private->files.isEmpty())
        return;

    const int leftMargin = contentsMargins().left();
    const int rightMargin = contentsMargins().right();
    const int effectiveWidth = std::max(width() - leftMargin - rightMargin, m_private->minContentWidth);
    const bool isSideBySide = (m_diffViewMode == DiffViewMode::SideBySide);
    const int sideW = isSideBySide ? std::max(m_private->sidePanelWidth, (effectiveWidth - 1) / 2) : effectiveWidth;
    const int rightSideW = isSideBySide ? (effectiveWidth - sideW - 1) : effectiveWidth;

    for (auto &file : m_private->files) {
        file.rect.setWidth(effectiveWidth);
        file.headerRect.setWidth(effectiveWidth);
        for (auto &hunk : file.hunks) {
            hunk.rect.setWidth(effectiveWidth);
            hunk.headerRect.setWidth(effectiveWidth);
            for (auto &line : hunk.lines) {
                line.rect.setWidth(effectiveWidth);
            }
            for (auto &sRow : hunk.sideRows) {
                sRow.rect.setWidth(effectiveWidth);
                sRow.leftRect.setLeft(leftMargin);
                sRow.leftRect.setWidth(sideW);
                sRow.rightRect.setLeft(leftMargin + sideW + 1);
                sRow.rightRect.setWidth(rightSideW);
            }
        }
    }
}

void QGitDiffWidget::paintEvent(QPaintEvent *event)
{
    QStylePainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const DiffThemeColors colors = resolveThemeColors(palette());
    painter.fillRect(event->rect(), colors.widgetBg);

    QFontMetrics fm(m_font);
    QFontMetrics uiFm(m_uiFont);
    QFont boldUiFont = m_uiFont;
    boldUiFont.setBold(true);
    QFontMetrics boldUiFm(boldUiFont);

    int fileIndex = 0, hunkIndex = 0, lineIndex = 0;

    const auto &files = m_private->files;
    const int lineNumWidth = m_private->lineNumWidth;
    const int signWidth = m_private->signWidth;
    const bool isSideBySide = (m_diffViewMode == DiffViewMode::SideBySide);

    for (const auto &file : files)
    {
        if (!event->region().intersected(file.rect).isEmpty())
        {
            // ---------------------------------------------------------------
            // File header card bar
            // ---------------------------------------------------------------
            if (!event->region().intersected(file.headerRect).isEmpty())
            {
                painter.fillRect(file.headerRect, colors.fileHeaderBg);
                painter.setPen(QPen(colors.fileCardBorder, 1));
                painter.drawLine(file.headerRect.left(), file.headerRect.bottom(),
                                 file.headerRect.right(), file.headerRect.bottom());

                // Right-side diff stats (+N -M) and status pill
                int rightCursor = file.headerRect.right() - 12;
                const int headerCenterY = file.headerRect.top() + (file.headerRect.height() - uiFm.height()) / 2 + uiFm.ascent();

                // Status badge
                QString badgeText;
                QColor badgeAccent;
                switch (file.status) {
                case GIT_DELTA_ADDED:
                    badgeText = tr("ADDED");
                    badgeAccent = colors.addedSignText;
                    break;
                case GIT_DELTA_DELETED:
                    badgeText = tr("DELETED");
                    badgeAccent = colors.deletedSignText;
                    break;
                case GIT_DELTA_RENAMED:
                    badgeText = tr("RENAMED");
                    badgeAccent = colors.hunkHeaderText;
                    break;
                case GIT_DELTA_COPIED:
                    badgeText = tr("COPIED");
                    badgeAccent = colors.hunkHeaderText;
                    break;
                case GIT_DELTA_TYPECHANGE:
                    badgeText = tr("TYPE");
                    badgeAccent = colors.hunkHeaderText;
                    break;
                default:
                    if (file.status == GIT_DELTA_MODIFIED) {
                        badgeText = tr("MODIFIED");
                        badgeAccent = colors.hunkHeaderText;
                    }
                    break;
                }

                if (!badgeText.isEmpty()) {
                    QFont badgeFont = m_uiFont;
                    badgeFont.setBold(true);
                    if (badgeFont.pointSizeF() > 0)
                        badgeFont.setPointSizeF(std::max(7.5, badgeFont.pointSizeF() - 1.5));
                    QFontMetrics bFm(badgeFont);
                    const int pillPadX = 8;
                    const int pillH = bFm.height() + 4;
                    const int pillW = bFm.horizontalAdvance(badgeText) + pillPadX * 2;
                    const QRect pillRect(rightCursor - pillW,
                                         file.headerRect.top() + (file.headerRect.height() - pillH) / 2,
                                         pillW, pillH);

                    QPainterPath pillPath;
                    pillPath.addRoundedRect(pillRect, 4.0, 4.0);
                    painter.fillPath(pillPath, blendColor(colors.fileHeaderBg, badgeAccent, colors.isDark ? 0.22 : 0.14));
                    painter.setPen(QPen(blendColor(colors.fileHeaderBg, badgeAccent, 0.45), 1));
                    painter.drawPath(pillPath);

                    painter.setFont(badgeFont);
                    painter.setPen(badgeAccent);
                    painter.drawText(pillRect, Qt::AlignCenter, badgeText);
                    rightCursor = pillRect.left() - 10;
                }

                // +added / -deleted line counters
                painter.setFont(boldUiFont);
                if (file.deletedCount > 0) {
                    const QString delStr = QStringLiteral("-%1").arg(file.deletedCount);
                    const int w = boldUiFm.horizontalAdvance(delStr);
                    painter.setPen(colors.deletedSignText);
                    painter.drawText(rightCursor - w, headerCenterY, delStr);
                    rightCursor -= w + 8;
                }
                if (file.addedCount > 0) {
                    const QString addStr = QStringLiteral("+%1").arg(file.addedCount);
                    const int w = boldUiFm.horizontalAdvance(addStr);
                    painter.setPen(colors.addedSignText);
                    painter.drawText(rightCursor - w, headerCenterY, addStr);
                    rightCursor -= w + 12;
                }

                // File path (directory muted, filename bold)
                QString displayPath = file.new_file.path().isEmpty() ? file.old_file.path() : file.new_file.path();
                if (file.status == GIT_DELTA_RENAMED && !file.old_file.path().isEmpty() && file.old_file.path() != file.new_file.path()) {
                    displayPath = file.old_file.path() + QStringLiteral(" → ") + file.new_file.path();
                }

                const int pathLeft = file.headerRect.left() + 12;
                const int maxPathWidth = std::max(40, rightCursor - pathLeft - 8);
                const int slashIdx = displayPath.lastIndexOf(u'/');
                if (slashIdx >= 0 && file.status != GIT_DELTA_RENAMED) {
                    const QString dirPart = displayPath.left(slashIdx + 1);
                    const QString namePart = displayPath.mid(slashIdx + 1);
                    const int nameW = boldUiFm.horizontalAdvance(namePart);
                    const int availDirW = maxPathWidth - nameW;
                    if (availDirW > 20) {
                        const QString elidedDir = uiFm.elidedText(dirPart, Qt::ElideLeft, availDirW);
                        painter.setFont(m_uiFont);
                        painter.setPen(colors.fileHeaderDirText);
                        painter.drawText(pathLeft, headerCenterY, elidedDir);
                        const int dirW = uiFm.horizontalAdvance(elidedDir);
                        painter.setFont(boldUiFont);
                        painter.setPen(colors.fileHeaderText);
                        painter.drawText(pathLeft + dirW, headerCenterY, namePart);
                    } else {
                        painter.setFont(boldUiFont);
                        painter.setPen(colors.fileHeaderText);
                        painter.drawText(pathLeft, headerCenterY, boldUiFm.elidedText(displayPath, Qt::ElideLeft, maxPathWidth));
                    }
                } else {
                    painter.setFont(boldUiFont);
                    painter.setPen(colors.fileHeaderText);
                    painter.drawText(pathLeft, headerCenterY, boldUiFm.elidedText(displayPath, Qt::ElideMiddle, maxPathWidth));
                }
            }

            painter.setFont(m_font);

            hunkIndex = 0;
            for (const auto &hunk : file.hunks)
            {
                if (!event->region().intersected(hunk.rect).isEmpty())
                {
                    // -------------------------------------------------------
                    // Hunk header (@@ ... @@)
                    // -------------------------------------------------------
                    if (!event->region().intersected(hunk.headerRect).isEmpty())
                    {
                        painter.fillRect(hunk.headerRect, colors.hunkHeaderBg);
                        painter.setPen(QPen(colors.hunkHeaderBorder, 1));
                        painter.drawLine(hunk.headerRect.left(), hunk.headerRect.bottom(),
                                         hunk.headerRect.right(), hunk.headerRect.bottom());

                        const int yHunkFont = hunk.headerRect.top() + (hunk.headerRect.height() - m_fontHeight) / 2 + m_fontAscent;
                        painter.setPen(colors.hunkHeaderText);
                        const QString hunkText = fm.elidedText(hunk.header.trimmed(), Qt::ElideRight,
                                                               std::max(40, hunk.headerRect.width() - 24));
                        painter.drawText(hunk.headerRect.left() + 12, yHunkFont, hunkText);
                    }

                    if (isSideBySide)
                    {
                        // ---------------------------------------------------
                        // Side-by-side rendering
                        // ---------------------------------------------------
                        for (const auto &sRow : hunk.sideRows)
                        {
                            if (!event->region().intersected(sRow.rect).isEmpty())
                            {
                                const int yFont = sRow.rect.top() + (sRow.rect.height() - m_fontHeight) / 2 + m_fontAscent;

                                // Left panel (Old / Base file)
                                if (sRow.oldLineIdx >= 0)
                                {
                                    const auto &oldLine = hunk.lines.at(sRow.oldLineIdx);
                                    const bool isDel = (oldLine.origin == '-');
                                    const QColor &rowBg = isDel ? colors.deletedBg : colors.contextBg;
                                    const QColor &gutterBg = isDel ? colors.deletedGutterBg : colors.contextGutterBg;
                                    const QColor &numColor = isDel ? colors.deletedLineNumText : colors.lineNumText;

                                    const int gutterW = 6 + lineNumWidth + 4;
                                    const QRect leftGutterRect(sRow.leftRect.left(), sRow.leftRect.top(), gutterW, sRow.leftRect.height());
                                    const QRect leftCodeRect(sRow.leftRect.left() + gutterW, sRow.leftRect.top(),
                                                             sRow.leftRect.width() - gutterW, sRow.leftRect.height());

                                    painter.fillRect(leftGutterRect, gutterBg);
                                    painter.fillRect(leftCodeRect, rowBg);

                                    // Gutter separator
                                    painter.setPen(QPen(colors.gutterBorder, 1));
                                    painter.drawLine(leftGutterRect.right(), sRow.leftRect.top(),
                                                     leftGutterRect.right(), sRow.leftRect.bottom());

                                    // Right-aligned line number
                                    if (oldLine.old_lineno >= 0) {
                                        painter.setPen(numColor);
                                        painter.drawText(QRect(sRow.leftRect.left() + 4, sRow.leftRect.top(),
                                                               lineNumWidth - 2, sRow.leftRect.height()),
                                                         Qt::AlignRight | Qt::AlignVCenter,
                                                         QString::number(oldLine.old_lineno));
                                    }

                                    // Sign indicator ('-' or ' ')
                                    const int signX = leftGutterRect.right() + 6;
                                    if (isDel) {
                                        painter.setPen(colors.deletedSignText);
                                        painter.drawText(signX, yFont, QStringLiteral("-"));
                                    }

                                    const int leftContentX = leftGutterRect.right() + 4 + signWidth;

                                    // Inline diff highlights on deleted line
                                    if (!oldLine.inlineRanges.isEmpty())
                                    {
                                        for (const auto &range : oldLine.inlineRanges)
                                        {
                                            if (range.charLength <= 0) continue;

                                            int xRangeStart = leftContentX + fm.horizontalAdvance(oldLine.displayText, range.charStart);
                                            int xRangeEnd   = leftContentX + fm.horizontalAdvance(oldLine.displayText, range.charStart + range.charLength);

                                            painter.fillRect(QRect(xRangeStart, sRow.leftRect.top(),
                                                                   xRangeEnd - xRangeStart, sRow.leftRect.height()),
                                                             colors.deletedInlineBg);
                                        }
                                    }

                                    drawLineContent(painter, fm, oldLine,
                                                    leftContentX, yFont,
                                                    m_showWhitespaceChars, colors.contextText);
                                }
                                else
                                {
                                    drawEmptySlotHatch(painter, sRow.leftRect, colors);
                                }

                                // Center 1px separator divider
                                const int divX = sRow.leftRect.right() + 1;
                                painter.setPen(QPen(colors.dividerColor, 1));
                                painter.drawLine(divX, sRow.rect.top(), divX, sRow.rect.bottom());

                                // Right panel (New / Modified file)
                                if (sRow.newLineIdx >= 0)
                                {
                                    const auto &newLine = hunk.lines.at(sRow.newLineIdx);
                                    const bool isAdd = (newLine.origin == '+');
                                    const QColor &rowBg = isAdd ? colors.addedBg : colors.contextBg;
                                    const QColor &gutterBg = isAdd ? colors.addedGutterBg : colors.contextGutterBg;
                                    const QColor &numColor = isAdd ? colors.addedLineNumText : colors.lineNumText;

                                    const int gutterW = 6 + lineNumWidth + 4;
                                    const QRect rightGutterRect(sRow.rightRect.left(), sRow.rightRect.top(), gutterW, sRow.rightRect.height());
                                    const QRect rightCodeRect(sRow.rightRect.left() + gutterW, sRow.rightRect.top(),
                                                              sRow.rightRect.width() - gutterW, sRow.rightRect.height());

                                    painter.fillRect(rightGutterRect, gutterBg);
                                    painter.fillRect(rightCodeRect, rowBg);

                                    // Gutter separator
                                    painter.setPen(QPen(colors.gutterBorder, 1));
                                    painter.drawLine(rightGutterRect.right(), sRow.rightRect.top(),
                                                     rightGutterRect.right(), sRow.rightRect.bottom());

                                    // Right-aligned line number
                                    if (newLine.new_lineno >= 0) {
                                        painter.setPen(numColor);
                                        painter.drawText(QRect(sRow.rightRect.left() + 4, sRow.rightRect.top(),
                                                               lineNumWidth - 2, sRow.rightRect.height()),
                                                         Qt::AlignRight | Qt::AlignVCenter,
                                                         QString::number(newLine.new_lineno));
                                    }

                                    // Sign indicator ('+' or ' ')
                                    const int signX = rightGutterRect.right() + 6;
                                    if (isAdd) {
                                        painter.setPen(colors.addedSignText);
                                        painter.drawText(signX, yFont, QStringLiteral("+"));
                                    }

                                    const int rightContentX = rightGutterRect.right() + 4 + signWidth;

                                    // Inline diff highlights on added line
                                    if (!newLine.inlineRanges.isEmpty())
                                    {
                                        for (const auto &range : newLine.inlineRanges)
                                        {
                                            if (range.charLength <= 0) continue;

                                            int xRangeStart = rightContentX + fm.horizontalAdvance(newLine.displayText, range.charStart);
                                            int xRangeEnd   = rightContentX + fm.horizontalAdvance(newLine.displayText, range.charStart + range.charLength);

                                            painter.fillRect(QRect(xRangeStart, sRow.rightRect.top(),
                                                                   xRangeEnd - xRangeStart, sRow.rightRect.height()),
                                                             colors.addedInlineBg);
                                        }
                                    }

                                    drawLineContent(painter, fm, newLine,
                                                    rightContentX, yFont,
                                                    m_showWhitespaceChars, colors.contextText);
                                }
                                else
                                {
                                    drawEmptySlotHatch(painter, sRow.rightRect, colors);
                                }

                                // Hover highlight on side row line
                                if (!m_readonly && (fileIndex == m_hoverFile) && (hunkIndex == m_hoverHunk))
                                {
                                    if (m_hoverLine == sRow.oldLineIdx && sRow.oldLineIdx >= 0 && m_hoverSide == 0)
                                        drawHoverOverlay(painter, sRow.leftRect, colors);
                                    else if (m_hoverLine == sRow.newLineIdx && sRow.newLineIdx >= 0 && m_hoverSide == 1)
                                        drawHoverOverlay(painter, sRow.rightRect, colors);
                                }
                            }
                        }
                    }
                    else
                    {
                        // ---------------------------------------------------
                        // Unified rendering
                        // ---------------------------------------------------
                        const int gutterW = 6 + lineNumWidth * 2 + 6;
                        const int contentX = m_private->contentOffset + contentsMargins().left();

                        lineIndex = 0;
                        for (const auto &line : hunk.lines)
                        {
                            if (!event->region().intersected(line.rect).isEmpty())
                            {
                                const bool isDel = (line.origin == '-');
                                const bool isAdd = (line.origin == '+');

                                const QColor &rowBg = isDel ? colors.deletedBg
                                                    : (isAdd ? colors.addedBg : colors.contextBg);
                                const QColor &gutterBg = isDel ? colors.deletedGutterBg
                                                       : (isAdd ? colors.addedGutterBg : colors.contextGutterBg);
                                const QColor &numColor = isDel ? colors.deletedLineNumText
                                                       : (isAdd ? colors.addedLineNumText : colors.lineNumText);

                                const QRect gutterRect(line.rect.left(), line.rect.top(), gutterW, line.rect.height());
                                const QRect codeRect(line.rect.left() + gutterW, line.rect.top(),
                                                     line.rect.width() - gutterW, line.rect.height());

                                painter.fillRect(gutterRect, gutterBg);
                                painter.fillRect(codeRect, rowBg);

                                // Gutter separator
                                painter.setPen(QPen(colors.gutterBorder, 1));
                                painter.drawLine(gutterRect.right(), line.rect.top(),
                                                 gutterRect.right(), line.rect.bottom());

                                // Right-aligned old & new line numbers
                                painter.setPen(numColor);
                                if (line.old_lineno >= 0) {
                                    painter.drawText(QRect(line.rect.left() + 4, line.rect.top(),
                                                           lineNumWidth - 4, line.rect.height()),
                                                     Qt::AlignRight | Qt::AlignVCenter,
                                                     QString::number(line.old_lineno));
                                }
                                if (line.new_lineno >= 0) {
                                    painter.drawText(QRect(line.rect.left() + 4 + lineNumWidth + 2, line.rect.top(),
                                                           lineNumWidth - 4, line.rect.height()),
                                                     Qt::AlignRight | Qt::AlignVCenter,
                                                     QString::number(line.new_lineno));
                                }

                                const int yFont = line.rect.top() + (line.rect.height() - m_fontHeight) / 2 + m_fontAscent;

                                // Sign indicator ('+', '-', or ' ')
                                if (isDel || isAdd) {
                                    painter.setPen(isDel ? colors.deletedSignText : colors.addedSignText);
                                    painter.drawText(gutterRect.right() + 6, yFont, isDel ? QStringLiteral("-") : QStringLiteral("+"));
                                }

                                // Inline diff highlighting
                                if (!line.inlineRanges.isEmpty())
                                {
                                    const QColor &hlColor = isDel ? colors.deletedInlineBg : colors.addedInlineBg;

                                    for (const auto &range : line.inlineRanges)
                                    {
                                        if (range.charLength <= 0) continue;

                                        int xRangeStart = contentX + fm.horizontalAdvance(line.displayText, range.charStart);
                                        int xRangeEnd   = contentX + fm.horizontalAdvance(line.displayText, range.charStart + range.charLength);

                                        painter.fillRect(QRect(xRangeStart, line.rect.top(),
                                                               xRangeEnd - xRangeStart, line.rect.height()),
                                                         hlColor);
                                    }
                                }

                                drawLineContent(painter, fm, line,
                                                contentX, yFont,
                                                m_showWhitespaceChars, colors.contextText);

                                if (!m_readonly && (fileIndex == m_hoverFile) && (hunkIndex == m_hoverHunk) && (lineIndex == m_hoverLine))
                                {
                                    drawHoverOverlay(painter, codeRect, colors);
                                }
                            }

                            lineIndex++;
                        }
                    }

                    // Hunk hover highlight (when hovering gutter or hunk header)
                    if (!m_readonly && (fileIndex == m_hoverFile) && (hunkIndex == m_hoverHunk) && (m_hoverLine == -1))
                    {
                        drawHoverOverlay(painter, hunk.rect, colors);
                    }
                }

                hunkIndex++;
            }

            // Crisp card border around the entire file diff block
            painter.setPen(QPen(colors.fileCardBorder, 1));
            painter.setBrush(Qt::NoBrush);
            painter.drawRect(file.rect.adjusted(0, 0, -1, -1));
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

void QGitDiffWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateLayoutWidths();
}

void QGitDiffWidget::leaveEvent(QEvent *event)
{
    QWidget::leaveEvent(event);

    if (!m_readonly && (m_hoverFile != -1 || m_hoverHunk != -1 || m_hoverLine != -1))
    {
        m_hoverFile = -1;
        m_hoverHunk = -1;
        m_hoverLine = -1;
        m_hoverSide = 0;
        setCursor(Qt::ArrowCursor);
        update();
    }
}

void QGitDiffWidget::changeEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::ThemeChange:
    case QEvent::PaletteChange:
    case QEvent::ApplicationPaletteChange:
        update();
        break;
    case QEvent::FontChange:
    case QEvent::StyleChange:
        initFonts();
        update();
        break;
    default:
        break;
    }

    QWidget::changeEvent(event);
}

void QGitDiffWidget::updatePosition()
{
    int l_hoverFile = -1;
    int l_hoverHunk = -1;
    int l_hoverLine = -1;
    int l_hoverSide = 0;

    if (m_readonly)
        return;

    QPoint point = this->mapFromGlobal(QCursor::pos());

    int file_index = 0;
    const auto &files = m_private->files;
    const int gutterBoundaryX = contentsMargins().left() + 6 + m_private->lineNumWidth * 2 + 6;
    const bool isSideBySide = (m_diffViewMode == DiffViewMode::SideBySide);

    for (const auto &file : files)
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
            for (const auto &hunk : file.hunks)
            {
                if (hunk.rect.contains(point))
                {
                    l_hoverHunk = hunk_index;

                    if (hunk.headerRect.contains(point))
                    {
                        // Hovering hunk header selects the whole hunk
                        l_hoverLine = -1;
                    }
                    else if (isSideBySide)
                    {
                        for (const auto &sRow : hunk.sideRows)
                        {
                            if (sRow.rect.contains(point))
                            {
                                if (sRow.leftRect.contains(point) && sRow.oldLineIdx >= 0)
                                {
                                    const auto &oldLine = hunk.lines.at(sRow.oldLineIdx);
                                    if (oldLine.origin == '-')
                                    {
                                        l_hoverLine = sRow.oldLineIdx;
                                        l_hoverSide = 0;
                                    }
                                    else
                                    {
                                        l_hoverFile = -1;
                                        l_hoverHunk = -1;
                                        l_hoverLine = -1;
                                    }
                                }
                                else if (sRow.rightRect.contains(point) && sRow.newLineIdx >= 0)
                                {
                                    const auto &newLine = hunk.lines.at(sRow.newLineIdx);
                                    if (newLine.origin == '+')
                                    {
                                        l_hoverLine = sRow.newLineIdx;
                                        l_hoverSide = 1;
                                    }
                                    else
                                    {
                                        l_hoverFile = -1;
                                        l_hoverHunk = -1;
                                        l_hoverLine = -1;
                                    }
                                }
                                else
                                {
                                    l_hoverFile = -1;
                                    l_hoverHunk = -1;
                                    l_hoverLine = -1;
                                }

                                break;
                            }
                        }
                    }
                    else
                    {
                        if (point.x() >= gutterBoundaryX)
                        {
                            int line_index = 0;
                            for (const auto &line : hunk.lines)
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
                    }

                    break;
                }

                hunk_index++;
            }

            break;
        }

        file_index++;
    }

    if ((m_hoverFile != l_hoverFile)||(m_hoverHunk != l_hoverHunk)||(m_hoverLine != l_hoverLine)||(m_hoverSide != l_hoverSide))
    {
        m_hoverFile = l_hoverFile;
        m_hoverHunk = l_hoverHunk;
        m_hoverLine = l_hoverLine;
        m_hoverSide = l_hoverSide;

        if (m_hoverHunk >= 0) {
            setCursor(Qt::PointingHandCursor);
        } else {
            setCursor(Qt::ArrowCursor);
        }

        update();
    }
}
