#include "qcomboboxgitdiffoptions.h"


QComboBoxGitDiffOptions::QComboBoxGitDiffOptions(QWidget *parent)
    : QCustomComboBox(parent)
{
    QListWidgetItem *item = nullptr;
    QFrame *separator = nullptr;

    item = new QListWidgetItem(tr("External Diff"));
    m_list->addItem(item);

    item = new QListWidgetItem();
    item->setFlags(Qt::NoItemFlags);
    separator = new QFrame(m_list);
    separator->setFrameShape(QFrame::HLine);
    m_list->addItem(item);
    m_list->setItemWidget(item, separator);

    item = new QListWidgetItem(tr("Ignore whitespace"));
    item->setCheckState(Qt::Unchecked);
    m_list->addItem(item);

    item = new QListWidgetItem(tr("Show whitespace"));
    item->setCheckState(Qt::Checked);
    m_list->addItem(item);

    item = new QListWidgetItem();
    item->setFlags(Qt::NoItemFlags);
    separator = new QFrame(m_list);
    separator->setFrameShape(QFrame::HLine);
    m_list->addItem(item);
    m_list->setItemWidget(item, separator);

    item = new QListWidgetItem(tr("Lines of context"));
    item->setFlags(Qt::NoItemFlags);
    m_list->addItem(item);


    item = new QListWidgetItem(tr("1"));
    item->setCheckState(Qt::Unchecked);
    m_list->addItem(item);

    item = new QListWidgetItem(tr("3"));
    item->setCheckState(Qt::Checked);
    m_list->addItem(item);

    item = new QListWidgetItem(tr("6"));
    item->setCheckState(Qt::Unchecked);
    m_list->addItem(item);

    item = new QListWidgetItem(tr("12"));
    item->setCheckState(Qt::Unchecked);
    m_list->addItem(item);

    item = new QListWidgetItem(tr("25"));
    item->setCheckState(Qt::Unchecked);
    m_list->addItem(item);

    item = new QListWidgetItem(tr("50"));
    item->setCheckState(Qt::Unchecked);
    m_list->addItem(item);

    item = new QListWidgetItem(tr("100"));
    item->setCheckState(Qt::Unchecked);
    m_list->addItem(item);

    m_list->setMinimumWidth(m_list->sizeHintForColumn(0));

    setCurrentIcon(QIcon(":/QComboBoxGitDiffOptions/gear"));
}

void QComboBoxGitDiffOptions::clicked(__attribute__((unused)) QListWidgetItem *item)
{

}
