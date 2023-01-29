#pragma once

#include <QMainWindow>
#include <QTreeWidgetItem>
#include <QEvent>

namespace Ui {
class QGitMasterMainWindow;
}

class QGitMasterMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit QGitMasterMainWindow(QWidget *parent = nullptr);
    ~QGitMasterMainWindow();

    static QGitMasterMainWindow *instance();

    void writeSettings();
    bool hasRepositoryWithName(const QString &name);
    void updateStatusBarText(const QString &text);

protected:
    bool event(QEvent *event) override;

private slots:
    void on_actionCloneNew_triggered();
    void on_actionOpen_triggered();
    void on_actionOpen_Bookmarked_triggered();

    void on_actionUndo_triggered();
    void on_actionRedo_triggered();
    void on_actionCut_triggered();
    void on_actionCopy_triggered();
    void on_actionPaste_triggered();
    void on_actionSelect_All_triggered();

    void on_actionShow_Hide_Bookmarks_triggered();
    void on_actionShow_Hosted_Repositories_triggered();
    void on_actionRefresh_triggered();
    void on_actionClose_Tab_triggered();
    void on_actionNext_Tab_triggered();
    void on_actionPrevious_Tab_triggered();
    void on_actionFull_Screen_triggered();

    void on_actionNewFolderRepository_triggered();
    void on_actionDeleteRepository_triggered();
    void on_actionSettingsRepository_triggered();
    void on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column);
    void on_tabWidget_tabCloseRequested(int index);
    void on_tabWidget_currentChanged(int index);
    void on_actionStash_triggered();

    void on_themeChanged_triggered(bool checked);

private:
    void readSettings();

    Ui::QGitMasterMainWindow *ui = nullptr;
    Qt::WindowStates m_prevWindowState = Qt::WindowNoState;
    QString m_appTheme;
};
