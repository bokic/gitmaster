#pragma once

#include <QTreeWidget>
#include <QIcon>
#include <QList>
#include <utility>
#include <QString>
#include <git2.h>

class QGitFileStatusTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    explicit QGitFileStatusTreeWidget(QWidget *parent = nullptr);
    ~QGitFileStatusTreeWidget() = default;

    enum ViewLayoutMode {
        FlatSingle = 0,
        FlatMulti = 1,
        TreeView = 2
    };

    enum class FileCategory : uint8_t {
        Unstaged,
        Staged,
        Pending
    };

    void setViewLayoutMode(ViewLayoutMode mode);
    ViewLayoutMode viewLayoutMode() const { return m_layoutMode; }

    static QIcon getFileIcon(git_status_t status, bool isStaged);
    static QString getStatusText(git_status_t status);

    void populateFiles(
        const QList<std::pair<QString, git_status_t>> &files,
        FileCategory category);

    void updateFolderCheckStates(QTreeWidgetItem *item);

private:
    ViewLayoutMode m_layoutMode = FlatSingle;
};
