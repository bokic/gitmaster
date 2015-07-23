#include "qgitrepository.h"
#include "ui_qgitrepository.h"

QGitRepository::QGitRepository(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::QGitRepository)
{
    ui->setupUi(this);
}

QGitRepository::~QGitRepository()
{
    delete ui;
}
