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
    ITEM_IGNORE_WHITESPACE,
    ITEM_SHOW_WHITESPACE,
    ITEM_SEPARATOR2,
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
    QStandardItemModel *model = new QStandardItemModel();
    setModel(model);

    if (QApplication::style()->name() == "fusion")
    {
        m_showIcons = true;
    }

    updateIconColor();

    QStandardItem* item = nullptr;

    item = new QStandardItem(tr("External Diff"));
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    if (m_showIcons) item->setIcon(m_iconUnchecked);
    model->appendRow(item);

    insertSeparator(model->rowCount());

    item = new QStandardItem(tr("Ignore whitespace"));
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    item->setData(Qt::Unchecked, Qt::CheckStateRole);
    if (m_showIcons) item->setIcon(m_iconUnchecked);
    model->appendRow(item);

    item = new QStandardItem(tr("Show whitespace"));
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    item->setData(Qt::Checked, Qt::CheckStateRole);
    if (m_showIcons) item->setIcon(m_iconChecked);
    model->appendRow(item);

    insertSeparator(model->rowCount());

    item = new QStandardItem(tr("Lines of context"));
    item->setFlags(Qt::NoItemFlags);
    if (m_showIcons) item->setIcon(m_iconUnchecked);
    model->appendRow(item);

    item = new QStandardItem(tr("1"));
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    item->setData(Qt::Unchecked, Qt::CheckStateRole);
    if (m_showIcons) item->setIcon(m_iconUnchecked);
    model->appendRow(item);

    item = new QStandardItem(tr("3"));
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    item->setData(Qt::Checked, Qt::CheckStateRole);
    if (m_showIcons) item->setIcon(m_iconChecked);
    model->appendRow(item);

    item = new QStandardItem(tr("6"));
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    item->setData(Qt::Unchecked, Qt::CheckStateRole);
    if (m_showIcons) item->setIcon(m_iconUnchecked);
    model->appendRow(item);

    item = new QStandardItem(tr("12"));
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    item->setData(Qt::Unchecked, Qt::CheckStateRole);
    if (m_showIcons) item->setIcon(m_iconUnchecked);
    model->appendRow(item);

    item = new QStandardItem(tr("25"));
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    item->setData(Qt::Unchecked, Qt::CheckStateRole);
    if (m_showIcons) item->setIcon(m_iconUnchecked);
    model->appendRow(item);

    item = new QStandardItem(tr("50"));
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    item->setData(Qt::Unchecked, Qt::CheckStateRole);
    if (m_showIcons) item->setIcon(m_iconUnchecked);
    model->appendRow(item);

    item = new QStandardItem(tr("100"));
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    item->setData(Qt::Unchecked, Qt::CheckStateRole);
    if (m_showIcons) item->setIcon(m_iconUnchecked);
    model->appendRow(item);

    view()->setMinimumWidth(view()->sizeHintForColumn(0));

    connect(this, &QComboBox::activated, this,  &QComboBoxGitDiffOptions::activated);
}

void QComboBoxGitDiffOptions::activated(int index)
{
    QStandardItemModel *items = qobject_cast<QStandardItemModel *>(model());
    if (!items) return;

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
