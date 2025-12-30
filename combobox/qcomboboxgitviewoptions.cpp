#include "qcomboboxgitviewoptions.h"

#include <QStandardItemModel>
#include <QAbstractItemView>
#include <QStylePainter>
#include <QApplication>
#include <QSvgRenderer>
#include <QFile>


enum {
    ITEM_FLAT_LIST_SINGLE_COLUMN,
    ITEM_FLAT_LIST_MULTIPLE_COLUMN,
    ITEM_TREE_VIEW,
    ITEM_SEPARATOR,
    ITEM_NO_STAGING,
    ITEM_FLUID_STAGING,
    ITEM_SPLIT_VIEW_STAGING,
};

QComboBoxGitViewOptions::QComboBoxGitViewOptions(QWidget *parent)
    : QComboBox(parent)
    , m_iconFlatListSingleColumn(":/QComboBoxGitViewOptions/flat_list_single_column")
    , m_iconFlatListMultipleColumn(":/QComboBoxGitViewOptions/flat_list_multiple_column")
    , m_iconTreeView(":/QComboBoxGitViewOptions/tree_view")
    , m_iconNoStaging(":/QComboBoxGitViewOptions/no_staging")
    , m_iconFluidStaging(":/QComboBoxGitViewOptions/fluid_staging")
    , m_iconSplitViewStaging(":/QComboBoxGitViewOptions/split_view_staging")
{
    QStandardItemModel *model = new QStandardItemModel();
    setModel(model);

    if (QApplication::style()->name() == "fusion")
    {
        m_showIcons = true;
    }

    updateIconColor();

    QStandardItem* item = nullptr;

    item = new QStandardItem(tr("Flat list (single column)"));
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    item->setCheckState(Qt::Checked);
    if (m_showIcons) item->setIcon(m_iconFlatListSingleColumn);
    model->appendRow(item);

    item = new QStandardItem(tr("Flat list (multiple column)"));
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    item->setCheckState(Qt::Unchecked);
    if (m_showIcons) item->setIcon(m_iconFlatListMultipleColumn);
    model->appendRow(item);

    item = new QStandardItem(tr("Tree view"));
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    item->setCheckState(Qt::Unchecked);
    if (m_showIcons) item->setIcon(m_iconTreeView);
    model->appendRow(item);

    insertSeparator(model->rowCount());

    item = new QStandardItem(tr("No staging"));
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    item->setCheckState(Qt::Unchecked);
    if (m_showIcons) item->setIcon(m_iconNoStaging);
    model->appendRow(item);

    item = new QStandardItem(tr("Fluid staging"));
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    item->setCheckState(Qt::Unchecked);
    if (m_showIcons) item->setIcon(m_iconFluidStaging);
    model->appendRow(item);

    item = new QStandardItem(tr("Split view staging"));
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    item->setCheckState(Qt::Checked);
    if (m_showIcons) item->setIcon(m_iconSplitViewStaging);
    model->appendRow(item);

    view()->setMinimumWidth(view()->sizeHintForColumn(0));

    setIcon(0);

    connect(this, &QComboBox::activated, this,  &QComboBoxGitViewOptions::activated);
}

QSize QComboBoxGitViewOptions::sizeHint() const
{
    return minimumSizeHint();
}

QSize QComboBoxGitViewOptions::minimumSizeHint() const
{
    QStyleOptionComboBox opt;
    opt.initFrom(this);

    QSize contentSize;

    opt.currentIcon = m_iconTreeView;
    opt.iconSize = iconSize();

// TODO: Periodicly check if we still need this Windows style fix.
#ifdef Q_OS_WIN
    contentSize = QSize(0, iconSize().width());
#else
    contentSize = iconSize();
#endif

    return style()->sizeFromContents(QStyle::CT_ComboBox, &opt, contentSize, this);
}

void QComboBoxGitViewOptions::paintEvent(QPaintEvent *event)
{
    QStyleOptionComboBox opt;
    QStylePainter p(this);

    Q_UNUSED(event);

    opt.initFrom(this);

    opt.currentIcon = m_icon;
    opt.iconSize = iconSize();

    p.drawComplexControl(QStyle::CC_ComboBox, opt);
    p.drawControl(QStyle::CE_ComboBoxLabel, opt);
}

void QComboBoxGitViewOptions::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::ThemeChange)
    {
        updateIconColor();

        setIcon(m_currentView);
    }

    QWidget::changeEvent(event);
}

void QComboBoxGitViewOptions::showPopup()
{
    view()->selectionModel()->reset();

    QComboBox::showPopup();

    QWidget *popup = view()->parentWidget();
    if (popup) {
        QPoint pos = mapToGlobal(QPoint(0, height()));
        popup->move(pos.x(), pos.y());
    }
}

void QComboBoxGitViewOptions::activated(int index)
{
    QAbstractItemModel *items = model();

    if ((index >= ITEM_FLAT_LIST_SINGLE_COLUMN)&&(index <= ITEM_TREE_VIEW))
    {
        for(int c = ITEM_FLAT_LIST_SINGLE_COLUMN; c <= ITEM_TREE_VIEW; c++)
        {
            items->setData(items->index(c, 0), (c == index)?Qt::Checked:Qt::Unchecked, Qt::CheckStateRole);
        }
    }

    if ((index >= ITEM_NO_STAGING)&&(index <= ITEM_SPLIT_VIEW_STAGING))
    {
        for(int c = ITEM_NO_STAGING; c <= ITEM_SPLIT_VIEW_STAGING; c++)
        {
            items->setData(items->index(c, 0), (c == index)?Qt::Checked:Qt::Unchecked, Qt::CheckStateRole);
        }
    }

    setIcon(index);
}

void QComboBoxGitViewOptions::setIcon(int index)
{
    QFile resource;

    switch(index)
    {
    case 0:
        m_currentView = index;
        resource.setFileName(":/QComboBoxGitViewOptions/flat_list_single_column");
        break;
    case 1:
        m_currentView = index;
        resource.setFileName(":/QComboBoxGitViewOptions/flat_list_multiple_column");
        break;
    case 2:
        m_currentView = index;
        resource.setFileName(":/QComboBoxGitViewOptions/tree_view");
        break;
    default:
        return;
    }

    if (resource.open(QIODeviceBase::ReadOnly))
    {
        auto svgContent = resource.readAll();

        QString newColor = palette().color(QPalette::PlaceholderText).name();
        svgContent.replace("#000000", newColor.toUtf8());

        QSvgRenderer renderer(svgContent);
        if (renderer.isValid()) {
            QImage image(iconSize(), QImage::Format_ARGB32_Premultiplied);
            image.fill(0);
            QPainter painter(&image);
            renderer.render(&painter);
            m_icon = QPixmap::fromImage(image);
            update();
        }
    }
}

void QComboBoxGitViewOptions::updateIconColor()
{
    bool doUpdate = false;
    QFile resource;

    resource.setFileName(":/QComboBoxGitViewOptions/flat_list_single_column");
    if (resource.open(QIODeviceBase::ReadOnly))
    {
        auto svgContent = resource.readAll();

        QString newColor = palette().color(QPalette::Text).name();
        svgContent.replace("#000000", newColor.toUtf8());

        QSvgRenderer renderer(svgContent);
        if (renderer.isValid()) {
            QImage image(iconSize(), QImage::Format_ARGB32_Premultiplied);
            image.fill(0);
            QPainter painter(&image);
            renderer.render(&painter);
            m_iconFlatListSingleColumn = QPixmap::fromImage(image);
            doUpdate = true;
        }

        resource.close();
    }

    resource.setFileName(":/QComboBoxGitViewOptions/flat_list_multiple_column");
    if (resource.open(QIODeviceBase::ReadOnly))
    {
        auto svgContent = resource.readAll();

        QString newColor = palette().color(QPalette::Text).name();
        svgContent.replace("#000000", newColor.toUtf8());

        QSvgRenderer renderer(svgContent);
        if (renderer.isValid()) {
            QImage image(iconSize(), QImage::Format_ARGB32_Premultiplied);
            image.fill(0);
            QPainter painter(&image);
            renderer.render(&painter);
            m_iconFlatListMultipleColumn = QPixmap::fromImage(image);
            doUpdate = true;
        }

        resource.close();
    }

    resource.setFileName(":/QComboBoxGitViewOptions/tree_view");
    if (resource.open(QIODeviceBase::ReadOnly))
    {
        auto svgContent = resource.readAll();

        QString newColor = palette().color(QPalette::Text).name();
        svgContent.replace("#000000", newColor.toUtf8());

        QSvgRenderer renderer(svgContent);
        if (renderer.isValid()) {
            QImage image(iconSize(), QImage::Format_ARGB32_Premultiplied);
            image.fill(0);
            QPainter painter(&image);
            renderer.render(&painter);
            m_iconTreeView = QPixmap::fromImage(image);
            doUpdate = true;
        }

        resource.close();
    }

    resource.setFileName(":/QComboBoxGitViewOptions/no_staging");
    if (resource.open(QIODeviceBase::ReadOnly))
    {
        auto svgContent = resource.readAll();

        QString newColor = palette().color(QPalette::Text).name();
        svgContent.replace("#000000", newColor.toUtf8());

        QSvgRenderer renderer(svgContent);
        if (renderer.isValid()) {
            QImage image(iconSize(), QImage::Format_ARGB32_Premultiplied);
            image.fill(0);
            QPainter painter(&image);
            renderer.render(&painter);
            m_iconNoStaging = QPixmap::fromImage(image);
            doUpdate = true;
        }

        resource.close();
    }

    resource.setFileName(":/QComboBoxGitViewOptions/fluid_staging");
    if (resource.open(QIODeviceBase::ReadOnly))
    {
        auto svgContent = resource.readAll();

        QString newColor = palette().color(QPalette::Text).name();
        svgContent.replace("#000000", newColor.toUtf8());

        QSvgRenderer renderer(svgContent);
        if (renderer.isValid()) {
            QImage image(iconSize(), QImage::Format_ARGB32_Premultiplied);
            image.fill(0);
            QPainter painter(&image);
            renderer.render(&painter);
            m_iconFluidStaging = QPixmap::fromImage(image);
            doUpdate = true;
        }

        resource.close();
    }

    resource.setFileName(":/QComboBoxGitViewOptions/split_view_staging");
    if (resource.open(QIODeviceBase::ReadOnly))
    {
        auto svgContent = resource.readAll();

        QString newColor = palette().color(QPalette::Text).name();
        svgContent.replace("#000000", newColor.toUtf8());

        QSvgRenderer renderer(svgContent);
        if (renderer.isValid()) {
            QImage image(iconSize(), QImage::Format_ARGB32_Premultiplied);
            image.fill(0);
            QPainter painter(&image);
            renderer.render(&painter);
            m_iconSplitViewStaging = QPixmap::fromImage(image);
            doUpdate = true;
        }

        resource.close();
    }

    if (doUpdate)
    {
        update();
    }
}
