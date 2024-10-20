#include "qcustomcombobox.h"
#include <QStylePainter>
#include <QApplication>
#include <QStyleOption>
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QScreen>
#include <QtMath>


QCustomComboBox::QCustomComboBox(QWidget *parent)
    : QComboBox(parent)
    , m_popup(new QFrame(this, Qt::Popup))
{
    setAttribute(Qt::WA_Hover);

    m_popup->setFrameStyle(QFrame::StyledPanel);

    QHBoxLayout *layout = new QHBoxLayout(m_popup);
    layout->setContentsMargins(0, 0, 0, 0);

    m_list = new QListWidget();

    layout->addWidget(m_list);

    m_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_list->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_list->setSelectionMode(QAbstractItemView::NoSelection);
    m_list->setFocusPolicy(Qt::NoFocus);
    m_list->setStyleSheet("QListWidget {border: none}"
                          "QListWidget::indicator:checked{ image: url(:/QCustomComboBox/check) }"
                          "QListWidget::indicator:unchecked{ image: none }");

    connect(m_list, &QListWidget::itemClicked, this, &QCustomComboBox::listItemClicked);
}

QSize QCustomComboBox::sizeHint() const
{
    return minimumSizeHint();
}

QSize QCustomComboBox::minimumSizeHint() const
{
    QSize ret;

    const QFontMetrics &fm = fontMetrics();

    auto iconSizes = m_icon.availableSizes();

    if (m_icon.isNull())
    {
        if (m_text.isEmpty())
            ret = fm.size(Qt::TextSingleLine, "xxxxxx");
        else
            ret = fm.size(Qt::TextSingleLine, m_text);
    }
    else
    {
        ret.setWidth(16);
    }

    ret.setHeight(qMax(qCeil(ret.height()), 22));

    if (!iconSizes.isEmpty()) {
        ret.setWidth(qMax(ret.width(), iconSizes.first().height()));
    }

    ret += QSize(ret.height() + 8, 4);

    return ret;
}

void QCustomComboBox::setCurrentText(const QString &text)
{
    m_text = text;
    m_icon = QIcon();

    updateGeometry();
    update();
}

void QCustomComboBox::setCurrentIcon(const QIcon &icon)
{
    m_icon = icon;
    m_text = QString();

    updateGeometry();
    update();
}

void QCustomComboBox::paintEvent(QPaintEvent *event)
{
    QStyleOptionComboBox opt;
    QStylePainter p(this);
    bool paintIcon = false;

    Q_UNUSED(event);

    paintIcon = !m_icon.isNull();

    opt.initFrom(this);

    if (paintIcon)
    {
        opt.currentIcon = m_icon;
        opt.iconSize = QSize(16, 16);
    }
    else
    {
        opt.currentText = m_text;
    }

    //opt.frame = true;
    p.drawComplexControl(QStyle::CC_ComboBox, opt);
    p.drawControl(QStyle::CE_ComboBoxLabel, opt);
}

void QCustomComboBox::mousePressEvent(QMouseEvent *event)
{
    QScreen *currentScreen = nullptr;
    QSize size;

    if (m_popup->isHidden())
    {
        currentScreen = QApplication::screenAt(event->globalPosition().toPoint());

        m_popup->move(mapToGlobal(QPoint(0, height())));

        for(int c = 0; c < m_list->count(); c++)
        {
            const auto &item = m_list->item(c);

            size.setHeight(size.height() + m_list->visualItemRect(item).height());
        }

        size.setHeight(size.height() + (m_popup->frameWidth() * 2) + 1); // TODO: 1px quirk.

        m_popup->resize(size);
        m_popup->update();

        if (m_popup->pos().x() < 0)
        {
            m_popup->move(QPoint(0, m_popup->pos().y()));
        }
        else if (m_popup->pos().x() + m_popup->width() > currentScreen->geometry().right())
        {
            m_popup->move(QPoint(currentScreen->geometry().right() - m_popup->width(), m_popup->pos().y()));
        }

        m_popup->show();
    }
    else
    {
        m_popup->hide();
    }
}

void QCustomComboBox::listItemClicked(QListWidgetItem *item)
{
    int clickedIndex = -1;

    if (item->flags() & Qt::ItemIsEnabled)
    {
        m_popup->hide();

        if (item->flags() & Qt::ItemIsUserCheckable)
        {
            for(int c = 0; c < m_list->count(); c++)
            {
                auto i = m_list->item(c);

                if (item == i)
                {
                    clickedIndex = c;
                    break;
                }
            }

            Q_ASSERT(clickedIndex >= 0);

            for(int c = clickedIndex - 1; c >= 0; c--)
            {
                auto i = m_list->item(c);

                if ((i->flags() & Qt::ItemIsUserCheckable) == 0)
                    break;

                i->setCheckState(Qt::Unchecked);
            }

            item->setCheckState(Qt::Checked);

            for(int c = clickedIndex + 1; c < m_list->count(); c++)
            {
                auto i = m_list->item(c);

                if ((i->flags() & Qt::ItemIsUserCheckable) == 0)
                    break;

                i->setCheckState(Qt::Unchecked);
            }
        }

        clicked(item);

        emit itemClicked(item);
    }
}
