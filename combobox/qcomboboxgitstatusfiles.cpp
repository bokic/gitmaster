#include "qcomboboxgitstatusfiles.h"


QComboBoxGitStatusFiles::QComboBoxGitStatusFiles(QWidget *parent)
    : QCustomComboBox(parent)
{
    QListWidgetItem *item = nullptr;

    item = new QListWidgetItem(tr("Show only"));
    item->setFlags(Qt::NoItemFlags);
    m_list->addItem(item);

    item = new QListWidgetItem(tr("Pending"));
    item->setCheckState(Qt::Checked);
    m_list->addItem(item);

    item = new QListWidgetItem(tr("Conflicts"));
    item->setCheckState(Qt::Unchecked);
    m_list->addItem(item);

    item = new QListWidgetItem(tr("Untracked"));
    item->setCheckState(Qt::Unchecked);
    m_list->addItem(item);

    item = new QListWidgetItem(tr("Ignored"));
    item->setCheckState(Qt::Unchecked);
    m_list->addItem(item);

    item = new QListWidgetItem(tr("Clean"));
    item->setCheckState(Qt::Unchecked);
    m_list->addItem(item);

    item = new QListWidgetItem(tr("Modified"));
    item->setCheckState(Qt::Unchecked);
    m_list->addItem(item);

    item = new QListWidgetItem(tr("All"));
    item->setCheckState(Qt::Unchecked);
    m_list->addItem(item);

    item = new QListWidgetItem();
    item->setFlags(Qt::NoItemFlags);
    auto separator = new QFrame(m_list);
    separator->setFrameShape(QFrame::HLine);
    m_list->addItem(item);
    m_list->setItemWidget(item, separator);

    item = new QListWidgetItem(tr("Sort by"));
    item->setFlags(Qt::NoItemFlags);
    m_list->addItem(item);

    item = new QListWidgetItem(tr("Path aplhabetically"));
    item->setCheckState(Qt::Unchecked);
    m_list->addItem(item);

    item = new QListWidgetItem(tr("Path aplhabetically (reversed)"));
    item->setCheckState(Qt::Unchecked);
    m_list->addItem(item);

    item = new QListWidgetItem(tr("File name aplhabetically"));
    item->setCheckState(Qt::Unchecked);
    m_list->addItem(item);

    item = new QListWidgetItem(tr("File name aplhabetically (reversed)"));
    item->setCheckState(Qt::Unchecked);
    m_list->addItem(item);

    item = new QListWidgetItem(tr("File status"));
    item->setCheckState(Qt::Checked);
    m_list->addItem(item);

    item = new QListWidgetItem(tr("Checked / unchecked"));
    item->setCheckState(Qt::Unchecked);
    m_list->addItem(item);

    m_list->setMinimumWidth(m_list->sizeHintForColumn(0));

    updateText();
}

void QComboBoxGitStatusFiles::updateText()
{
    QString text, show, sort;
    int showFilesIdx, sortFilesIdx;

    showFilesIdx = showFiles();
    switch(showFilesIdx)
    {
    case 0:
        show = tr("Pending");
        break;
    case 1:
        show = tr("Conflicts");
        break;
    case 2:
        show = tr("Untracked");
        break;
    case 3:
        show = tr("Ignored");
        break;
    case 4:
        show = tr("Clean");
        break;
    case 5:
        show = tr("Modified");
        break;
    case 6:
        show = tr("All");
        break;
    default:
        show = tr("Unknown");
        break;
    }

    sortFilesIdx = showSortBy();
    switch(sortFilesIdx)
    {
    case 0:
        sort = tr("Path alphabetically");
        break;
    case 1:
        sort = tr("Path alphabetically (reversed)");
        break;
    case 2:
        sort = tr("File name alphabetically");
        break;
    case 3:
        sort = tr("File name alphabetically (reversed)");
        break;
    case 4:
        sort = tr("File status");
        break;
    case 5:
        sort = tr("Checked / unchecked");
        break;
    default:
        sort = tr("Unknown");
        break;
    }

    text = tr("%1 files, sorted by %2").arg(show, sort);

    setCurrentText(text);
}

int QComboBoxGitStatusFiles::showFiles() const
{
    const int offset = 1;
    const int len = 7;

    for(int c = offset; c < offset + len; c++)
        if (m_list->item(c)->checkState() == Qt::Checked)
            return c - offset;

    return -1;
}

int QComboBoxGitStatusFiles::showSortBy() const
{
    const int offset = 10;
    const int len = 6;

    for(int c = offset; c < offset + len; c++)
        if (m_list->item(c)->checkState() == Qt::Checked)
            return c - offset;

    return -1;
}

void QComboBoxGitStatusFiles::clicked(__attribute__((unused)) QListWidgetItem *item)
{
    updateText();
}
