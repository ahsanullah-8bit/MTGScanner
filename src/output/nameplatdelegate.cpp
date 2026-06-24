#include "nameplatdelegate.h"

#include <QSize>
#include <QImage>
#include <QDebug>
#include <QObject>
#include <QVariant>
#include <QPainter>
#include <QPainterPath>
#include <QPaintDevice>
#include <QStyledItemDelegate>

#include <core/constants.hpp>

namespace MTGS {

NameplateDelegate::NameplateDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{}

void NameplateDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

    QVariant v = index.data(Qt::DecorationRole);
    QImage img = v.value<QImage>();
    if (!img.isNull()) {
        QRect target_rect = option.rect.adjusted(4, 3, -4, -3);

        QPainterPath path;
        path.addRoundedRect(target_rect, 6, 6);
        painter->setClipPath(path);

        painter->drawImage(target_rect, img);
    }

    if (option.state & QStyle::State_Selected) {
        painter->setClipping(false);
        painter->setPen(QPen(option.palette.highlight(), 2));
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(option.rect.adjusted(1, 1, -1, -1), 6, 6);
    }

    painter->restore();
}

QSize NameplateDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QVariant v = index.data(Qt::SizeHintRole);
    if (v.isValid() && v.canConvert<QSize>())
        return v.toSize();

    return QSize(TITLE_WIDTH, TITLE_HEIGHT + 6);
}

}