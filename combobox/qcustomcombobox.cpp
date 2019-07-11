#include "qcustomcombobox.h"
#include <QStylePainter>
#include <QStyleOption>
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QtMath>


QCustomComboBox::QCustomComboBox(QWidget *parent)
    : QWidget(parent)
    , m_popup(new QFrame(nullptr, Qt::Popup))
{
    setAttribute(Qt::WA_Hover);

    QHBoxLayout *layout = new QHBoxLayout(m_popup);
    layout->setMargin(0);

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

QCustomComboBox::~QCustomComboBox()
{
    delete m_popup;
    m_popup = nullptr;
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
        ret.setWidth(qMax(ret.width(), m_icon.availableSizes().first().height()));
    }

    ret += QSize(ret.height() + 4, 4);

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

void QCustomComboBox::paintEvent(__attribute__((unused)) QPaintEvent *event)
{
    QStyleOptionComboBox opt;
    QStylePainter p(this);
    bool paintIcon = false;

    paintIcon = !m_icon.isNull();

    opt.initFrom(this);

    if (paintIcon)
    {
        opt.currentIcon = m_icon;
        opt.iconSize = QSize(16, 16);
        opt.editable = true;
    }

    //opt.frame = true;
    p.drawComplexControl(QStyle::CC_ComboBox, opt);

    if (!paintIcon)
    {
        int margin = 4;

        QRect rect = QRect(margin, margin, width() - margin, height() - margin);
        p.drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, m_text);
    }
}

void QCustomComboBox::mousePressEvent(__attribute__((unused)) QMouseEvent *event)
{
    if (m_popup->isHidden())
    {
        QSize size;

        m_popup->move(mapToGlobal(QPoint(0, height())));

        for(int c = 0; c < m_list->count(); c++)
        {
            const auto &item = m_list->item(c);

            size.setHeight(size.height() + m_list->visualItemRect(item).height());
        }

        size.setHeight(size.height() + 1); // TODO: 1px quirk.

        m_popup->resize(size);

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
