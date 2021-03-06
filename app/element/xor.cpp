// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "xor.h"

#include "common.h"

namespace
{
int id = qRegisterMetaType<Xor>();
}

Xor::Xor(QGraphicsItem *parent)
    : GraphicElement(ElementType::Xor, ElementGroup::Gate, 2, 8, 1, 1, parent)
{
    m_defaultSkins.append(":/basic/xor.png");
    m_alternativeSkins = m_defaultSkins;
    setPixmap(m_defaultSkins.first());

    setCanChangeSkin(true);
    updatePorts();
    setPortName("XOR");
    setToolTip(m_translatedName);
}
