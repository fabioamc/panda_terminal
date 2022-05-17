/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "logicelement.h"

class LogicDFlipFlop : public LogicElement
{
public:
    explicit LogicDFlipFlop();

protected:
    void _updateLogic(const QVector<bool> &inputs) override;

private:
    bool m_lastClk = false;
    bool m_lastValue = true;
};

