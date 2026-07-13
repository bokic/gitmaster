#pragma once

#include <QDialog>
#include <QString>


namespace Ui {
class QGitCreateTagDialog;
}

class QGitCreateTagDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QGitCreateTagDialog(const QString &commitHash, QWidget *parent = nullptr);
    ~QGitCreateTagDialog();

    QString tagName() const;
    QString tagMessage() const;
    bool isAnnotated() const;
    bool forceCreate() const;

private slots:
    void on_tagName_lineEdit_textChanged(const QString &text);
    void on_annotated_radioButton_toggled(bool checked);

private:
    Ui::QGitCreateTagDialog *ui = nullptr;
};
