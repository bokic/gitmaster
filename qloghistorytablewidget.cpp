#include "qloghistorytablewidget.h"

QLogHistoryTableWidget::QLogHistoryTableWidget(QWidget *parent)
    : QTableWidget(parent)
{

}

void QLogHistoryTableWidget::addCommit(const QGitCommit &commit)
{
    QString commitId = commit.id();
    int index = 0;
    int row = rowCount();    
    QList<QVariant> data;

    index = m_hashIndex.indexOf(commitId);

    if (index < 0)
    {
        m_hashIndex.append(commitId);
        index = m_hashIndex.length() - 1;
    }

    data.append(index);

    if (commit.parents().count() > 0)
    {
        m_hashIndex[index] = commit.parents().at(0).commitHash();
        data.append(index);

        for(int c = 1; c < commit.parents().count(); c++)
        {
            //
        }
    }

    insertRow(row);

    QTableWidgetItem *item = nullptr;

    item = new QTableWidgetItem();
    item->setData(Qt::UserRole, data);
    setItem(row, 0, item);

    item = new QTableWidgetItem(commit.message().split('\n').first());
    item->setData(Qt::UserRole, commit.message());
    setItem(row, 1, item);

    item = new QTableWidgetItem(commit.time().toString());
    setItem(row, 2, item);

    item = new QTableWidgetItem(QString("%1 <%2>").arg(commit.author().name(), commit.author().email()));
    setItem(row, 3, item);

    item = new QTableWidgetItem(commit.id().left(7));
    item->setData(Qt::UserRole, commit.id());
    setItem(row, 4, item);
}
