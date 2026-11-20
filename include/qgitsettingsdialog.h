#pragma once

#include <QDialog>
#include <QList>
#include <QMap>
#include <QString>
#include <qgit.h>

class QComboBox;
class QLabel;

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
    void on_lineEdit_search_textChanged(const QString &text);
    void on_radioButton_local_toggled(bool checked);
    void on_radioButton_global_toggled(bool checked);
    void accept() override;

private:
    struct AdvancedRow {
        QString key;
        QLabel *label = nullptr;
        QComboBox *combo = nullptr;
        QLabel *description = nullptr;
    };

    void reloadScope(bool global);
    void populateAdvancedForm();
    void filterAdvancedForm();
    void applyAdvancedEntry(const QString &key, const QString &value, bool isNone);

    Ui::QGitSettingsDialog *ui = nullptr;
    QGit *m_git = nullptr;
    QMap<QString, QString> m_allConfigs;
    QList<AdvancedRow> m_advancedRows;
    bool m_configModified = false;
    bool m_populatingAdvanced = false;
};
