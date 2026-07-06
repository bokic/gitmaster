#include "qgitcreatetagdialog.h"
#include "ui_qgitcreatetagdialog.h"


QGitCreateTagDialog::QGitCreateTagDialog(const QString &commitHash, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::QGitCreateTagDialog)
{
    ui->setupUi(this);

    // Display the shortened commit hash in the form label
    ui->commitHash_label->setText(QString("<b>%1</b>").arg(commitHash.left(12)));
}

QGitCreateTagDialog::~QGitCreateTagDialog()
{
    delete ui;
}

QString QGitCreateTagDialog::tagName() const
{
    return ui->tagName_lineEdit->text().trimmed();
}

QString QGitCreateTagDialog::tagMessage() const
{
    if (ui->annotated_radioButton->isChecked()) {
        return ui->message_plainTextEdit->toPlainText().trimmed();
    }
    return QString();
}

bool QGitCreateTagDialog::isAnnotated() const
{
    return ui->annotated_radioButton->isChecked();
}

bool QGitCreateTagDialog::forceCreate() const
{
    return ui->force_checkBox->isChecked();
}

void QGitCreateTagDialog::on_tagName_lineEdit_textChanged(const QString &text)
{
    ui->createTag_pushButton->setEnabled(!text.trimmed().isEmpty());
}

void QGitCreateTagDialog::on_annotated_radioButton_toggled(bool checked)
{
    ui->message_groupBox->setEnabled(checked);
}
