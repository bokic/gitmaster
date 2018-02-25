#include "qloghistorytablewidget.h"

QLogHistoryTableWidget::QLogHistoryTableWidget(QWidget *parent)
    : QTableWidget(parent)
{
    QPalette p = palette();
    p.setColor(QPalette::Inactive, QPalette::Highlight, p.color(QPalette::Active, QPalette::Highlight));
    p.setColor(QPalette::Active, QPalette::HighlightedText, p.color(QPalette::Inactive, QPalette::HighlightedText));
    setPalette(p);
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
        const QString &parentHash = commit.parents().at(0).commitHash();

        int parentIndex = m_hashIndex.indexOf(parentHash);
        if (parentIndex < 0)
        {
            m_hashIndex[index] = parentHash;
        }
        else
        {
            m_hashIndex.removeAt(index);
        }

        data.append(index);

        for(int c = 1; c < commit.parents().count(); c++)
        {
            //
        }
    }

    insertRow(row);

    QTableWidgetItem *item = nullptr;

    item = new QTableWidgetItem();
    if (row % 2) item->setData(Qt::BackgroundRole, QBrush(palette().window()));
    item->setData(Qt::UserRole, data);
    setItem(row, 0, item);

    item = new QTableWidgetItem(commit.message().split('\n').first());
    if (row % 2) item->setData(Qt::BackgroundRole, QBrush(palette().window()));
    item->setData(Qt::UserRole, commit.message());
    setItem(row, 1, item);

    item = new QTableWidgetItem(commit.time().toString());
    if (row % 2) item->setData(Qt::BackgroundRole, QBrush(palette().window()));
    setItem(row, 2, item);

    item = new QTableWidgetItem(QString("%1 <%2>").arg(commit.author().name(), commit.author().email()));
    if (row % 2) item->setData(Qt::BackgroundRole, QBrush(palette().window()));
    item->setData(Qt::UserRole, commit.author().name());
    item->setData(Qt::UserRole + 1, commit.author().email());
    setItem(row, 3, item);

    item = new QTableWidgetItem(commit.id().left(7));
    if (row % 2) item->setData(Qt::BackgroundRole, QBrush(palette().window()));
    item->setData(Qt::UserRole, commit.id());
    setItem(row, 4, item);
}
