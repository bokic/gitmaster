// SPDX-License-Identifier: LGPL-3.0-or-later
#include "qgitsettingsdialog.h"
#include "ui_qgitsettingsdialog.h"
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QSet>

namespace {

class ElidedLabel : public QLabel
{
public:
    explicit ElidedLabel(const QString &text, QWidget *parent = nullptr)
        : QLabel(text, parent)
    {
        setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
        setToolTip(text);
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event)
        QPainter painter(this);
        painter.setPen(palette().color(QPalette::PlaceholderText));
        const QString elided = fontMetrics().elidedText(text(), Qt::ElideRight, width());
        painter.drawText(rect(), Qt::AlignLeft | Qt::AlignVCenter, elided);
    }
};

struct ConfigSpec {
    const char *key;
    QStringList options;
    bool editable;
    const char *description;
};

const QList<ConfigSpec> &knownConfigSpecs()
{
    static const QList<ConfigSpec> specs = {
        { "core.autocrlf",        { QStringLiteral("true"), QStringLiteral("false"), QStringLiteral("input") }, false,
          "Convert CRLF/LF line endings automatically on checkout and commit." },
        { "core.filemode",        { QStringLiteral("true"), QStringLiteral("false") }, false,
          "Track executable file permission changes in the working tree." },
        { "core.ignorecase",      { QStringLiteral("true"), QStringLiteral("false") }, false,
          "Treat file and directory names as case-insensitive." },
        { "core.symlinks",        { QStringLiteral("true"), QStringLiteral("false") }, false,
          "Create real symbolic links instead of plain text files on checkout." },
        { "core.longpaths",       { QStringLiteral("true"), QStringLiteral("false") }, false,
          "Allow paths longer than 260 characters on Windows." },
        { "core.editor",          { QStringLiteral("nano"), QStringLiteral("vim"), QStringLiteral("code --wait") }, true,
          "Default text editor used for commit messages and interactive rebase." },
        { "core.pager",           { QStringLiteral("less -FRX"), QStringLiteral("cat") }, true,
          "Pager command used by Git CLI to display long output." },
        { "init.defaultBranch",   { QStringLiteral("main"), QStringLiteral("master") }, true,
          "Default branch name created when initializing a new repository." },
        { "pull.rebase",          { QStringLiteral("false"), QStringLiteral("true"), QStringLiteral("merges"), QStringLiteral("interactive") }, false,
          "Rebase local commits on top of upstream instead of creating a merge commit on pull." },
        { "pull.ff",              { QStringLiteral("true"), QStringLiteral("false"), QStringLiteral("only") }, false,
          "Control fast-forward behavior when pulling changes." },
        { "push.default",         { QStringLiteral("simple"), QStringLiteral("current"), QStringLiteral("upstream"), QStringLiteral("matching"), QStringLiteral("nothing") }, false,
          "Which branches to push when no refspec is given on the command line." },
        { "push.autoSetupRemote", { QStringLiteral("true"), QStringLiteral("false") }, false,
          "Automatically set upstream tracking branch on first push." },
        { "push.followTags",      { QStringLiteral("true"), QStringLiteral("false") }, false,
          "Push annotated tags pointing to commits being pushed." },
        { "fetch.prune",          { QStringLiteral("true"), QStringLiteral("false") }, false,
          "Remove stale remote-tracking branches automatically when fetching." },
        { "fetch.pruneTags",      { QStringLiteral("true"), QStringLiteral("false") }, false,
          "Remove local tags that no longer exist on the remote when fetching." },
        { "merge.ff",             { QStringLiteral("true"), QStringLiteral("false"), QStringLiteral("only") }, false,
          "Allow, disallow, or require fast-forward merges." },
        { "merge.conflictStyle",  { QStringLiteral("merge"), QStringLiteral("diff3"), QStringLiteral("zdiff3") }, false,
          "Format used for conflict markers (diff3/zdiff3 also show common ancestor)." },
        { "rebase.autoStash",     { QStringLiteral("true"), QStringLiteral("false") }, false,
          "Automatically stash and re-apply dirty working tree changes during rebase." },
        { "rebase.autoSquash",    { QStringLiteral("true"), QStringLiteral("false") }, false,
          "Automatically reorder and apply fixup!/squash! commits in interactive rebase." },
        { "diff.algorithm",       { QStringLiteral("myers"), QStringLiteral("minimal"), QStringLiteral("patience"), QStringLiteral("histogram") }, false,
          "Algorithm used to compute line-by-line file diffs." },
        { "diff.renames",         { QStringLiteral("true"), QStringLiteral("false"), QStringLiteral("copies") }, false,
          "Detect renamed or copied files when generating diffs." },
        { "credential.helper",    { QStringLiteral("cache"), QStringLiteral("store"), QStringLiteral("manager"), QStringLiteral("osxkeychain"), QStringLiteral("libsecret") }, true,
          "Helper used to cache or store remote authentication credentials." },
        { "http.sslVerify",       { QStringLiteral("true"), QStringLiteral("false") }, false,
          "Verify SSL/TLS certificates when communicating over HTTPS." },
        { "rerere.enabled",       { QStringLiteral("true"), QStringLiteral("false") }, false,
          "Remember resolved merge conflicts and reuse them automatically." },
        { "submodule.recurse",    { QStringLiteral("true"), QStringLiteral("false") }, false,
          "Automatically update submodules during checkout, pull, and other commands." },
    };
    return specs;
}

} // namespace

QGitSettingsDialog::QGitSettingsDialog(QGit *git, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::QGitSettingsDialog)
    , m_git(git)
{
    ui->setupUi(this);

    bool isRepo = false;
    bool hasLocal = false;
    bool hasGlobal = false;
    if (m_git) {
        QGit::invokeSync(m_git, [&]() {
            isRepo = QGit::isGitRepository(m_git->path());
            auto hasAnyGeneralSetting = [&](bool global) {
                try {
                    return !m_git->configString("user.name", global).isEmpty()
                        || !m_git->configString("user.email", global).isEmpty()
                        || !m_git->configString("core.autocrlf", global).isEmpty();
                } catch (const QGitError &) {
                    return false;
                }
            };
            if (isRepo) {
                hasLocal = hasAnyGeneralSetting(false);
            }
            hasGlobal = hasAnyGeneralSetting(true);
        });
    }

    {
        const QSignalBlocker blockLocal(ui->radioButton_local);
        const QSignalBlocker blockGlobal(ui->radioButton_global);
        if (!isRepo) {
            ui->radioButton_local->setEnabled(false);
            ui->radioButton_global->setChecked(true);
        } else if (hasLocal && !hasGlobal) {
            ui->radioButton_local->setChecked(true);
        } else {
            ui->radioButton_global->setChecked(true);
        }
    }

    ui->gridLayout_advanced->setAlignment(Qt::AlignTop);
    ui->gridLayout_advanced->setColumnStretch(0, 0);
    ui->gridLayout_advanced->setColumnStretch(1, 0);
    ui->gridLayout_advanced->setColumnStretch(2, 1);
    ui->gridLayout_advanced->setHorizontalSpacing(10);
    ui->gridLayout_advanced->setVerticalSpacing(6);

    // Load configurations
    reloadScope(ui->radioButton_global->isChecked());
}

QGitSettingsDialog::~QGitSettingsDialog()
{
    delete ui;
}

void QGitSettingsDialog::reloadScope(bool global)
{
    if (!m_git) {
        m_allConfigs.clear();
        populateAdvancedForm();
        return;
    }

    try {
        QString userName, userEmail, autocrlf;
        QGit::invokeSync(m_git, [&]() {
            userName = m_git->configString("user.name", global);
            userEmail = m_git->configString("user.email", global);
            autocrlf = m_git->configString("core.autocrlf", global);
            m_allConfigs = m_git->configEntries(global);
        });
        ui->lineEdit_userName->setText(userName);
        ui->lineEdit_userEmail->setText(userEmail);
        int index = ui->comboBox_autoCrlf->findText(autocrlf);
        ui->comboBox_autoCrlf->setCurrentIndex(index >= 0 ? index : 0);
    } catch (const QGitError &error) {
        m_allConfigs.clear();
        qWarning("QGitSettingsDialog::reloadScope: %s", qPrintable(error.errorString()));
    }
    populateAdvancedForm();
}

void QGitSettingsDialog::populateAdvancedForm()
{
    m_populatingAdvanced = true;

    for (const auto &row : std::as_const(m_advancedRows)) {
        ui->gridLayout_advanced->removeWidget(row.label);
        ui->gridLayout_advanced->removeWidget(row.combo);
        ui->gridLayout_advanced->removeWidget(row.description);
        delete row.label;
        delete row.combo;
        delete row.description;
    }
    m_advancedRows.clear();

    QSet<QString> addedKeys;
    int gridRow = 0;

    auto addRow = [&](const QString &key, const QStringList &options, bool editable, const QString &descriptionText) {
        auto *label = new QLabel(key, ui->scrollAreaWidgetContents_advanced);
        auto *combo = new QComboBox(ui->scrollAreaWidgetContents_advanced);
        combo->setEditable(editable);
        combo->setMinimumWidth(140);

        auto *descLabel = new ElidedLabel(descriptionText, ui->scrollAreaWidgetContents_advanced);

        combo->addItem(tr("<None>"), QVariant(true));
        for (const QString &opt : options) {
            combo->addItem(opt, QVariant(false));
        }

        if (m_allConfigs.contains(key)) {
            const QString currentVal = m_allConfigs.value(key);
            int idx = combo->findText(currentVal);
            if (idx < 1) {
                combo->addItem(currentVal, QVariant(false));
                idx = combo->count() - 1;
            }
            combo->setCurrentIndex(idx);
        } else {
            combo->setCurrentIndex(0);
        }

        connect(combo, QOverload<int>::of(&QComboBox::activated), this, [this, key, combo](int index) {
            if (m_populatingAdvanced) return;
            const bool isNone = combo->itemData(index, Qt::UserRole).toBool();
            applyAdvancedEntry(key, isNone ? QString() : combo->itemText(index), isNone);
        });

        if (editable && combo->lineEdit()) {
            connect(combo->lineEdit(), &QLineEdit::editingFinished, this, [this, key, combo]() {
                if (m_populatingAdvanced) return;
                const QString text = combo->currentText();
                if (combo->currentIndex() == 0 && combo->itemData(0, Qt::UserRole).toBool() && text == combo->itemText(0)) {
                    applyAdvancedEntry(key, QString(), true);
                } else if (!text.isEmpty() && m_allConfigs.value(key) != text) {
                    applyAdvancedEntry(key, text, false);
                }
            });
        }

        ui->gridLayout_advanced->addWidget(label, gridRow, 0);
        ui->gridLayout_advanced->addWidget(combo, gridRow, 1);
        ui->gridLayout_advanced->addWidget(descLabel, gridRow, 2);
        ++gridRow;

        m_advancedRows.append({ key, label, combo, descLabel });
        addedKeys.insert(key);
    };

    for (const auto &spec : knownConfigSpecs()) {
        addRow(QString::fromLatin1(spec.key), spec.options, spec.editable, tr(spec.description));
    }

    for (auto it = m_allConfigs.cbegin(); it != m_allConfigs.cend(); ++it) {
        if (!addedKeys.contains(it.key())) {
            addRow(it.key(), {}, true, tr("Custom repository/user configuration entry."));
        }
    }

    m_populatingAdvanced = false;
    filterAdvancedForm();
}

void QGitSettingsDialog::filterAdvancedForm()
{
    const QString filter = ui->lineEdit_search->text().trimmed();
    for (const auto &row : std::as_const(m_advancedRows)) {
        const bool visible = filter.isEmpty()
            || row.key.contains(filter, Qt::CaseInsensitive)
            || row.combo->currentText().contains(filter, Qt::CaseInsensitive)
            || row.description->text().contains(filter, Qt::CaseInsensitive);
        row.label->setVisible(visible);
        row.combo->setVisible(visible);
        row.description->setVisible(visible);
    }
}

void QGitSettingsDialog::applyAdvancedEntry(const QString &key, const QString &value, bool isNone)
{
    if (!m_git) return;

    const bool globalWrite = ui->radioButton_global->isChecked();
    try {
        if (isNone) {
            if (m_allConfigs.contains(key)) {
                QGit::invokeSync(m_git, [&]() {
                    m_git->deleteConfigEntry(key, globalWrite);
                });
                m_allConfigs.remove(key);
                m_configModified = true;
            }
        } else {
            QGit::invokeSync(m_git, [&]() {
                m_git->setConfigString(key, value, globalWrite);
            });
            m_allConfigs.insert(key, value);
            m_configModified = true;
        }

        if (key == QLatin1String("user.name")) {
            ui->lineEdit_userName->setText(isNone ? QString() : value);
        } else if (key == QLatin1String("user.email")) {
            ui->lineEdit_userEmail->setText(isNone ? QString() : value);
        } else if (key == QLatin1String("core.autocrlf")) {
            int idx = isNone ? 0 : ui->comboBox_autoCrlf->findText(value);
            ui->comboBox_autoCrlf->setCurrentIndex(idx >= 0 ? idx : 0);
        }
    } catch (const QGitError &error) {
        QMessageBox::critical(this, tr("Error"), error.errorString());
        reloadScope(globalWrite);
    }
}

void QGitSettingsDialog::on_pushButton_add_clicked()
{
    if (!m_git) return;

    QDialog dlg(this);
    dlg.setWindowTitle(tr("Add Custom Configuration"));
    dlg.setMinimumWidth(380);

    auto *layout = new QFormLayout(&dlg);
    auto *keyEdit = new QLineEdit(&dlg);
    keyEdit->setPlaceholderText(tr("e.g., core.autocrlf"));
    auto *valEdit = new QLineEdit(&dlg);

    layout->addRow(tr("Key:"), keyEdit);
    layout->addRow(tr("Value:"), valEdit);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    layout->addRow(buttons);

    auto *okBtn = buttons->button(QDialogButtonBox::Ok);
    okBtn->setEnabled(false);
    connect(keyEdit, &QLineEdit::textChanged, &dlg, [&](const QString &t) {
        okBtn->setEnabled(!t.trimmed().isEmpty());
    });
    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    keyEdit->setFocus();

    if (dlg.exec() != QDialog::Accepted)
        return;

    const QString key = keyEdit->text().trimmed();
    const QString val = valEdit->text();
    const bool globalWrite = ui->radioButton_global->isChecked();
    try {
        QGit::invokeSync(m_git, [&]() {
            m_git->setConfigString(key, val, globalWrite);
        });
        m_configModified = true;
        reloadScope(globalWrite);
    } catch (const QGitError &error) {
        QMessageBox::critical(this, tr("Error"), error.errorString());
    }
}

void QGitSettingsDialog::on_lineEdit_search_textChanged(const QString &text)
{
    Q_UNUSED(text);
    filterAdvancedForm();
}

void QGitSettingsDialog::on_radioButton_local_toggled(bool checked)
{
    if (!checked) return;
    reloadScope(false);
}

void QGitSettingsDialog::on_radioButton_global_toggled(bool checked)
{
    if (!checked) return;
    reloadScope(true);
}

void QGitSettingsDialog::accept()
{
    if (!m_git) {
        QDialog::accept();
        return;
    }

    QString name = ui->lineEdit_userName->text().trimmed();
    QString email = ui->lineEdit_userEmail->text().trimmed();
    QString autocrlf = ui->comboBox_autoCrlf->currentText();

    bool globalWrite = ui->radioButton_global->isChecked();

    try {
        QGit::invokeSync(m_git, [&]() {
            auto applyConfig = [&](const QString &key, const QString &value) {
                if (value.isEmpty()) {
                    m_git->deleteConfigEntry(key, globalWrite);
                } else {
                    m_git->setConfigString(key, value, globalWrite);
                }
            };
            applyConfig("user.name", name);
            applyConfig("user.email", email);
            applyConfig("core.autocrlf", autocrlf);
        });
        m_configModified = true;
        QDialog::accept();
    } catch (const QGitError &error) {
        QMessageBox::critical(this, tr("Save Error"), error.errorString());
    }
}
