#include "qgitremovesubmoduledialog.h"
#include "ui_qgitremovesubmoduledialog.h"
#include <QPushButton>

QGitRemoveSubmoduleDialog::QGitRemoveSubmoduleDialog(
    const QDir &repoPath,
    const QGitSubmodule &submodule,
    QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::QGitRemoveSubmoduleDialog)
    , m_repoPath(repoPath)
    , m_submodule(submodule)
{
    ui->setupUi(this);

    QPushButton *okBtn = ui->buttonBox->button(QDialogButtonBox::Ok);
    if (okBtn) {
        okBtn->setText(tr("Remove Submodule"));
    }

    ui->label_name_val->setText(m_submodule.name.isEmpty() ? tr("(unknown)") : m_submodule.name);
    ui->label_path_val->setText(m_submodule.path.isEmpty() ? tr("(unknown)") : m_submodule.path);
    ui->label_url_val->setText(m_submodule.url.isEmpty() ? tr("(none)") : m_submodule.url);

    QString headDisplay;
    if (!m_submodule.headId.isEmpty()) {
        headDisplay = m_submodule.headId;
    } else if (!m_submodule.indexId.isEmpty()) {
        headDisplay = m_submodule.indexId + tr(" (in index)");
    } else {
        headDisplay = tr("(uninitialized)");
    }
    ui->label_commit_val->setText(headDisplay);
}

QGitRemoveSubmoduleDialog::~QGitRemoveSubmoduleDialog()
{
    delete ui;
}

QString QGitRemoveSubmoduleDialog::submoduleName() const
{
    return m_submodule.name;
}

bool QGitRemoveSubmoduleDialog::removeWorkingDirectory() const
{
    return ui->checkBox_removeWorkingDir->isChecked();
}

bool QGitRemoveSubmoduleDialog::removeGitDir() const
{
    return ui->checkBox_removeGitDir->isChecked();
}

bool QGitRemoveSubmoduleDialog::force() const
{
    return ui->checkBox_force->isChecked();
}
