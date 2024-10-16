#include "qgitclonerepositorydialog.h"
#include "ui_qgitclonerepositorydialog.h"
#include <QShowEvent>


QGitCloneRepositoryDialog::QGitCloneRepositoryDialog(const QString &url, const QString &path, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::QGitCloneRepositoryDialog)
    , m_url(url)
    , m_path(path)
    , m_git(new QGit())
{
    ui->setupUi(this);

    ui->label->setText(tr("Cloning <a href=\"%1\">%1</a> into <b>%2</b>").arg(url, path));

    m_git->moveToThread(&m_thread);

    connect(this, &QGitCloneRepositoryDialog::clone, m_git, &QGit::clone);
    connect(m_git, &QGit::cloneReply, this, &QGitCloneRepositoryDialog::cloneReply);
    connect(m_git, &QGit::cloneTransferReply, this, &QGitCloneRepositoryDialog::cloneTransferReply);
    connect(m_git, &QGit::cloneProgressReply, this, &QGitCloneRepositoryDialog::cloneProgressReply);

	m_git->setPath(m_path);

    m_thread.start();
}

QGitCloneRepositoryDialog::~QGitCloneRepositoryDialog()
{
    m_thread.quit();
    m_thread.wait();

    delete m_git; m_git = nullptr;

    delete ui;
}

void QGitCloneRepositoryDialog::showEvent(QShowEvent *event)
{
    if (event->type() == QEvent::Show)
    {
        emit clone(m_url);
    }
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

        m_thread.requestInterruption();
    }
}

void QGitCloneRepositoryDialog::cloneReply(QGitError error)
{
    Q_UNUSED(error)

    if (m_thread.isRunning())
    {
       reject();

       return;
    }

    ui->pushButton_close->setText(tr("&Close"));
    ui->pushButton_close->setEnabled(true);
}

void QGitCloneRepositoryDialog::cloneTransferReply(unsigned int total_objects, unsigned int indexed_objects, unsigned int received_objects, unsigned int local_objects, unsigned int total_deltas, unsigned int indexed_deltas, size_t received_bytes)
{
    Q_UNUSED(local_objects)

    if (total_objects > 0)
    {
        ui->progressBar_recievingObjects->setMaximum(static_cast<int>(total_objects));
        ui->progressBar_recievingObjects->setValue(static_cast<int>(received_objects));

        ui->progressBar_recievingIndexes->setMaximum(static_cast<int>(total_objects));
        ui->progressBar_recievingIndexes->setValue(static_cast<int>(indexed_objects));
    }

    if (total_deltas > 0)
    {
        ui->progressBar_indexingDeltas->setMaximum(static_cast<int>(total_deltas));
        ui->progressBar_indexingDeltas->setValue(static_cast<int>(indexed_deltas));
    }

    ui->label_status->setText(tr("Recieved %1 bytes.").arg(received_bytes));
}

void QGitCloneRepositoryDialog::cloneProgressReply(QString path, size_t completed_steps, size_t total_steps)
{
    Q_UNUSED(path)

    ui->progressBar_creatingFiles->setMaximum(static_cast<int>(total_steps));
    ui->progressBar_creatingFiles->setValue(static_cast<int>(completed_steps));
}
