#include "qcomboboxgitstatusfiles.h"

#include <QStandardItemModel>
#include <QAbstractItemView>
#include <QStylePainter>
#include <QApplication>
#include <QSvgRenderer>
#include <QFile>


enum {
    ITEM_SHOW_ONLY,
    ITEM_PENDING,
    ITEM_CONFLICTS,
    ITEM_UNTRACKED,
    ITEM_IGNORED,
    ITEM_CLEAN,
    ITEM_MODIFIED,
    ITEM_ALL,
    ITEM_SEPARATOR,
    ITEM_SORT_BY,
    ITEM_PATH_APLHA,
    ITEM_PATH_APLHA_REV,
    ITEM_FILE_APLHA,
    ITEM_FILE_APLHA_REV,
    ITEM_FILE_STATUS,
    ITEM_CHECKED_UNCHECHED,
};

QComboBoxGitStatusFiles::QComboBoxGitStatusFiles(QWidget *parent)
    : QComboBoxGitBase(parent)
    , m_iconChecked(":/QCustomComboBox/check")
    , m_iconUnchecked(":/QCustomComboBox/uncheck")
{
    initStandardModel();
    updateIconColor();

    addHeaderItem(tr("Show only"), m_iconUnchecked);
    addOptionItem(tr("Pending"), true, Qt::Checked, m_iconChecked);
    addOptionItem(tr("Conflicts"), true, Qt::Unchecked, m_iconUnchecked);
    addOptionItem(tr("Untracked"), true, Qt::Unchecked, m_iconUnchecked);
    addOptionItem(tr("Ignored"), true, Qt::Unchecked, m_iconUnchecked);
    addOptionItem(tr("Clean"), true, Qt::Unchecked, m_iconUnchecked);
    addOptionItem(tr("Modified"), true, Qt::Unchecked, m_iconUnchecked);
    addOptionItem(tr("All"), true, Qt::Unchecked, m_iconUnchecked);

    insertSeparator(count());

    addHeaderItem(tr("Sort by"), m_iconUnchecked);
    addOptionItem(tr("Path alphabetically"), true, Qt::Unchecked, m_iconUnchecked);
    addOptionItem(tr("Path alphabetically (reversed)"), true, Qt::Unchecked, m_iconUnchecked);
    addOptionItem(tr("File name alphabetically"), true, Qt::Unchecked, m_iconUnchecked);
    addOptionItem(tr("File name alphabetically (reversed)"), true, Qt::Unchecked, m_iconUnchecked);
    addOptionItem(tr("File status"), true, Qt::Checked, m_iconChecked);
    addOptionItem(tr("Checked / unchecked"), true, Qt::Unchecked, m_iconUnchecked);

    view()->setMinimumWidth(view()->sizeHintForColumn(0));

    connect(this, &QComboBox::activated, this, &QComboBoxGitStatusFiles::activated);

    updateText();
}

void QComboBoxGitStatusFiles::updateText()
{
    QString show, sort;
    int showFilesIdx;
    int sortFilesIdx;

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

    m_text = tr("%1 files, sorted by %2").arg(show, sort);
    m_displayText = m_text;
    updateGeometry();
    update();
}

QComboBoxGitStatusFiles::QComboBoxGitStatusFilesShowFiles QComboBoxGitStatusFiles::showFiles() const
{
    QAbstractItemModel *items = model();

    if (items->index(ITEM_PENDING, 0).data(Qt::CheckStateRole) == Qt::Checked) {
        return QShowPendingFiles;
    }
    if (items->index(ITEM_CONFLICTS, 0).data(Qt::CheckStateRole) == Qt::Checked) {
        return QShowConflictFiles;
    }
    if (items->index(ITEM_UNTRACKED, 0).data(Qt::CheckStateRole) == Qt::Checked) {
        return QShowUntracked;
    }
    if (items->index(ITEM_IGNORED, 0).data(Qt::CheckStateRole) == Qt::Checked) {
        return QShowIgnored;
    }
    if (items->index(ITEM_CLEAN, 0).data(Qt::CheckStateRole) == Qt::Checked) {
        return QShowClean;
    }
    if (items->index(ITEM_MODIFIED, 0).data(Qt::CheckStateRole) == Qt::Checked) {
        return QShowModified;
    }
    if (items->index(ITEM_ALL, 0).data(Qt::CheckStateRole) == Qt::Checked) {
        return QShowAll;
    }

    return QShowAll;
}

QComboBoxGitStatusFiles::QComboBoxGitStatusFilesOrderFiles QComboBoxGitStatusFiles::showSortBy() const
{
    QAbstractItemModel *items = model();

    if (items->index(ITEM_PATH_APLHA, 0).data(Qt::CheckStateRole) == Qt::Checked) {
        return QFilePathSortFiles;
    }
    if (items->index(ITEM_PATH_APLHA_REV, 0).data(Qt::CheckStateRole) == Qt::Checked) {
        return QReversedFilePathSortFiles;
    }
    if (items->index(ITEM_FILE_APLHA, 0).data(Qt::CheckStateRole) == Qt::Checked) {
        return QFileNameSortFiles;
    }
    if (items->index(ITEM_FILE_APLHA_REV, 0).data(Qt::CheckStateRole) == Qt::Checked) {
        return QReversedFileNameSortFiles;
    }
    if (items->index(ITEM_FILE_STATUS, 0).data(Qt::CheckStateRole) == Qt::Checked) {
        return QFileStatusSortFiles;
    }
    if (items->index(ITEM_CHECKED_UNCHECHED, 0).data(Qt::CheckStateRole) == Qt::Checked) {
        return QCheckedUncheckedSortFiles;
    }

    return QUnsortedFiles;
}

void QComboBoxGitStatusFiles::activated(int index)
{
    QStandardItemModel *items = qobject_cast<QStandardItemModel *>(model());
    if (!items) return;

    if ((index >= ITEM_SHOW_ONLY)&&(index <= ITEM_ALL))
    {
        for(int c = ITEM_SHOW_ONLY; c <= ITEM_ALL; c++)
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

    if ((index >= ITEM_SORT_BY)&&(index <= ITEM_CHECKED_UNCHECHED))
    {
        for(int c = ITEM_SORT_BY; c <= ITEM_CHECKED_UNCHECHED; c++)
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

    updateText();

    emit itemClicked(index);
}

void QComboBoxGitStatusFiles::updateIconColor()
{
    auto resource = QFile(":/QCustomComboBox/check");
    if (resource.open(QIODeviceBase::ReadOnly))
    {
        auto svgContent = resource.readAll();

        QString newColor = palette().color(QPalette::Text).name();
        svgContent.replace("#000000", newColor.toUtf8());

        QSvgRenderer renderer(svgContent);
        if (renderer.isValid()) {
            QImage image(iconSize(), QImage::Format_ARGB32_Premultiplied);
            image.fill(0);
            QPainter painter(&image);
            renderer.render(&painter);
            m_iconChecked = QPixmap::fromImage(image);
            update();
        }
    }
}
