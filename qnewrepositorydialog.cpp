#include "qnewrepositorydialog.h"
#include "ui_qnewrepositorydialog.h"

QNewRepositoryDialog::QNewRepositoryDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QNewRepositoryDialog)
{
    ui->setupUi(this);
}

QNewRepositoryDialog::~QNewRepositoryDialog()
{
    delete ui;
}
