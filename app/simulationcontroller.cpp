// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "simulationcontroller.h"

#include "buzzer.h"
#include "clock.h"
#include "common.h"
#include "elementmapping.h"
#include "globalproperties.h"
#include "graphicelement.h"
#include "ic.h"
#include "icmapping.h"
#include "qneconnection.h"
#include "scene.h"
#include "simulationcontroller.h"

#include <QDebug>
#include <QGraphicsView>

SimulationController::SimulationController(Scene *scene)
    : QObject(scene)
    , m_scene(scene)
{
    m_simulationTimer.setInterval(globalClock);
    m_viewTimer.setInterval(1000 / 30);
    m_viewTimer.start();
    connect(&m_viewTimer, &QTimer::timeout, this, &SimulationController::updateView);
    connect(&m_simulationTimer, &QTimer::timeout, this, &SimulationController::tic);
}

SimulationController::~SimulationController()
{
    clear();
}

// If (m_shouldRestart) then the simulation controller will be cleared the next time that it is updated.
void SimulationController::setRestart() { m_shouldRestart = true; }

void SimulationController::updateScene(const QRectF &rect)
{
    if (!canRun()) {
        return;
    }

    const auto items = m_scene->items(rect);

    for (auto *item : items) {
        if (auto *connection = qgraphicsitem_cast<QNEConnection *>(item)) {
            updatePort(connection->start());
            continue;
        }

        if (auto *element = qgraphicsitem_cast<GraphicElement *>(item); element && element->elementGroup() == ElementGroup::Output) {
            const auto inputs = element->inputs();
            for (auto *input : inputs) {
                updatePort(input);
            }
        }
    }
}

void SimulationController::updateView()
{
    updateScene(m_scene->views().first()->sceneRect());
}

void SimulationController::updateAll()
{
    updateScene(m_scene->itemsBoundingRect());
}

bool SimulationController::canRun()
{
    return (m_elmMapping) ? m_elmMapping->canRun() : false;
}

bool SimulationController::isRunning()
{
    return m_simulationTimer.isActive();
}

void SimulationController::tic()
{
    update();
}

void SimulationController::update()
{
    if (m_shouldRestart) {
        m_shouldRestart = false;
        clear();
    }
    if (m_elmMapping) { // TODO: Remove this check, if possible. May increse the simulation speed significantly.
        m_elmMapping->update();
    }
}

void SimulationController::startTimer()
{
    qCDebug(zero) << "Starting timer.";
    m_simulationTimer.start();
}

void SimulationController::stop()
{
    m_simulationTimer.stop();
    m_viewTimer.stop();
    const auto elms = m_scene->elements();
    for (auto *elm : elms) {
        if (auto *buzzer = dynamic_cast<Buzzer *>(elm)) {
            buzzer->mute(true);
        }
    }
}

void SimulationController::start()
{
    qCDebug(zero) << "Start simulation controller.";
    Clock::reset = true;
    reSortElements();
    m_simulationTimer.start();
    m_viewTimer.start();
    qCDebug(zero) << "Simulation started.";
    const auto elms = m_scene->elements();
    for (auto *elm : elms) {
        if (auto *buzzer = dynamic_cast<Buzzer *>(elm)) {
            buzzer->mute(false);
        }
    }
}

void SimulationController::reSortElements()
{
    qCDebug(two) << "GENERATING SIMULATION LAYER.";
    auto elements = m_scene->elements();
    qCDebug(zero) << "Elements read:" << elements.size();
    if (elements.empty()) {
        return;
    }
    qCDebug(two) << "Deleting existing mapping.";
    delete m_elmMapping;
    qCDebug(two) << "Recreating mapping for simulation.";
    m_elmMapping = new ElementMapping(elements);
    if (m_elmMapping->canInitialize()) {
        qCDebug(two) << "Can initialize.";
        m_elmMapping->initialize();
        qCDebug(two) << "Sorting.";
        m_elmMapping->sort();
        qCDebug(two) << "Updating.";
        update();
    } else {
        qCDebug(zero) << "Cannot initialize simulation.";
    }
    qCDebug(zero) << "Finished simulation layer.";
}

void SimulationController::clear()
{
    delete m_elmMapping;
    m_elmMapping = nullptr;
}

void SimulationController::updatePort(QNEOutputPort *port)
{
    if (port) {
        GraphicElement *elm = port->graphicElement();
        LogicElement *logElm = nullptr;
        int portIndex = 0;
        if (elm->elementType() == ElementType::IC) {
            IC *ic = dynamic_cast<IC *>(elm);
            logElm = m_elmMapping->icMapping(ic)->output(port->index());
        } else {
            logElm = m_elmMapping->logicElement(elm);
            portIndex = port->index();
        }
        if (logElm->isValid()) {
            port->setValue(static_cast<signed char>(logElm->outputValue(portIndex)));
        } else {
            port->setValue(-1);
        }
    }
}

void SimulationController::updatePort(QNEInputPort *port)
{
    GraphicElement *elm = port->graphicElement();
    LogicElement *logElm = m_elmMapping->logicElement(elm);
    int portIndex = port->index();
    if (logElm->isValid()) {
        port->setValue(static_cast<signed char>(logElm->inputValue(portIndex)));
    } else {
        port->setValue(-1);
    }
    if (elm->elementGroup() == ElementGroup::Output) {
        elm->refresh();
    }
}
