#include "qcomboboxgitviewoptions.h"


QComboBoxGitViewOptions::QComboBoxGitViewOptions(QWidget *parent)
    : QCustomComboBox(parent)
    , m_iconFlatListSingleColumn(":/QComboBoxGitViewOptions/flat_list_single_column")
    , m_iconFlatListMultipleColumn(":/QComboBoxGitViewOptions/flat_list_multiple_column")
    , m_iconTreeView(":/QComboBoxGitViewOptions/tree_view")
{
    QListWidgetItem *item = nullptr;

    item = new QListWidgetItem(tr("Flat list (single column)"));
    item->setCheckState(Qt::Checked);
    item->setIcon(m_iconFlatListSingleColumn);
    m_list->addItem(item);

    item = new QListWidgetItem(tr("Flat list (multiple column)"));
    item->setCheckState(Qt::Unchecked);
    item->setIcon(m_iconFlatListMultipleColumn);
    m_list->addItem(item);

    item = new QListWidgetItem(tr("Tree view"));
    item->setCheckState(Qt::Unchecked);
    item->setIcon(m_iconTreeView);
    m_list->addItem(item);

    item = new QListWidgetItem();
    item->setFlags(Qt::NoItemFlags);
    auto separator = new QFrame(m_list);
    separator->setFrameShape(QFrame::HLine);
    m_list->addItem(item);
    m_list->setItemWidget(item, separator);

    item = new QListWidgetItem(tr("No staging"));
    item->setCheckState(Qt::Unchecked);
    item->setIcon(QIcon(":/QComboBoxGitViewOptions/no_staging"));
    m_list->addItem(item);

    item = new QListWidgetItem(tr("Fluid staging"));
    item->setCheckState(Qt::Unchecked);
    item->setIcon(QIcon(":/QComboBoxGitViewOptions/fluid_staging"));
    m_list->addItem(item);

    item = new QListWidgetItem(tr("Split view staging"));
    item->setCheckState(Qt::Checked);
    item->setIcon(QIcon(":/QComboBoxGitViewOptions/split_view_staging"));
    m_list->addItem(item);

    m_list->setMinimumWidth(m_list->sizeHintForColumn(0));

    updateIcon();
}

void QComboBoxGitViewOptions::updateIcon()
{
    if (m_list->item(0)->checkState() == Qt::Checked)
        setCurrentIcon(m_iconFlatListSingleColumn);
    else if (m_list->item(1)->checkState() == Qt::Checked)
        setCurrentIcon(m_iconFlatListMultipleColumn);
    else if (m_list->item(2)->checkState() == Qt::Checked)
        setCurrentIcon(m_iconTreeView);
}

int QComboBoxGitViewOptions::viewType() const
{
    return -1;
}

int QComboBoxGitViewOptions::stagingType() const
{
    return -1;
}

void QComboBoxGitViewOptions::clicked(QListWidgetItem *item)
{
    Q_UNUSED(item);

    updateIcon();
}
