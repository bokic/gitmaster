#pragma once

#include <QTreeWidget>
#include <QStyledItemDelegate>
#include <QLineEdit>
#include <QIcon>
#include "qgit.h"
#include "qgitbranch.h"
#include "qgittag.h"

class QGitBranchTreeItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit QGitBranchTreeItemDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QWidget *editor = QStyledItemDelegate::createEditor(parent, option, index);
        QLineEdit *lineEdit = qobject_cast<QLineEdit *>(editor);
        if (lineEdit) {
            QString text = lineEdit->text();
            int suffixIndex = text.indexOf(QStringLiteral(" [↑"));
            if (suffixIndex == -1) {
                suffixIndex = text.indexOf(QStringLiteral(" [↓"));
            }
            if (suffixIndex != -1) {
                lineEdit->setText(text.left(suffixIndex).trimmed());
            }
        }
        return editor;
    }
};

class QGitBranchTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    explicit QGitBranchTreeWidget(QWidget *parent = nullptr);
    ~QGitBranchTreeWidget() = default;

    void populateBranchesAndTags(
        const QList<QGitBranch> &branches,
        const QList<QGitTag> &tags,
        const QList<QGitSubmodule> &submodules,
        const QList<QGitWorktree> &worktrees,
        const QString &currentBranch);

    void populateStashes(const QStringList &stashes);

private:
    QIcon m_iconTag;
    QIcon m_iconWhiteCheckbox;
    QIcon m_iconCurrentBranch;
    QIcon m_iconBranch;
    QIcon m_iconRemote;
    QIcon m_iconStash;
    QIcon m_iconRemoteBranch;
    QIcon m_iconSubmodule;
    QIcon m_iconWorktree;
};
