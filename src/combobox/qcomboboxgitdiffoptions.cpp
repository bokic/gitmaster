#include "qcomboboxgitdiffoptions.h"

#include <QStandardItemModel>
#include <QAbstractItemView>
#include <QStylePainter>
#include <QApplication>
#include <QSvgRenderer>
#include <QFile>


enum {
    ITEM_EXTERNAL_DIFF,
    ITEM_SEPARATOR1,
    ITEM_DIFF_VIEW_HEADER,
    ITEM_DIFF_VIEW_UNIFIED,
    ITEM_DIFF_VIEW_SIDE_BY_SIDE,
    ITEM_SEPARATOR2,
    ITEM_IGNORE_WHITESPACE,
    ITEM_SHOW_WHITESPACE,
    ITEM_SEPARATOR3,
    ITEM_INLINE_DIFF_HEADER,
    ITEM_INLINE_DIFF_OFF,
    ITEM_INLINE_DIFF_CHAR,
    ITEM_INLINE_DIFF_WORD,
    ITEM_SEPARATOR4,
    ITEM_VISUALIZE_WHITESPACE,
    ITEM_SEPARATOR5,
    ITEM_SHOW_1_LINE,
    ITEM_SHOW_2_LINE,
    ITEM_SHOW_3_LINE,
    ITEM_SHOW_6_LINE,
    ITEM_SHOW_12_LINE,
    ITEM_SHOW_25_LINE,
    ITEM_SHOW_50_LINE,
    ITEM_SHOW_100_LINE,
};

QComboBoxGitDiffOptions::QComboBoxGitDiffOptions(QWidget *parent)
    : QComboBoxGitBase(parent)
    , m_iconChecked(":/QCustomComboBox/check")
    , m_iconUnchecked(":/QCustomComboBox/uncheck")
{
    initStandardModel();
    updateIconColor();

    addOptionItem(tr("External Diff"), true, Qt::Unchecked, m_iconUnchecked);

    insertSeparator(count());

    addHeaderItem(tr("Diff layout"), m_iconUnchecked);
    addOptionItem(tr("Unified"), true, Qt::Checked, m_iconChecked);
    addOptionItem(tr("Side-by-side"), true, Qt::Unchecked, m_iconUnchecked);

    insertSeparator(count());

    addOptionItem(tr("Ignore whitespace"), true, Qt::Unchecked, m_iconUnchecked);
    addOptionItem(tr("Show whitespace"), true, Qt::Checked, m_iconChecked);

    insertSeparator(count());

    addHeaderItem(tr("Inline diff"), m_iconUnchecked);
    addOptionItem(tr("Off"), true, Qt::Checked, m_iconChecked);
    addOptionItem(tr("Character-level"), true, Qt::Unchecked, m_iconUnchecked);
    addOptionItem(tr("Word-level"), true, Qt::Unchecked, m_iconUnchecked);

    insertSeparator(count());

    addOptionItem(tr("Visualize Whitespace"), true, Qt::Unchecked, m_iconUnchecked);

    insertSeparator(count());

    addHeaderItem(tr("Lines of context"), m_iconUnchecked);
    addOptionItem(tr("1"), true, Qt::Unchecked, m_iconUnchecked);
    addOptionItem(tr("3"), true, Qt::Checked, m_iconChecked);
    addOptionItem(tr("6"), true, Qt::Unchecked, m_iconUnchecked);
    addOptionItem(tr("12"), true, Qt::Unchecked, m_iconUnchecked);
    addOptionItem(tr("25"), true, Qt::Unchecked, m_iconUnchecked);
    addOptionItem(tr("50"), true, Qt::Unchecked, m_iconUnchecked);
    addOptionItem(tr("100"), true, Qt::Unchecked, m_iconUnchecked);

    view()->setMinimumWidth(view()->sizeHintForColumn(0));

    connect(this, &QComboBox::activated, this, &QComboBoxGitDiffOptions::activated);
}

void QComboBoxGitDiffOptions::activated(int index)
{
    QStandardItemModel *items = qobject_cast<QStandardItemModel *>(model());
    if (!items) return;

    if ((index >= ITEM_DIFF_VIEW_UNIFIED)&&(index <= ITEM_DIFF_VIEW_SIDE_BY_SIDE))
    {
        for(int c = ITEM_DIFF_VIEW_UNIFIED; c <= ITEM_DIFF_VIEW_SIDE_BY_SIDE; c++)
        {
            if (c == index)
            {
                items->setData(items->index(c, 0), Qt::Checked, Qt::CheckStateRole);
                if (m_showIcons) items->item(c, 0)->setIcon(m_iconChecked);
            }
            else
            {
                items->setData(items->index(c, 0), Qt::Unchecked, Qt::CheckStateRole);
                if (m_showIcons) items->item(c, 0)->setIcon(m_iconUnchecked);
            }
        }
    }

    if ((index >= ITEM_IGNORE_WHITESPACE)&&(index <= ITEM_SHOW_WHITESPACE))
    {
        for(int c = ITEM_IGNORE_WHITESPACE; c <= ITEM_SHOW_WHITESPACE; c++)
        {
            if (c == index)
            {
                items->setData(items->index(c, 0), Qt::Checked, Qt::CheckStateRole);
                if (m_showIcons) items->item(c, 0)->setIcon(m_iconChecked);
            }
            else
            {
                items->setData(items->index(c, 0), Qt::Unchecked, Qt::CheckStateRole);
                if (m_showIcons) items->item(c, 0)->setIcon(m_iconUnchecked);
            }
        }
    }

    if ((index >= ITEM_INLINE_DIFF_OFF)&&(index <= ITEM_INLINE_DIFF_WORD))
    {
        for(int c = ITEM_INLINE_DIFF_OFF; c <= ITEM_INLINE_DIFF_WORD; c++)
        {
            if (c == index)
            {
                items->setData(items->index(c, 0), Qt::Checked, Qt::CheckStateRole);
                if (m_showIcons) items->item(c, 0)->setIcon(m_iconChecked);
            }
            else
            {
                items->setData(items->index(c, 0), Qt::Unchecked, Qt::CheckStateRole);
                if (m_showIcons) items->item(c, 0)->setIcon(m_iconUnchecked);
            }
        }
    }

    if (index == ITEM_VISUALIZE_WHITESPACE)
    {
        // Independent checkbox: toggle on/off
        bool current = items->data(items->index(ITEM_VISUALIZE_WHITESPACE, 0), Qt::CheckStateRole).toInt() == Qt::Checked;
        items->setData(items->index(ITEM_VISUALIZE_WHITESPACE, 0), current ? Qt::Unchecked : Qt::Checked, Qt::CheckStateRole);
        if (m_showIcons) items->item(ITEM_VISUALIZE_WHITESPACE, 0)->setIcon(current ? m_iconUnchecked : m_iconChecked);
    }

    if ((index >= ITEM_SHOW_1_LINE)&&(index <= ITEM_SHOW_100_LINE))
    {
        for(int c = ITEM_SHOW_1_LINE; c <= ITEM_SHOW_100_LINE; c++)
        {
            if (c == index)
            {
                items->setData(items->index(c, 0), Qt::Checked, Qt::CheckStateRole);
                if (m_showIcons) items->item(c, 0)->setIcon(m_iconChecked);
            }
            else
            {
                items->setData(items->index(c, 0), Qt::Unchecked, Qt::CheckStateRole);
                if (m_showIcons) items->item(c, 0)->setIcon(m_iconUnchecked);
            }
        }
    }

    emit optionsChanged();
}

QComboBoxGitDiffOptions::DiffViewMode QComboBoxGitDiffOptions::diffViewMode() const
{
    QStandardItemModel *items = qobject_cast<QStandardItemModel *>(model());
    if (!items) return DiffViewMode::Unified;

    if (items->data(items->index(ITEM_DIFF_VIEW_SIDE_BY_SIDE, 0), Qt::CheckStateRole).toInt() == Qt::Checked)
        return DiffViewMode::SideBySide;
    return DiffViewMode::Unified;
}

bool QComboBoxGitDiffOptions::ignoreWhitespace() const
{
    QStandardItemModel *items = qobject_cast<QStandardItemModel *>(model());
    if (!items) return false;

    return items->data(items->index(ITEM_IGNORE_WHITESPACE, 0), Qt::CheckStateRole).toInt() == Qt::Checked;
}

int QComboBoxGitDiffOptions::linesOfContent() const
{
    QStandardItemModel *items = qobject_cast<QStandardItemModel *>(model());
    if (!items) return 1;

    for(int c = ITEM_SHOW_1_LINE; c <= ITEM_SHOW_100_LINE; c++)
    {
        if (items->data(items->index(c, 0), Qt::CheckStateRole).toInt() == Qt::Checked)
        {
            return items->item(c, 0)->text().toInt();
        }
    }

    return 1;
}

QComboBoxGitDiffOptions::InlineDiffMode QComboBoxGitDiffOptions::inlineDiffMode() const
{
    QStandardItemModel *items = qobject_cast<QStandardItemModel *>(model());
    if (!items) return InlineDiffMode::Off;

    if (items->data(items->index(ITEM_INLINE_DIFF_CHAR, 0), Qt::CheckStateRole).toInt() == Qt::Checked)
        return InlineDiffMode::CharacterLevel;
    if (items->data(items->index(ITEM_INLINE_DIFF_WORD, 0), Qt::CheckStateRole).toInt() == Qt::Checked)
        return InlineDiffMode::WordLevel;
    return InlineDiffMode::Off;
}

bool QComboBoxGitDiffOptions::showWhitespaceChars() const
{
    QStandardItemModel *items = qobject_cast<QStandardItemModel *>(model());
    if (!items) return false;

    return items->data(items->index(ITEM_VISUALIZE_WHITESPACE, 0), Qt::CheckStateRole).toInt() == Qt::Checked;
}


void QComboBoxGitDiffOptions::updateIconColor()
{
    bool doUpdate = false;

    QFile checkResource = QFile(":/QCustomComboBox/check");
    if (checkResource.open(QIODeviceBase::ReadOnly))
    {
        auto svgContent = checkResource.readAll();

        QString newColor = palette().color(QPalette::Text).name();
        svgContent.replace("#000000", newColor.toUtf8());

        QSvgRenderer renderer(svgContent);
        if (renderer.isValid()) {
            QImage image(iconSize(), QImage::Format_ARGB32_Premultiplied);
            image.fill(0);
            QPainter painter(&image);
            renderer.render(&painter);
            m_iconChecked = QPixmap::fromImage(image);
            doUpdate = true;
        }
    }

    QFile gearResource = QFile(":/QComboBoxGitDiffOptions/gear");
    if (gearResource.open(QIODeviceBase::ReadOnly))
    {
        auto svgContent = gearResource.readAll();

        QString newColor = palette().color(QPalette::PlaceholderText).name();
        svgContent.replace("#000000", newColor.toUtf8());

        QSvgRenderer renderer(svgContent);
        if (renderer.isValid()) {
            QImage image(iconSize(), QImage::Format_ARGB32_Premultiplied);
            image.fill(0);
            QPainter painter(&image);
            renderer.render(&painter);
            m_icon = QPixmap::fromImage(image);
            doUpdate = true;
        }
    }

    if (doUpdate)
    {
        update();
    }
}
