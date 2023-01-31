#include "qcomboboxgitviewoptions.h"


QComboBoxGitViewOptions::QComboBoxGitViewOptions(QWidget *parent)
    : QCustomComboBox(parent)
{
    QListWidgetItem *item = nullptr;

    item = new QListWidgetItem(tr("Flat list (single column)"));
    item->setCheckState(Qt::Checked);
    item->setIcon(QIcon(":/QComboBoxGitViewOptions/flat_list_single_column"));
    m_list->addItem(item);

    item = new QListWidgetItem(tr("Flat list (multiple column)"));
    item->setCheckState(Qt::Unchecked);
    item->setIcon(QIcon(":/QComboBoxGitViewOptions/flat_list_multiple_column"));
    m_list->addItem(item);

    item = new QListWidgetItem(tr("Tree view"));
    item->setCheckState(Qt::Unchecked);
    item->setIcon(QIcon(":/QComboBoxGitViewOptions/tree_view"));
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
        setCurrentIcon(QIcon(":/QComboBoxGitViewOptions/flat_list_single_column"));
    else if (m_list->item(1)->checkState() == Qt::Checked)
        setCurrentIcon(QIcon(":/QComboBoxGitViewOptions/flat_list_multiple_column"));
    else if (m_list->item(2)->checkState() == Qt::Checked)
        setCurrentIcon(QIcon(":/QComboBoxGitViewOptions/tree_view"));
}

int QComboBoxGitViewOptions::viewType() const
{
    return -1;
}

int QComboBoxGitViewOptions::stagingType() const
{
    return -1;
}

void QComboBoxGitViewOptions::clicked(__attribute__((unused)) QListWidgetItem *item)
{
    updateIcon();
}
