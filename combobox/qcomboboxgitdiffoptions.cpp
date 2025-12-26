#include "qcomboboxgitdiffoptions.h"

#include <QStandardItemModel>
#include <QAbstractItemView>
#include <QStylePainter>


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
    : QComboBox(parent)
    , m_icon(":/QComboBoxGitDiffOptions/gear")
{
    QStandardItemModel *model = new QStandardItemModel();
    setModel(model);

    QStandardItem* item = nullptr;

    item = new QStandardItem(tr("External Diff"));
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    model->appendRow(item);

    insertSeparator(model->rowCount());

    item = new QStandardItem(tr("Ignore whitespace"));
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    item->setData(Qt::Unchecked, Qt::CheckStateRole);
    model->appendRow(item);

    item = new QStandardItem(tr("Show whitespace"));
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    item->setData(Qt::Checked, Qt::CheckStateRole);
    model->appendRow(item);

    insertSeparator(model->rowCount());

    item = new QStandardItem(tr("Lines of context"));
    item->setFlags(Qt::NoItemFlags);
    model->appendRow(item);

    item = new QStandardItem(tr("1"));
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    item->setData(Qt::Unchecked, Qt::CheckStateRole);
    model->appendRow(item);

    item = new QStandardItem(tr("3"));
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    item->setData(Qt::Checked, Qt::CheckStateRole);
    model->appendRow(item);

    item = new QStandardItem(tr("6"));
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    item->setData(Qt::Unchecked, Qt::CheckStateRole);
    model->appendRow(item);

    item = new QStandardItem(tr("12"));
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    item->setData(Qt::Unchecked, Qt::CheckStateRole);
    model->appendRow(item);

    item = new QStandardItem(tr("25"));
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    item->setData(Qt::Unchecked, Qt::CheckStateRole);
    model->appendRow(item);

    item = new QStandardItem(tr("50"));
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    item->setData(Qt::Unchecked, Qt::CheckStateRole);
    model->appendRow(item);

    item = new QStandardItem(tr("100"));
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    item->setData(Qt::Unchecked, Qt::CheckStateRole);
    model->appendRow(item);

    view()->setMinimumWidth(view()->sizeHintForColumn(0));

    connect(this, &QComboBox::activated, this,  &QComboBoxGitDiffOptions::activated);
}

QSize QComboBoxGitDiffOptions::sizeHint() const
{
    return minimumSizeHint();
}

QSize QComboBoxGitDiffOptions::minimumSizeHint() const
{
    QStyleOptionComboBox opt;
    opt.initFrom(this);

    QSize contentSize;

    //opt.currentIcon = m_icon;
    //opt.iconSize = iconSize();

// TODO: Periodicly check if we still need this Windows style fix.
#ifdef Q_OS_WIN
    contentSize = QSize(0, iconSize().width());
#else
    contentSize = iconSize();
    //contentSize = {16, 16};
#endif

    return style()->sizeFromContents(QStyle::CT_ComboBox, &opt, contentSize, this);
}

void QComboBoxGitDiffOptions::paintEvent(QPaintEvent *event)
{
    QStyleOptionComboBox opt;
    QStylePainter p(this);

    Q_UNUSED(event);

    opt.initFrom(this);

    opt.currentIcon = m_icon;
    opt.iconSize = iconSize();

    p.drawComplexControl(QStyle::CC_ComboBox, opt);
    p.drawControl(QStyle::CE_ComboBoxLabel, opt);
}

void QComboBoxGitDiffOptions::showPopup()
{
    view()->selectionModel()->reset();

    QComboBox::showPopup();

    QWidget *popup = view()->parentWidget();
    if (popup) {
        QPoint pos = mapToGlobal(QPoint(0, height()));
        popup->move(pos.x(), pos.y());
    }
}

void QComboBoxGitDiffOptions::activated(int index)
{
    QAbstractItemModel *items = model();

    if ((index >= ITEM_IGNORE_WHITESPACE)&&(index <= ITEM_SHOW_WHITESPACE))
    {
        for(int c = ITEM_IGNORE_WHITESPACE; c <= ITEM_SHOW_WHITESPACE; c++)
        {
            items->setData(items->index(c, 0), (c == index)?Qt::Checked:Qt::Unchecked, Qt::CheckStateRole);
        }
    }

    if ((index >= ITEM_SHOW_1_LINE)&&(index <= ITEM_SHOW_100_LINE))
    {
        for(int c = ITEM_SHOW_1_LINE; c <= ITEM_SHOW_100_LINE; c++)
        {
            items->setData(items->index(c, 0), (c == index)?Qt::Checked:Qt::Unchecked, Qt::CheckStateRole);
        }
    }
}
