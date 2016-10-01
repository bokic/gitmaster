#include "qgitclonerepositorydialog.h"
#include "ui_qgitclonerepositorydialog.h"

QGitCloneRepositoryDialog::QGitCloneRepositoryDialog(const QString &url, const QString &path, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::QGitCloneRepositoryDialog)
    , m_url(url)
    , m_path(path)
    , m_git(new QGit())
    , m_aborted(false)
{
    ui->setupUi(this);

    ui->label->setText(tr("Cloning <a href=\"%1\">%1</a> into <b>%2</b>").arg(url).arg(path));

    m_git->moveToThread(&m_thread);

    connect(this, SIGNAL(repositoryClone(QDir,QUrl)), m_git, SLOT(repositoryClone(QDir,QUrl)));
    connect(m_git, SIGNAL(repositoryCloneReply(QDir,int)), this, SLOT(repositoryCloneReply(QDir,int)));
    connect(m_git, SIGNAL(repositoryCloneTransferReply(uint,uint,uint,uint,uint,uint,size_t)), this, SLOT(repositoryCloneTransferReply(uint,uint,uint,uint,uint,uint,size_t)));
    connect(m_git, SIGNAL(repositoryCloneProgressReply(QDir,int,int)), this, SLOT(repositoryCloneProgressReply(QDir,int,int)));

    m_thread.start();

    emit repositoryClone(m_path, m_url);
}

QGitCloneRepositoryDialog::~QGitCloneRepositoryDialog()
{
    m_thread.quit();
    m_thread.wait();

    delete m_git; m_git = nullptr;

    delete ui;
}

void QGitCloneRepositoryDialog::on_pushButton_close_clicked()
{
    if (ui->pushButton_close->text() == tr("&Close"))
    {
        accept();
    }
    else
    {
        ui->pushButton_close->setText(tr("&Aborting"));
        ui->pushButton_close->setEnabled(false);

        m_git->abort();
        m_aborted = true;
    }
}

void QGitCloneRepositoryDialog::repositoryCloneReply(QDir path, int error)
{
    Q_UNUSED(path);
    Q_UNUSED(error);

    if (m_aborted)
    {
       reject();

       return;
    }

    ui->pushButton_close->setText(tr("&Close"));
    ui->pushButton_close->setEnabled(true);
}

void QGitCloneRepositoryDialog::repositoryCloneTransferReply(unsigned int total_objects, unsigned int indexed_objects, unsigned int received_objects, unsigned int local_objects, unsigned int total_deltas, unsigned int indexed_deltas, size_t received_bytes)
{
    Q_UNUSED(local_objects);

    if (total_objects > 0)
    {
        ui->progressBar_recievingObjects->setMaximum(total_objects);
        ui->progressBar_recievingObjects->setValue(received_objects);

        ui->progressBar_recievingIndexes->setMaximum(total_objects);
        ui->progressBar_recievingIndexes->setValue(indexed_objects);
    }

    if (total_deltas > 0)
    {
        ui->progressBar_indexingDeltas->setMaximum(total_deltas);
        ui->progressBar_indexingDeltas->setValue(indexed_deltas);
    }

    ui->label_status->setText(tr("Recieved %1 bytes.").arg(received_bytes));
}

void QGitCloneRepositoryDialog::repositoryCloneProgressReply(QDir path, int completed_steps, int total_steps)
{
    Q_UNUSED(path);

    ui->progressBar_creatingFiles->setMaximum(total_steps);
    ui->progressBar_creatingFiles->setValue(completed_steps);
}
