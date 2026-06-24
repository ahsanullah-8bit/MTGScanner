#pragma once

#include <QObject>
#include <QStyledItemDelegate>

namespace MTGS {

class NameplateDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit NameplateDelegate(QObject *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

}