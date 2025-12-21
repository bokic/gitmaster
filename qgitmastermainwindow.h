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
    void clearStatusBarText();

protected:
    bool event(QEvent *event) override;

private slots:
    void on_treeWidget_itemSelectionChanged();
    void on_toolButton_NewRepository_clicked();
    void on_toolButton_RemoveItem_clicked();

    void on_actionCommit_triggered();
    void on_actionFetch_triggered();
    void on_actionPull_triggered();
    void on_actionPush_triggered();

    void on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column);
    void on_tabWidget_tabCloseRequested(int index);
    void on_tabWidget_currentChanged(int index);
    void on_actionStash_triggered();

private:
    void readSettings();

    Ui::QGitMasterMainWindow *ui = nullptr;
    QString m_appTheme;
};
