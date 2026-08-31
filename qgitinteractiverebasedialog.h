#pragma once

#include <QDialog>
#include <QList>
#include <QString>
#include "qgit.h"
#include "qgitrebasetodoitem.h"

namespace Ui {
class QGitInteractiveRebaseDialog;
}

class QComboBox;

class QGitInteractiveRebaseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QGitInteractiveRebaseDialog(QGit *git, const QString &baseCommitId, QWidget *parent = nullptr);
    ~QGitInteractiveRebaseDialog() override;

    QList<QGitRebaseTodoItem> todoItems() const;
    QString baseCommitId() const;

private slots:
    void on_tableWidget_itemSelectionChanged();
    void on_tableWidget_customContextMenuRequested(const QPoint &pos);
    void on_plainTextEdit_message_textChanged();
    void on_toolButton_pick_clicked();
    void on_toolButton_reword_clicked();
    void on_toolButton_squash_clicked();
    void on_toolButton_fixup_clicked();
    void on_toolButton_drop_clicked();
    void on_toolButton_moveUp_clicked();
    void on_toolButton_moveDown_clicked();
    void on_toolButton_reset_clicked();
    void on_pushButton_rebase_clicked();
    void on_pushButton_cancel_clicked();
    void onActionComboChanged(int index);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    void setupUiCustomizations();
    void loadCommits();
    void populateTable();
    void updateRowAppearance(int row);
    void setActionForSelectedRows(QGitRebaseAction action);
    void moveSelectedRows(int direction); // -1 = up, +1 = down
    void saveCurrentMessage();
    void loadMessageForRow(int row);
    bool validateRebase(QString &errorMessage) const;
    void updateValidationStatus();
    static QString actionToString(QGitRebaseAction action);
    static QString actionToolTip(QGitRebaseAction action);

    Ui::QGitInteractiveRebaseDialog *ui;
    QGit *m_git = nullptr;
    QString m_baseCommitId;
    QString m_branchName;
    QList<QGitRebaseTodoItem> m_items;
    QList<QGitRebaseTodoItem> m_originalItems;
    int m_currentEditingRow = -1;
    bool m_updatingUi = false;
};
