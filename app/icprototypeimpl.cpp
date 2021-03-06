// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "icprototypeimpl.h"

#include "common.h"
#include "elementfactory.h"
#include "ic.h"
#include "icmapping.h"
#include "icprototype.h"
#include "qneconnection.h"
#include "qneport.h"
#include "serializationfunctions.h"

#include <QFile>
#include <QFileInfo>

ICPrototypeImpl::~ICPrototypeImpl()
{
    qDeleteAll(m_elements);
}

bool comparePorts(QNEPort *port1, QNEPort *port2)
{
    QPointF p1 = port1->graphicElement()->pos();
    QPointF p2 = port2->graphicElement()->pos();
    if (p1 != p2) {
        return p1.y() < p2.y() || (qFuzzyCompare(p1.y(), p2.y()) && p1.x() < p2.x());
    }
    p1 = port1->pos();
    p2 = port2->pos();
    return p1.x() < p2.x() || (qFuzzyCompare(p1.x(), p2.x()) && p1.y() < p2.y());
}

void ICPrototypeImpl::sortPorts(QVector<QNEPort *> &map)
{
    std::stable_sort(map.begin(), map.end(), comparePorts);
}

void ICPrototypeImpl::loadFile(const QString &fileName)
{
    qCDebug(zero) << "Reading IC.";
    clear();
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        throw Pandaception("Error opening file: " + file.errorString());
    }
    QDataStream stream(&file);
    auto items = SerializationFunctions::load(stream);
    for (auto *item : qAsConst(items)) {
        loadItem(item);
    }
    file.close();
    setInputSize(m_inputs.size());
    setOutputSize(m_outputs.size());
    sortPorts(m_inputs);
    sortPorts(m_outputs);
    loadInputs();
    loadOutputs();
    qCDebug(zero) << "Finished Reading IC.";
}

void ICPrototypeImpl::loadInputs()
{
    for (int portIndex = 0; portIndex < inputSize(); ++portIndex) {
        GraphicElement *elm = m_inputs.at(portIndex)->graphicElement();
        QString lb = elm->label();
        if (!m_inputs.at(portIndex)->portName().isEmpty()) {
            lb += " ";
            lb += m_inputs.at(portIndex)->portName();
        }
        if (!elm->genericProperties().isEmpty()) {
            lb += " [" + elm->genericProperties() + "]";
        }
        m_inputLabels[portIndex] = lb;
    }
}

void ICPrototypeImpl::loadOutputs()
{
    for (int portIndex = 0; portIndex < outputSize(); ++portIndex) {
        GraphicElement *elm = m_outputs.at(portIndex)->graphicElement();
        QString lb = elm->label();
        if (!m_outputs.at(portIndex)->portName().isEmpty()) {
            lb += " ";
            lb += m_outputs.at(portIndex)->portName();
        }
        if (!elm->genericProperties().isEmpty()) {
            lb += " [" + elm->genericProperties() + "]";
        }
        m_outputLabels[portIndex] = lb;
    }
}

void ICPrototypeImpl::loadInputElement(GraphicElement *elm)
{
    const auto outputs = elm->outputs();
    for (auto *port : outputs) {
        auto *nodeElm = ElementFactory::buildElement(ElementType::Node);
        nodeElm->setPos(elm->pos());
        nodeElm->setLabel(elm->label());
        if (elm->label().isEmpty()) {
            nodeElm->setLabel(ElementFactory::typeToText(elm->elementType()));
        }
        auto *nodeInput = nodeElm->input();
        nodeInput->setPos(port->pos());
        nodeInput->setName(port->name());
        nodeInput->setRequired(false);
        nodeInput->setDefaultValue(port->value());
        nodeInput->setValue(port->value());
        if (elm->elementType() == ElementType::Clock) {
            nodeInput->setRequired(true);
        }
        m_inputs.append(nodeInput);
        m_elements.append(nodeElm);
        auto conns = port->connections();
        for (auto *conn : conns) {
            if (port == conn->start()) {
                conn->setStart(nodeElm->output());
            }
        }
    }
    elm->disable(); // Maybe I will have to enable and disable it as well.
}

void ICPrototypeImpl::loadOutputElement(GraphicElement *elm)
{
    const auto inputs = elm->inputs();
    for (auto *port : inputs) {
        auto *nodeElm = ElementFactory::buildElement(ElementType::Node);
        nodeElm->setPos(elm->pos());
        nodeElm->setLabel(elm->label());
        if (elm->label().isEmpty()) {
            nodeElm->setLabel(ElementFactory::typeToText(elm->elementType()));
        }
        auto *nodeOutput = nodeElm->output();
        nodeOutput->setPos(port->pos());
        nodeOutput->setName(port->name());
        m_outputs.append(nodeOutput);
        m_elements.append(nodeElm);
        auto conns = port->connections();
        for (auto *conn : conns) {
            if (port == conn->end()) {
                conn->setEnd(nodeElm->input());
            }
        }
    }
}

void ICPrototypeImpl::loadItem(QGraphicsItem *item)
{
    if (item->type() == GraphicElement::Type) {
        auto *elm = qgraphicsitem_cast<GraphicElement *>(item);
        if (elm) {
            if (elm->elementGroup() == ElementGroup::Input) {
                loadInputElement(elm);
            } else if (elm->elementGroup() == ElementGroup::Output) {
                loadOutputElement(elm);
            } else {
                m_elements.append(elm);
            }
        }
    }
}

void ICPrototypeImpl::clear()
{
    m_inputs.clear();
    m_outputs.clear();
    setInputSize(0);
    setOutputSize(0);
    qDeleteAll(m_elements);
    m_elements.clear();
}

int ICPrototypeImpl::inputSize() const
{
    return m_inputs.size();
}

int ICPrototypeImpl::outputSize() const
{
    return m_outputs.size();
}

void ICPrototypeImpl::setOutputSize(const int outputSize)
{
    m_outputLabels = QVector<QString>(outputSize);
}

void ICPrototypeImpl::setInputSize(const int inputSize)
{
    m_inputLabels = QVector<QString>(inputSize);
}

GraphicElement *ICPrototypeImpl::element(const int index)
{
    return m_elements[index];
}

QString ICPrototypeImpl::inputLabel(const int index) const
{
    return m_inputLabels[index];
}

QString ICPrototypeImpl::outputLabel(const int index) const
{
    return m_outputLabels[index];
}

QNEPort *ICPrototypeImpl::input(const int index)
{
    return m_inputs[index];
}

QNEPort *ICPrototypeImpl::output(const int index)
{
    return m_outputs[index];
}

ICMapping *ICPrototypeImpl::generateMapping() const
{
    return new ICMapping(m_elements, m_inputs, m_outputs);
}
