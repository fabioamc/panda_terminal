// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "not.h"

#include "common.h"

namespace
{
int id = qRegisterMetaType<Not>();
}

Not::Not(QGraphicsItem *parent)
    : GraphicElement(ElementType::Not, ElementGroup::Gate, 1, 1, 1, 1, parent)
{
    qCDebug(zero) << "Creating not.";

    m_pixmapSkinName = QStringList{":/basic/not.png"};
    setPixmap(m_pixmapSkinName.first());

    setOutputsOnTop(true);
    setCanChangeSkin(true);
    updatePorts();
    setPortName("NOT");
    setToolTip(m_translatedName);
}

void Not::setSkin(const bool defaultSkin, const QString &fileName)
{
    m_pixmapSkinName[0] = (defaultSkin) ? ":/basic/not.png" : fileName;
    setPixmap(m_pixmapSkinName[0]);
}
