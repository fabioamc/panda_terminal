// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dlatch.h"

#include "common.h"
#include "qneport.h"

namespace
{
int id = qRegisterMetaType<DLatch>();
}

DLatch::DLatch(QGraphicsItem *parent)
    : GraphicElement(ElementType::DLatch, ElementGroup::Memory, 2, 2, 2, 2, parent)
{
    qCDebug(zero) << "Creating dlatch.";

    m_pixmapSkinName = QStringList{":/memory/D-latch.png"};
    setPixmap(m_pixmapSkinName.first());

    setRotatable(false);
    setCanChangeSkin(true);
    DLatch::updatePorts();
    setPortName("D Latch");
    setToolTip(m_translatedName);

    input(0)->setName("Data");
    input(1)->setName("Enable");

    output(0)->setName("Q");
    output(1)->setName("~Q");

    output(0)->setDefaultValue(0);
    output(1)->setDefaultValue(1);
}

void DLatch::updatePorts()
{
    input(0)->setPos(topPosition(), 13);     /* Data   */
    input(1)->setPos(topPosition(), 45);     /* Enable */

    output(0)->setPos(bottomPosition(), 15); /* Q      */
    output(1)->setPos(bottomPosition(), 45); /* ~Q     */
}

void DLatch::setSkin(const bool defaultSkin, const QString &fileName)
{
    m_pixmapSkinName[0] = (defaultSkin) ? ":/memory/D-latch.png" : fileName;
    setPixmap(m_pixmapSkinName[0]);
}
