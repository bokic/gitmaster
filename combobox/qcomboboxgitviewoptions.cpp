#include "qcomboboxgitviewoptions.h"

#include <QStandardItemModel>
#include <QAbstractItemView>
#include <QStylePainter>


enum {
    ITEM_FLAT_LIST_SINGLE_COLUMN,
    ITEM_FLAT_LIST_MULTIPLE_COLUMN,
    ITEM_TREE_VIEW,
    ITEM_SEPARATOR,
    ITEM_NO_STAGING,
    ITEM_FLUID_STAGING,
    ITEM_SPLIT_VIEW_STAGING,
};

QComboBoxGitViewOptions::QComboBoxGitViewOptions(QWidget *parent)
    : QComboBox(parent)
    , m_iconFlatListSingleColumn(":/QComboBoxGitViewOptions/flat_list_single_column")
    , m_iconFlatListMultipleColumn(":/QComboBoxGitViewOptions/flat_list_multiple_column")
    , m_iconTreeView(":/QComboBoxGitViewOptions/tree_view")
    , m_iconNoStaging(":/QComboBoxGitViewOptions/no_staging")
    , m_iconFluidStaging(":/QComboBoxGitViewOptions/fluid_staging")
    , m_iconSplitViewStaging(":/QComboBoxGitViewOptions/split_view_staging")
{
    QStandardItemModel *model = new QStandardItemModel();
    setModel(model);

    QStandardItem* item = nullptr;

    item = new QStandardItem(tr("Flat list (single column)"));
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    item->setCheckState(Qt::Checked);
    item->setIcon(m_iconFlatListSingleColumn);
    model->appendRow(item);

    item = new QStandardItem(tr("Flat list (multiple column)"));
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    item->setCheckState(Qt::Unchecked);
    item->setIcon(m_iconFlatListMultipleColumn);
    model->appendRow(item);

    item = new QStandardItem(tr("Tree view"));
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    item->setCheckState(Qt::Unchecked);
    item->setIcon(m_iconTreeView);
    model->appendRow(item);

    insertSeparator(model->rowCount());

    item = new QStandardItem(tr("No staging"));
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    item->setCheckState(Qt::Unchecked);
    item->setIcon(m_iconNoStaging);
    model->appendRow(item);

    item = new QStandardItem(tr("Fluid staging"));
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    item->setCheckState(Qt::Unchecked);
    item->setIcon(m_iconFluidStaging);
    model->appendRow(item);

    item = new QStandardItem(tr("Split view staging"));
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    item->setCheckState(Qt::Checked);
    item->setIcon(m_iconSplitViewStaging);
    model->appendRow(item);

    // TODO: Need to find better way to resize combobox popup window.
    view()->setMinimumWidth(view()->sizeHintForColumn(0));
    int height = 0;
    for(int r  = 0; r < model->rowCount(); r++)
    {
        height += view()->sizeHintForRow(r);
    }
    view()->setMinimumHeight(height);

    m_icon = m_iconFlatListSingleColumn;

    connect(this, &QComboBox::activated, this,  &QComboBoxGitViewOptions::activated);
}

QSize QComboBoxGitViewOptions::sizeHint() const
{
    return minimumSizeHint();
}

QSize QComboBoxGitViewOptions::minimumSizeHint() const
{
    QStyleOptionComboBox opt;
    opt.initFrom(this);

    QSize contentSize;

    opt.currentIcon = m_iconTreeView;
    opt.iconSize = iconSize();

// TODO: Periodicly check if we still need this Windows style fix.
#ifdef Q_OS_WIN
    contentSize = QSize(0, iconSize().width());
#else
    contentSize = iconSize();
#endif

    return style()->sizeFromContents(QStyle::CT_ComboBox, &opt, contentSize, this);
}

void QComboBoxGitViewOptions::paintEvent(QPaintEvent *event)
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

void QComboBoxGitViewOptions::activated(int index)
{
    QAbstractItemModel *items = model();

    if ((index >= ITEM_FLAT_LIST_SINGLE_COLUMN)&&(index <= ITEM_TREE_VIEW))
    {
        for(int c = ITEM_FLAT_LIST_SINGLE_COLUMN; c <= ITEM_TREE_VIEW; c++)
        {
            items->setData(items->index(c, 0), (c == index)?Qt::Checked:Qt::Unchecked, Qt::CheckStateRole);
        }
    }

    if ((index >= ITEM_NO_STAGING)&&(index <= ITEM_SPLIT_VIEW_STAGING))
    {
        for(int c = ITEM_NO_STAGING; c <= ITEM_SPLIT_VIEW_STAGING; c++)
        {
            items->setData(items->index(c, 0), (c == index)?Qt::Checked:Qt::Unchecked, Qt::CheckStateRole);
        }
    }

    if (index == 0) m_icon = m_iconFlatListSingleColumn;
    if (index == 1) m_icon = m_iconFlatListMultipleColumn;
    if (index == 2) m_icon = m_iconTreeView;
}
