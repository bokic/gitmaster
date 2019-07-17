#include "qcomboboxgitstatusfiles.h"


#define ITEM_SHOW_ONLY         0
#define ITEM_PENDING           1
#define ITEM_CONFLICTS         2
#define ITEM_UNTRACKED         3
#define ITEM_IGNORED           4
#define ITEM_CLEAN             5
#define ITEM_MODIFIED          6
#define ITEM_ALL               7
#define ITEM_SEPARATOR         8
#define ITEM_SORT_BY           9
#define ITEM_PATH_APLHA        10
#define ITEM_PATH_APLHA_REV    11
#define ITEM_FILE_APLHA        12
#define ITEM_FILE_APLHA_REV    13
#define ITEM_FILE_STATUS       14
#define ITEM_CHECKED_UNCHECHED 15

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
    case QComboBoxGitStatusFiles::QShowPendingFiles:
        show = tr("Pending");
        break;
    case QComboBoxGitStatusFiles::QShowConflictFiles:
        show = tr("Conflicts");
        break;
    case QComboBoxGitStatusFiles::QShowUntracked:
        show = tr("Untracked");
        break;
    case QComboBoxGitStatusFiles::QShowIgnored:
        show = tr("Ignored");
        break;
    case QComboBoxGitStatusFiles::QShowClean:
        show = tr("Clean");
        break;
    case QComboBoxGitStatusFiles::QShowModified:
        show = tr("Modified");
        break;
    case QComboBoxGitStatusFiles::QShowAll:
        show = tr("All");
        break;
    default:
        show = tr("Unknown");
        break;
    }

    sortFilesIdx = showSortBy();
    switch(sortFilesIdx)
    {
    case QComboBoxGitStatusFiles::QFilePathSortFiles:
        sort = tr("Path alphabetically");
        break;
    case QComboBoxGitStatusFiles::QReversedFilePathSortFiles:
        sort = tr("Path alphabetically (reversed)");
        break;
    case QComboBoxGitStatusFiles::QFileNameSortFiles:
        sort = tr("File name alphabetically");
        break;
    case QComboBoxGitStatusFiles::QReversedFileNameSortFiles:
        sort = tr("File name alphabetically (reversed)");
        break;
    case QComboBoxGitStatusFiles::QFileStatusSortFiles:
        sort = tr("File status");
        break;
    case QComboBoxGitStatusFiles::QCheckedUncheckedSortFiles:
        sort = tr("Checked / unchecked");
        break;
    default:
        sort = tr("Unknown");
        break;
    }

    text = tr("%1 files, sorted by %2").arg(show, sort);

    setCurrentText(text);
}

QComboBoxGitStatusFiles::QComboBoxGitStatusFilesShowFiles QComboBoxGitStatusFiles::showFiles() const
{
    if (m_list->item(ITEM_PENDING)->checkState() == Qt::Checked) {
        return QShowPendingFiles;
    } else if (m_list->item(ITEM_CONFLICTS)->checkState() == Qt::Checked) {
        return QShowConflictFiles;
    } else if (m_list->item(ITEM_UNTRACKED)->checkState() == Qt::Checked) {
        return QShowUntracked;
    } else if (m_list->item(ITEM_IGNORED)->checkState() == Qt::Checked) {
        return QShowIgnored;
    } else if (m_list->item(ITEM_CLEAN)->checkState() == Qt::Checked) {
        return QShowClean;
    } else if (m_list->item(ITEM_MODIFIED)->checkState() == Qt::Checked) {
        return QShowModified;
    } else if (m_list->item(ITEM_ALL)->checkState() == Qt::Checked) {
        return QShowAll;
    }

    return QShowAll;
}

QComboBoxGitStatusFiles::QComboBoxGitStatusFilesOrderFiles QComboBoxGitStatusFiles::showSortBy() const
{
    if (m_list->item(ITEM_PATH_APLHA)->checkState() == Qt::Checked) {
        return QFilePathSortFiles;
    } else if (m_list->item(ITEM_PATH_APLHA_REV)->checkState() == Qt::Checked) {
        return QReversedFilePathSortFiles;
    } else if (m_list->item(ITEM_FILE_APLHA)->checkState() == Qt::Checked) {
        return QFileNameSortFiles;
    } else if (m_list->item(ITEM_FILE_APLHA_REV)->checkState() == Qt::Checked) {
        return QReversedFileNameSortFiles;
    } else if (m_list->item(ITEM_FILE_STATUS)->checkState() == Qt::Checked) {
        return QFileStatusSortFiles;
    } else if (m_list->item(ITEM_CHECKED_UNCHECHED)->checkState() == Qt::Checked) {
        return QCheckedUncheckedSortFiles;
    }

    return QUnsortedFiles;
}

void QComboBoxGitStatusFiles::clicked(__attribute__((unused)) QListWidgetItem *item)
{
    updateText();
}
