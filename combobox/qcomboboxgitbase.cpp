#include "qcomboboxgitbase.h"

#include <QStyleOptionComboBox>
#include <QStylePainter>
#include <QAbstractItemView>
#include <QStandardItemModel>
#include <QApplication>
#include <QEvent>

QComboBoxGitBase::QComboBoxGitBase(QWidget *parent)
    : QComboBox(parent)
{
}

QSize QComboBoxGitBase::sizeHint() const
{
    return minimumSizeHint();
}

QSize QComboBoxGitBase::minimumSizeHint() const
{
    QStyleOptionComboBox opt;
    opt.initFrom(this);

    opt.currentIcon = m_icon;
    opt.iconSize = iconSize();

    QSize contentSize;
    if (!m_displayText.isEmpty()) {
        QFontMetrics fm = fontMetrics();
        contentSize = fm.size(Qt::TextSingleLine, m_displayText);
#ifdef Q_OS_WIN
        contentSize.setWidth(contentSize.width() - iconSize().width());
#endif
    } else {
#ifdef Q_OS_WIN
        contentSize = QSize(0, iconSize().width());
#else
        contentSize = iconSize();
#endif
    }

    return style()->sizeFromContents(QStyle::CT_ComboBox, &opt, contentSize, this);
}

void QComboBoxGitBase::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QStyleOptionComboBox opt;
    QStylePainter p(this);

    opt.initFrom(this);
    opt.currentIcon = m_icon;
    if (!m_displayText.isEmpty()) {
        opt.currentText = m_displayText;
    }
    opt.iconSize = iconSize();

    p.drawComplexControl(QStyle::CC_ComboBox, opt);
    p.drawControl(QStyle::CE_ComboBoxLabel, opt);
}

void QComboBoxGitBase::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::ThemeChange)
    {
        updateIconColor();
    }

    QWidget::changeEvent(event);
}

void QComboBoxGitBase::showPopup()
{
    if (view() && view()->selectionModel()) {
        view()->selectionModel()->reset();
    }

    QComboBox::showPopup();

    if (view()) {
        QWidget *popup = view()->parentWidget();
        if (popup) {
            QPoint pos = mapToGlobal(QPoint(0, height()));
            popup->move(pos.x(), pos.y());
        }
    }
}

QStandardItemModel *QComboBoxGitBase::initStandardModel()
{
    QStandardItemModel *model = new QStandardItemModel(this);
    setModel(model);

    if (QApplication::style()->name() == "fusion")
    {
        m_showIcons = true;
    }

    return model;
}

QStandardItem *QComboBoxGitBase::addOptionItem(const QString &text, bool isSelectable, Qt::CheckState checkState, const QIcon &icon)
{
    QStandardItemModel *stdModel = qobject_cast<QStandardItemModel *>(model());
    if (!stdModel) return nullptr;

    QStandardItem *item = new QStandardItem(text);
    if (isSelectable) {
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    } else {
        item->setFlags(Qt::NoItemFlags);
    }
    item->setData(checkState, Qt::CheckStateRole);

    if (m_showIcons && !icon.isNull()) {
        item->setIcon(icon);
    }

    stdModel->appendRow(item);
    return item;
}

QStandardItem *QComboBoxGitBase::addHeaderItem(const QString &text, const QIcon &icon)
{
    return addOptionItem(text, false, Qt::Unchecked, icon);
}
