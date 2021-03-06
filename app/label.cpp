// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "label.h"

#include "elementfactory.h"

#include <QDrag>
#include <QMimeData>
#include <QMouseEvent>

Label::Label(QWidget *parent)
    : QLabel(parent)
{
}

ElementType Label::elementType()
{
    return m_elementType;
}

void Label::setElementType(const ElementType elementType)
{
    m_elementType = elementType;
}

void Label::mousePressEvent(QMouseEvent *event)
{
    startDrag(event->pos());
}

void Label::setPixmapData(const QPixmap &pixmapData)
{
    m_pixmapData = pixmapData;
    setPixmap(pixmapData.scaled(64, 64));
}

const QPixmap &Label::pixmapData() const
{
    return m_pixmapData;
}

QString Label::name() const
{
    return m_name;
}

void Label::setName(const QString &name)
{
    m_name = name;
}

QString Label::auxData() const
{
    return m_auxData;
}

void Label::setAuxData(const QString &auxData)
{
    m_auxData = auxData;
    setProperty("Name", auxData);
}

void Label::startDrag(QPoint pos)
{
    QPixmap pixMap = pixmapData();
    if (pos.isNull()) {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
        pos = pixmap().rect().center();
#else
        pos = pixmap()->rect().center();
#endif
    }
    QByteArray itemData;
    QDataStream stream(&itemData, QIODevice::WriteOnly);
    stream << pos << m_elementType << m_auxData;

    auto *mimeData = new QMimeData;
    mimeData->setData("application/x-dnditemdata", itemData);
    auto *drag = new QDrag(parent());
    drag->setMimeData(mimeData);
    drag->setPixmap(pixMap);
    drag->setHotSpot(pos);
    drag->exec(Qt::CopyAction, Qt::CopyAction);
}
