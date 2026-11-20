#pragma once

#include <QDialog>
#include <QMap>
#include <QString>
#include <qgit.h>

namespace Ui {
class QGitSettingsDialog;
}

class QGitSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QGitSettingsDialog(QGit *git, QWidget *parent = nullptr);
    ~QGitSettingsDialog() override;

    bool configModified() const { return m_configModified; }

private slots:
    void on_pushButton_add_clicked();
    void on_pushButton_edit_clicked();
    void on_pushButton_delete_clicked();
    void on_lineEdit_search_textChanged(const QString &text);
    void on_radioButton_local_toggled(bool checked);
    void on_radioButton_global_toggled(bool checked);
    void accept() override;

private:
    void reloadScope(bool global);
    void populateTable();
    void updateAdvancedButtons();
    bool showConfigEntryDialog(const QString &title, QString &key, QString &value, bool keyEditable);

    Ui::QGitSettingsDialog *ui = nullptr;
    QGit *m_git = nullptr;
    QMap<QString, QString> m_allConfigs;
    bool m_configModified = false;
};
