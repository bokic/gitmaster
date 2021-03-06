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
    void on_actionNewFolderRepository_triggered();
    void on_actionDeleteRepository_triggered();
    void on_actionSettingsRepository_triggered();
    void on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column);
    void on_tabWidget_tabCloseRequested(int index);
    void on_tabWidget_currentChanged(int index);
    void on_actionStash_triggered();

private:
    void readSettings();

    Ui::QGitMasterMainWindow *ui;
};
