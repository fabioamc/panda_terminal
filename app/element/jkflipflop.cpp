// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "jkflipflop.h"

#include "common.h"
#include "qneport.h"

namespace
{
int id = qRegisterMetaType<JKFlipFlop>();
}

JKFlipFlop::JKFlipFlop(QGraphicsItem *parent)
    : GraphicElement(ElementType::JKFlipFlop, ElementGroup::Memory, 5, 5, 2, 2, parent)
{
    m_defaultSkins = QStringList{":/memory/JK-flipflop.png"};
    m_alternativeSkins = m_defaultSkins;
    setPixmap(m_defaultSkins.first());

    setRotatable(false);
    setCanChangeSkin(true);
    JKFlipFlop::updatePorts();
    setPortName("FlipFlop JK");
    setToolTip(m_translatedName);

    input(0)->setName("J");
    input(1)->setName("Clock");
    input(2)->setName("K");
    input(3)->setName("~Preset");
    input(4)->setName("~Clear");

    output(0)->setName("Q");
    output(1)->setName("~Q");

    input(0)->setRequired(false); /* J */
    input(2)->setRequired(false); /* K */
    input(3)->setRequired(false); /* p */
    input(4)->setRequired(false); /* c */

    input(0)->setDefaultValue(1);
    input(2)->setDefaultValue(1);
    input(3)->setDefaultValue(1);
    input(4)->setDefaultValue(1);

    output(0)->setDefaultValue(0);
    output(1)->setDefaultValue(1);
}

void JKFlipFlop::updatePorts()
{
    input(0)->setPos(leftPosition(), 13);   /* J      */
    input(1)->setPos(leftPosition(), 29);   /* Clk    */
    input(2)->setPos(leftPosition(), 45);   /* K      */
    input(3)->setPos(32, leftPosition());   /* Preset */
    input(4)->setPos(32, rightPosition());  /* Clear  */

    output(0)->setPos(rightPosition(), 15); /* Q      */
    output(1)->setPos(rightPosition(), 45); /* ~Q     */
}
