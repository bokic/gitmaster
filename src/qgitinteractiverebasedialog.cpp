#include "qgitinteractiverebasedialog.h"
#include "ui_qgitinteractiverebasedialog.h"
#include <QComboBox>
#include <QMenu>
#include <QMessageBox>
#include <QKeyEvent>
#include <QFont>
#include <QColor>

QGitInteractiveRebaseDialog::QGitInteractiveRebaseDialog(QGit *git, const QString &baseCommitId, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::QGitInteractiveRebaseDialog)
    , m_git(git)
    , m_baseCommitId(baseCommitId)
{
    ui->setupUi(this);
    setupUiCustomizations();
    loadCommits();
}

QGitInteractiveRebaseDialog::~QGitInteractiveRebaseDialog()
{
    delete ui;
}

void QGitInteractiveRebaseDialog::setupUiCustomizations()
{
    ui->tableWidget->setColumnCount(5);
    ui->tableWidget->setHorizontalHeaderLabels({
        tr("Action"),
        tr("Commit"),
        tr("Subject"),
        tr("Author"),
        tr("Date")
    });

    ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);

    ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);

    ui->splitter->setStretchFactor(0, 3);
    ui->splitter->setStretchFactor(1, 2);

    connect(ui->toolButton_pick, &QToolButton::clicked, this, &QGitInteractiveRebaseDialog::on_toolButton_pick_clicked);
    connect(ui->toolButton_reword, &QToolButton::clicked, this, &QGitInteractiveRebaseDialog::on_toolButton_reword_clicked);
    connect(ui->toolButton_squash, &QToolButton::clicked, this, &QGitInteractiveRebaseDialog::on_toolButton_squash_clicked);
    connect(ui->toolButton_fixup, &QToolButton::clicked, this, &QGitInteractiveRebaseDialog::on_toolButton_fixup_clicked);
    connect(ui->toolButton_drop, &QToolButton::clicked, this, &QGitInteractiveRebaseDialog::on_toolButton_drop_clicked);
    connect(ui->toolButton_moveUp, &QToolButton::clicked, this, &QGitInteractiveRebaseDialog::on_toolButton_moveUp_clicked);
    connect(ui->toolButton_moveDown, &QToolButton::clicked, this, &QGitInteractiveRebaseDialog::on_toolButton_moveDown_clicked);
    connect(ui->toolButton_reset, &QToolButton::clicked, this, &QGitInteractiveRebaseDialog::on_toolButton_reset_clicked);

    connect(ui->tableWidget, &QTableWidget::customContextMenuRequested, this, &QGitInteractiveRebaseDialog::on_tableWidget_customContextMenuRequested);
    connect(ui->tableWidget, &QTableWidget::itemSelectionChanged, this, &QGitInteractiveRebaseDialog::on_tableWidget_itemSelectionChanged);
    connect(ui->plainTextEdit_message, &QPlainTextEdit::textChanged, this, &QGitInteractiveRebaseDialog::on_plainTextEdit_message_textChanged);
    connect(ui->pushButton_rebase, &QPushButton::clicked, this, &QGitInteractiveRebaseDialog::on_pushButton_rebase_clicked);
    connect(ui->pushButton_cancel, &QPushButton::clicked, this, &QGitInteractiveRebaseDialog::on_pushButton_cancel_clicked);
}

void QGitInteractiveRebaseDialog::loadCommits()
{
    if (!m_git) return;

    m_branchName = m_git->currentBranch();
    QString baseShort = m_baseCommitId.left(8);
    QString baseSummary = m_git->commitSummary(m_baseCommitId);

    ui->label_title->setText(tr("Interactive Rebase on branch <b>%1</b>").arg(m_branchName));
    if (!m_baseCommitId.isEmpty()) {
        ui->label_base->setText(tr("Rebasing on top of: <code>%1</code> %2")
                                    .arg(baseShort, baseSummary.isEmpty() ? QString() : QStringLiteral("(%1)").arg(baseSummary)));
    } else {
        ui->label_base->setText(tr("Rebasing all commits from root"));
    }

    QList<QGitCommit> commits = m_git->getCommitsForRebase(m_baseCommitId);
    m_items.clear();
    for (const auto &c : commits) {
        QGitRebaseTodoItem item;
        item.action = QGitRebaseAction::Pick;
        item.commitId = c.id();
        item.shortHash = c.id().left(8);
        item.summary = c.message().split('\n').first();
        item.message = c.message();
        item.author = c.author().name();
        item.timeStr = c.time().toString(QStringLiteral("yyyy-MM-dd hh:mm"));
        m_items.append(item);
    }
    m_originalItems = m_items;

    ui->label_stats->setText(tr("%1 commits to rebase").arg(m_items.size()));
    populateTable();

    if (!m_items.isEmpty()) {
        ui->tableWidget->selectRow(0);
    }
}

void QGitInteractiveRebaseDialog::populateTable()
{
    m_updatingUi = true;
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setRowCount(m_items.size());

    for (int row = 0; row < m_items.size(); ++row) {
        const auto &item = m_items.at(row);

        auto *combo = new QComboBox(ui->tableWidget);
        combo->addItem(tr("Pick (p)"), (int)QGitRebaseAction::Pick);
        combo->addItem(tr("Reword (r)"), (int)QGitRebaseAction::Reword);
        combo->addItem(tr("Squash (s)"), (int)QGitRebaseAction::Squash);
        combo->addItem(tr("Fixup (f)"), (int)QGitRebaseAction::Fixup);
        combo->addItem(tr("Drop (d)"), (int)QGitRebaseAction::Drop);
        combo->setCurrentIndex((int)item.action);
        combo->setProperty("row", row);

        connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &QGitInteractiveRebaseDialog::onActionComboChanged);

        ui->tableWidget->setCellWidget(row, 0, combo);

        auto *itemHash = new QTableWidgetItem(item.shortHash);
        QFont monoFont(QStringLiteral("Monospace"));
        monoFont.setStyleHint(QFont::TypeWriter);
        itemHash->setFont(monoFont);
        itemHash->setFlags(itemHash->flags() & ~Qt::ItemIsEditable);
        ui->tableWidget->setItem(row, 1, itemHash);

        auto *itemSubj = new QTableWidgetItem(item.summary);
        itemSubj->setFlags(itemSubj->flags() & ~Qt::ItemIsEditable);
        ui->tableWidget->setItem(row, 2, itemSubj);

        auto *itemAuth = new QTableWidgetItem(item.author);
        itemAuth->setFlags(itemAuth->flags() & ~Qt::ItemIsEditable);
        ui->tableWidget->setItem(row, 3, itemAuth);

        auto *itemDate = new QTableWidgetItem(item.timeStr);
        itemDate->setFlags(itemDate->flags() & ~Qt::ItemIsEditable);
        ui->tableWidget->setItem(row, 4, itemDate);

        updateRowAppearance(row);
    }

    m_updatingUi = false;
    updateValidationStatus();
}

void QGitInteractiveRebaseDialog::updateRowAppearance(int row)
{
    if (row < 0 || row >= m_items.size()) return;

    const auto &item = m_items.at(row);
    bool isDropped = (item.action == QGitRebaseAction::Drop);

    for (int col = 1; col < ui->tableWidget->columnCount(); ++col) {
        QTableWidgetItem *tItem = ui->tableWidget->item(row, col);
        if (!tItem) continue;

        QFont f = tItem->font();
        f.setStrikeOut(isDropped);
        tItem->setFont(f);

        if (isDropped) {
            tItem->setForeground(QColor(128, 128, 128));
        } else if (item.action == QGitRebaseAction::Squash) {
            tItem->setForeground(QColor(218, 112, 214)); // Orchid/Amber
        } else if (item.action == QGitRebaseAction::Fixup) {
            tItem->setForeground(QColor(147, 112, 219)); // Purple
        } else if (item.action == QGitRebaseAction::Reword) {
            tItem->setForeground(QColor(97, 175, 239));  // Blue
        } else {
            tItem->setForeground(QBrush()); // Default
        }
    }
}

void QGitInteractiveRebaseDialog::onActionComboChanged(int index)
{
    if (m_updatingUi) return;

    auto *combo = qobject_cast<QComboBox *>(sender());
    if (!combo) return;

    int row = combo->property("row").toInt();
    if (row >= 0 && row < m_items.size()) {
        m_items[row].action = static_cast<QGitRebaseAction>(index);
        updateRowAppearance(row);
        updateValidationStatus();

        if (row == m_currentEditingRow) {
            loadMessageForRow(row);
        }
    }
}

void QGitInteractiveRebaseDialog::on_tableWidget_itemSelectionChanged()
{
    if (m_updatingUi) return;

    int selectedRow = -1;
    auto selectedRows = ui->tableWidget->selectionModel()->selectedRows();
    if (!selectedRows.isEmpty()) {
        selectedRow = selectedRows.first().row();
    }

    saveCurrentMessage();
    m_currentEditingRow = selectedRow;
    loadMessageForRow(selectedRow);
}

void QGitInteractiveRebaseDialog::saveCurrentMessage()
{
    if (m_currentEditingRow >= 0 && m_currentEditingRow < m_items.size()) {
        m_items[m_currentEditingRow].message = ui->plainTextEdit_message->toPlainText();
        m_items[m_currentEditingRow].summary = m_items[m_currentEditingRow].message.split('\n').first();

        // Update subject in table if changed
        auto *subjItem = ui->tableWidget->item(m_currentEditingRow, 2);
        if (subjItem) {
            subjItem->setText(m_items[m_currentEditingRow].summary);
        }
    }
}

void QGitInteractiveRebaseDialog::loadMessageForRow(int row)
{
    m_updatingUi = true;

    if (row >= 0 && row < m_items.size()) {
        const auto &item = m_items.at(row);
        ui->label_messageCommitInfo->setText(
            tr("Commit <b>%1</b> (%2) - %3: %4")
                .arg(item.shortHash, item.author, actionToString(item.action), item.summary));
        ui->plainTextEdit_message->setPlainText(item.message);
        ui->plainTextEdit_message->setEnabled(true);
        ui->groupBox_message->setEnabled(true);
    } else {
        ui->label_messageCommitInfo->setText(tr("Select a commit to view or edit its message."));
        ui->plainTextEdit_message->clear();
        ui->plainTextEdit_message->setEnabled(false);
        ui->groupBox_message->setEnabled(false);
    }

    m_updatingUi = false;
}

void QGitInteractiveRebaseDialog::on_plainTextEdit_message_textChanged()
{
    if (m_updatingUi) return;
    if (m_currentEditingRow < 0 || m_currentEditingRow >= m_items.size()) return;

    // Automatically switch from Pick to Reword when user edits the commit message
    if (m_items[m_currentEditingRow].action == QGitRebaseAction::Pick) {
        m_items[m_currentEditingRow].action = QGitRebaseAction::Reword;
        auto *combo = qobject_cast<QComboBox *>(ui->tableWidget->cellWidget(m_currentEditingRow, 0));
        if (combo) {
            m_updatingUi = true;
            combo->setCurrentIndex((int)QGitRebaseAction::Reword);
            m_updatingUi = false;
        }
        updateRowAppearance(m_currentEditingRow);
    }

    saveCurrentMessage();
}

void QGitInteractiveRebaseDialog::setActionForSelectedRows(QGitRebaseAction action)
{
    auto selectedRows = ui->tableWidget->selectionModel()->selectedRows();
    if (selectedRows.isEmpty()) return;

    m_updatingUi = true;
    for (const auto &modelIndex : selectedRows) {
        int row = modelIndex.row();
        if (row >= 0 && row < m_items.size()) {
            m_items[row].action = action;
            auto *combo = qobject_cast<QComboBox *>(ui->tableWidget->cellWidget(row, 0));
            if (combo) {
                combo->setCurrentIndex((int)action);
            }
            updateRowAppearance(row);
        }
    }
    m_updatingUi = false;

    updateValidationStatus();
    if (m_currentEditingRow >= 0) {
        loadMessageForRow(m_currentEditingRow);
    }
}

void QGitInteractiveRebaseDialog::on_toolButton_pick_clicked()
{
    setActionForSelectedRows(QGitRebaseAction::Pick);
}

void QGitInteractiveRebaseDialog::on_toolButton_reword_clicked()
{
    setActionForSelectedRows(QGitRebaseAction::Reword);
    ui->plainTextEdit_message->setFocus();
}

void QGitInteractiveRebaseDialog::on_toolButton_squash_clicked()
{
    setActionForSelectedRows(QGitRebaseAction::Squash);
}

void QGitInteractiveRebaseDialog::on_toolButton_fixup_clicked()
{
    setActionForSelectedRows(QGitRebaseAction::Fixup);
}

void QGitInteractiveRebaseDialog::on_toolButton_drop_clicked()
{
    setActionForSelectedRows(QGitRebaseAction::Drop);
}

void QGitInteractiveRebaseDialog::moveSelectedRows(int direction)
{
    auto selectedIndices = ui->tableWidget->selectionModel()->selectedRows();
    if (selectedIndices.isEmpty()) return;

    saveCurrentMessage();

    QList<int> rows;
    for (const auto &idx : selectedIndices) {
        rows.append(idx.row());
    }
    std::sort(rows.begin(), rows.end());

    if (direction < 0) { // Move up
        if (rows.first() == 0) return;
        for (int r : rows) {
            m_items.swapItemsAt(r, r - 1);
        }
    } else if (direction > 0) { // Move down
        if (rows.last() == m_items.size() - 1) return;
        for (int i = rows.size() - 1; i >= 0; --i) {
            int r = rows.at(i);
            m_items.swapItemsAt(r, r + 1);
        }
    }

    populateTable();

    // Re-select rows
    ui->tableWidget->clearSelection();
    for (int r : rows) {
        int newRow = r + direction;
        ui->tableWidget->selectRow(newRow);
    }
}

void QGitInteractiveRebaseDialog::on_toolButton_moveUp_clicked()
{
    moveSelectedRows(-1);
}

void QGitInteractiveRebaseDialog::on_toolButton_moveDown_clicked()
{
    moveSelectedRows(1);
}

void QGitInteractiveRebaseDialog::on_toolButton_reset_clicked()
{
    auto confirm = QMessageBox::question(
        this,
        tr("Reset Interactive Rebase"),
        tr("Reset all commits back to 'Pick' in their original order?"),
        QMessageBox::Yes | QMessageBox::No
    );

    if (confirm == QMessageBox::Yes) {
        saveCurrentMessage();
        m_items = m_originalItems;
        populateTable();
        if (!m_items.isEmpty()) {
            ui->tableWidget->selectRow(0);
        }
    }
}

void QGitInteractiveRebaseDialog::on_tableWidget_customContextMenuRequested(const QPoint &pos)
{
    QModelIndex index = ui->tableWidget->indexAt(pos);
    if (!index.isValid()) return;

    QMenu menu(this);
    QAction *pickAct = menu.addAction(tr("Pick"));
    QAction *rewordAct = menu.addAction(tr("Reword"));
    QAction *squashAct = menu.addAction(tr("Squash"));
    QAction *fixupAct = menu.addAction(tr("Fixup"));
    QAction *dropAct = menu.addAction(tr("Drop"));

    menu.addSeparator();
    QAction *moveUpAct = menu.addAction(tr("Move Up"));
    QAction *moveDownAct = menu.addAction(tr("Move Down"));

    QAction *chosen = menu.exec(ui->tableWidget->viewport()->mapToGlobal(pos));
    if (!chosen) return;

    if (chosen == pickAct) on_toolButton_pick_clicked();
    else if (chosen == rewordAct) on_toolButton_reword_clicked();
    else if (chosen == squashAct) on_toolButton_squash_clicked();
    else if (chosen == fixupAct) on_toolButton_fixup_clicked();
    else if (chosen == dropAct) on_toolButton_drop_clicked();
    else if (chosen == moveUpAct) on_toolButton_moveUp_clicked();
    else if (chosen == moveDownAct) on_toolButton_moveDown_clicked();
}

void QGitInteractiveRebaseDialog::keyPressEvent(QKeyEvent *event)
{
    if (ui->tableWidget->hasFocus()) {
        if (event->key() == Qt::Key_P && !(event->modifiers() & Qt::ControlModifier)) {
            on_toolButton_pick_clicked();
            return;
        } else if (event->key() == Qt::Key_R && !(event->modifiers() & Qt::ControlModifier)) {
            on_toolButton_reword_clicked();
            return;
        } else if (event->key() == Qt::Key_S && !(event->modifiers() & Qt::ControlModifier)) {
            on_toolButton_squash_clicked();
            return;
        } else if (event->key() == Qt::Key_F && !(event->modifiers() & Qt::ControlModifier)) {
            on_toolButton_fixup_clicked();
            return;
        } else if (event->key() == Qt::Key_D && !(event->modifiers() & Qt::ControlModifier)) {
            on_toolButton_drop_clicked();
            return;
        } else if (event->key() == Qt::Key_Delete) {
            on_toolButton_drop_clicked();
            return;
        }
    }
    QDialog::keyPressEvent(event);
}

bool QGitInteractiveRebaseDialog::validateRebase(QString &errorMessage) const
{
    if (m_items.isEmpty()) {
        errorMessage = tr("No commits to rebase.");
        return false;
    }

    bool foundFirst = false;
    for (const auto &item : m_items) {
        if (item.action == QGitRebaseAction::Drop) {
            continue;
        }
        if (!foundFirst) {
            foundFirst = true;
            if (item.action == QGitRebaseAction::Squash || item.action == QGitRebaseAction::Fixup) {
                errorMessage = tr("The first commit in the rebase (%1) cannot be squashed or fixed up.").arg(item.shortHash);
                return false;
            }
        }
    }

    if (!foundFirst) {
        errorMessage = tr("Warning: All commits are marked as Drop. This will reset the branch to the base commit.");
    }

    return true;
}

void QGitInteractiveRebaseDialog::updateValidationStatus()
{
    QString error;
    bool valid = validateRebase(error);
    if (!valid) {
        ui->label_validation->setText(QStringLiteral("⚠️ ") + error);
        ui->pushButton_rebase->setEnabled(false);
    } else {
        ui->label_validation->setText(error); // Could be warning for all dropped
        ui->pushButton_rebase->setEnabled(true);
    }
}

void QGitInteractiveRebaseDialog::on_pushButton_rebase_clicked()
{
    saveCurrentMessage();

    QString error;
    if (!validateRebase(error)) {
        QMessageBox::warning(this, tr("Invalid Rebase Plan"), error);
        return;
    }

    bool allDropped = true;
    for (const auto &item : m_items) {
        if (item.action != QGitRebaseAction::Drop) {
            allDropped = false;
            break;
        }
    }

    if (allDropped) {
        auto res = QMessageBox::question(
            this,
            tr("All Commits Dropped"),
            tr("All commits are marked as dropped. This will reset the branch to the base commit (%1).\n\nDo you want to continue?").arg(m_baseCommitId.left(8)),
            QMessageBox::Yes | QMessageBox::No
        );
        if (res != QMessageBox::Yes) {
            return;
        }
    }

    accept();
}

void QGitInteractiveRebaseDialog::on_pushButton_cancel_clicked()
{
    reject();
}

QList<QGitRebaseTodoItem> QGitInteractiveRebaseDialog::todoItems() const
{
    return m_items;
}

QString QGitInteractiveRebaseDialog::baseCommitId() const
{
    return m_baseCommitId;
}

QString QGitInteractiveRebaseDialog::actionToString(QGitRebaseAction action)
{
    switch (action) {
    case QGitRebaseAction::Pick: return tr("Pick");
    case QGitRebaseAction::Reword: return tr("Reword");
    case QGitRebaseAction::Squash: return tr("Squash");
    case QGitRebaseAction::Fixup: return tr("Fixup");
    case QGitRebaseAction::Drop: return tr("Drop");
    }
    return QString();
}
