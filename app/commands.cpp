// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "commands.h"

#include "common.h"
#include "elementfactory.h"
#include "globalproperties.h"
#include "graphicelement.h"
#include "qneconnection.h"
#include "qneport.h"
#include "scene.h"
#include "serializationfunctions.h"
#include "simulationcontroller.h"

#include <QDrag>
#include <QGraphicsItem>
#include <QIODevice>
#include <cmath>
#include <stdexcept>

void storeIds(const QList<QGraphicsItem *> &items, QVector<int> &ids)
{
    ids.reserve(items.size());
    for (auto *item : items) {
        auto *itemId = dynamic_cast<ItemWithId *>(item);
        if (itemId) {
            ids.append(itemId->id());
        }
    }
}

void storeOtherIds(const QList<QGraphicsItem *> &connections, const QVector<int> &ids, QVector<int> &otherIds)
{
    for (auto *item : connections) {
        auto *conn = qgraphicsitem_cast<QNEConnection *>(item);
        if ((item->type() == QNEConnection::Type) && conn) {
            auto *p1 = conn->start();
            if (p1 && p1->graphicElement() && !ids.contains(p1->graphicElement()->id())) {
                otherIds.append(p1->graphicElement()->id());
            }
            auto *p2 = conn->end();
            if (p2 && p2->graphicElement() && !ids.contains(p2->graphicElement()->id())) {
                otherIds.append(p2->graphicElement()->id());
            }
        }
    }
}

QList<QGraphicsItem *> loadList(const QList<QGraphicsItem *> &items, QVector<int> &ids, QVector<int> &otherIds)
{
    QList<QGraphicsItem *> elements;
    /* Stores selected graphicElements */
    for (auto *item : items) {
        if (item->type() == GraphicElement::Type) {
            if (!elements.contains(item)) {
                elements.append(item);
            }
        }
    }
    QList<QGraphicsItem *> connections;
    /* Stores all the wires linked to these elements */
    for (auto *item : elements) {
        auto *elm = qgraphicsitem_cast<GraphicElement *>(item);
        if (elm) {
            const auto inputs = elm->inputs();
            for (auto *port : inputs) {
                for (auto *conn : port->connections()) {
                    if (!connections.contains(conn)) {
                        connections.append(conn);
                    }
                }
            }
            const auto outputs = elm->outputs();
            for (auto *port : outputs) {
                for (auto *conn : port->connections()) {
                    if (!connections.contains(conn)) {
                        connections.append(conn);
                    }
                }
            }
        }
    }
    /* Stores the other wires selected */
    for (auto *item : items) {
        if (item->type() == QNEConnection::Type) {
            if (!connections.contains(item)) {
                connections.append(item);
            }
        }
    }
    /* Stores the ids of all elements listed in items; */
    storeIds(elements + connections, ids);
    /* Stores all the elements linked to each connection that will not be deleted. */
    storeOtherIds(connections, ids, otherIds);
    return elements + connections;
}

QList<QGraphicsItem *> findItems(const QVector<int> &ids)
{
    QList<QGraphicsItem *> items;
    items.reserve(ids.size());
    for (const int id : ids) {
        auto *item = dynamic_cast<QGraphicsItem *>(ElementFactory::itemById(id));
        if (item) {
            items.append(item);
        }
    }
    if (items.size() != ids.size()) {
        throw Pandaception(QObject::tr("One or more items was not found on the scene."));
    }
    return items;
}

QList<GraphicElement *> findElements(const QVector<int> &ids)
{
    QList<GraphicElement *> items;
    items.reserve(ids.size());
    for (const int id : ids) {
        auto *item = dynamic_cast<GraphicElement *>(ElementFactory::itemById(id));
        if (item) {
            items.append(item);
        }
    }
    if (items.size() != ids.size()) {
        throw Pandaception(QObject::tr("One or more elements was not found on the scene."));
    }
    return items;
}

void saveItems(QByteArray &itemData, const QList<QGraphicsItem *> &items, const QVector<int> &otherIds)
{
    itemData.clear();
    QDataStream stream(&itemData, QIODevice::WriteOnly);
    auto others = findElements(otherIds);
    for (auto *elm : qAsConst(others)) {
        elm->save(stream);
    }
    SerializationFunctions::serialize(items, stream);
}

void addItems(Scene *scene, const QList<QGraphicsItem *> &items)
{
    scene->clearSelection();
    for (auto *item : items) {
        if (item->scene() != scene) {
            scene->addItem(item);
        }
        if (item->type() == GraphicElement::Type) {
            item->setSelected(true);
        }
    }
}

QList<QGraphicsItem *> loadItems(Scene *scene, QByteArray &itemData, const QVector<int> &ids, QVector<int> &otherIds)
{
    if (itemData.isEmpty()) {
        return {};
    }
    auto otherElms = findElements(otherIds).toVector();
    QDataStream stream(&itemData, QIODevice::ReadOnly);
    double version = GlobalProperties::version;
    QMap<quint64, QNEPort *> portMap;
    for (auto *elm : qAsConst(otherElms)) {
        elm->load(stream, portMap, version);
    }
    /*
     * Assuming that all connections are stored after the elements, we will deserialize the elements first.
     * We will store one additional information: The element IDs!
     */
    auto items = SerializationFunctions::deserialize(stream, version, portMap);
    if (items.size() != ids.size()) {
        throw Pandaception(QObject::tr("One or more elements were not found on scene. Expected %1, found %2.").arg(ids.size(), items.size()));
    }
    for (int i = 0; i < items.size(); ++i) {
        auto *itemId = dynamic_cast<ItemWithId *>(items[i]);
        if (itemId) {
            ElementFactory::updateItemId(itemId, ids[i]);
        }
    }
    addItems(scene, items);
    return items;
}

void deleteItems(Scene *scene, const QList<QGraphicsItem *> &items)
{
    /* Delete items on reverse order */
    for (int i = items.size() - 1; i >= 0; --i) {
        scene->removeItem(items[i]);
        delete items[i];
    }
}

AddItemsCommand::AddItemsCommand(const QList<QGraphicsItem *> &items, Scene *scene, QUndoCommand *parent)
    : QUndoCommand(parent)
{
    auto items_ = loadList(items, m_ids, m_otherIds);
    m_scene = scene;
    addItems(m_scene, items_);
    setText(tr("Add %1 elements").arg(items.size()));
}

void AddItemsCommand::undo()
{
    qCDebug(zero) << "UNDO " + text();
    auto items = findItems(m_ids);

    auto *simController = m_scene->simulationController();
    // We need to restart the simulation controller when deleting through the Undo command to
    // guarantee that no crashes occur when deleting input elements (clocks, input buttons, etc.)
    simController->setRestart();

    saveItems(m_itemData, items, m_otherIds);
    deleteItems(m_scene, items);
    m_scene->setCircuitUpdateRequired();
}

void AddItemsCommand::redo()
{
    qCDebug(zero) << "REDO " + text();
    loadItems(m_scene, m_itemData, m_ids, m_otherIds);
    m_scene->setCircuitUpdateRequired();
}

DeleteItemsCommand::DeleteItemsCommand(const QList<QGraphicsItem *> &items, Scene *scene, QUndoCommand *parent)
    : QUndoCommand(parent)
{
    auto items_ = loadList(items, m_ids, m_otherIds);
    m_scene = scene;
    setText(tr("Delete %1 elements").arg(items_.size()));
}

void DeleteItemsCommand::undo()
{
    qCDebug(zero) << "UNDO " + text();
    loadItems(m_scene, m_itemData, m_ids, m_otherIds);
    m_scene->setCircuitUpdateRequired();
}

void DeleteItemsCommand::redo()
{
    qCDebug(zero) << "REDO " + text();
    auto items = findItems(m_ids);
    saveItems(m_itemData, items, m_otherIds);
    deleteItems(m_scene, items);
    m_scene->setCircuitUpdateRequired();
}

RotateCommand::RotateCommand(const QList<GraphicElement *> &items, const int angle, Scene *scene, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_scene(scene)
    , m_angle(angle)
{
    setText(tr("Rotate %1 degrees").arg(m_angle));
    m_ids.reserve(items.size());
    m_positions.reserve(items.size());
    for (auto *item : items) {
        m_positions.append(item->pos());
        item->setPos(item->pos());
        m_ids.append(item->id());
    }
}

void RotateCommand::undo()
{
    qCDebug(zero) << "UNDO " + text();
    auto list = findElements(m_ids);
    auto *scene = list.at(0)->scene();
    scene->clearSelection();
    for (int i = 0; i < list.size(); ++i) {
        auto *elm = list[i];
        if (elm->rotatable()) {
            elm->setRotation(elm->rotation() - m_angle);
        }
        elm->setPos(m_positions[i]);
        elm->update();
        elm->setSelected(true);
    }
    m_scene->setAutosaveRequired();
}

void RotateCommand::redo()
{
    qCDebug(zero) << "REDO " + text();
    auto list = findElements(m_ids);
    auto *scene = list[0]->scene();
    scene->clearSelection();
    double cx = 0;
    double cy = 0;
    int sz = 0;
    for (auto *item : qAsConst(list)) {
        cx += item->pos().x();
        cy += item->pos().y();
        sz++;
    }
    if (sz != 0) {
        cx /= sz;
        cy /= sz;
    }
    for (auto *elm : qAsConst(list)) {
        QTransform transform;
        transform.translate(cx, cy);
        transform.rotate(m_angle);
        transform.translate(-cx, -cy);
        if (elm->rotatable()) {
            elm->setRotation(elm->rotation() + m_angle);
        }
        elm->setPos(transform.map(elm->pos()));
        elm->update();
        elm->setSelected(true);
    }
    m_scene->setAutosaveRequired();
}

MoveCommand::MoveCommand(const QList<GraphicElement *> &list, const QList<QPointF> &oldPositions, Scene *scene, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_oldPositions(oldPositions)
    , m_scene(scene)
{
    m_newPositions.reserve(list.size());
    m_ids.reserve(list.size());
    for (auto *elm : list) {
        m_ids.append(elm->id());
        m_newPositions.append(elm->pos());
    }
    setText(tr("Move elements"));
}

void MoveCommand::undo()
{
    qCDebug(zero) << "UNDO " + text();
    auto elms = findElements(m_ids).toVector();
    for (int i = 0; i < elms.size(); ++i) {
        elms[i]->setPos(m_oldPositions[i]);
    }
    m_scene->setAutosaveRequired();
}

void MoveCommand::redo()
{
    qCDebug(zero) << "REDO " + text();
    auto elms = findElements(m_ids).toVector();
    for (int i = 0; i < elms.size(); ++i) {
        elms[i]->setPos(m_newPositions[i]);
    }
    m_scene->setAutosaveRequired();
}

UpdateCommand::UpdateCommand(const QVector<GraphicElement *> &elements, const QByteArray &oldData, Scene *scene, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_oldData(oldData),
      m_scene(scene)
{
    ids.reserve(elements.size());
    QDataStream stream(&m_newData, QIODevice::WriteOnly);
    for (auto *elm : elements) {
        elm->save(stream);
        ids.append(elm->id());
    }
    setText(tr("Update %1 elements").arg(elements.size()));
}

void UpdateCommand::undo()
{
    qCDebug(zero) << "UNDO " + text();
    loadData(m_oldData);
    m_scene->setCircuitUpdateRequired();
}

void UpdateCommand::redo()
{
    qCDebug(zero) << "REDO " + text();
    loadData(m_newData);
    m_scene->setCircuitUpdateRequired();
}

void UpdateCommand::loadData(QByteArray itemData)
{
    auto elements = findElements(ids).toVector();
    QDataStream stream(&itemData, QIODevice::ReadOnly);
    QMap<quint64, QNEPort *> portMap;
    if (!elements.isEmpty() && elements.first()->scene()) {
        elements.first()->scene()->clearSelection();
    }
    double version = GlobalProperties::version;
    if (!elements.isEmpty()) {
        for (auto *elm : qAsConst(elements)) {
            elm->load(stream, portMap, version);
            elm->setSelected(true);
        }
    }
}

SplitCommand::SplitCommand(QNEConnection *conn, QPointF mousePos, Scene *scene, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_scene(scene)
{
    auto *node = ElementFactory::buildElement(ElementType::Node);
    auto *conn2 = ElementFactory::buildConnection();

    /* Align node to Grid */
    m_nodePos = mousePos - node->boundingRect().center();
    if (m_scene) {
        int gridSize = m_scene->gridSize();
        qreal xV = qRound(m_nodePos.x() / gridSize) * gridSize;
        qreal yV = qRound(m_nodePos.y() / gridSize) * gridSize;
        m_nodePos = QPointF(xV, yV);
    }
    /* Rotate line according to angle between p1 and p2 */
    m_nodeAngle = static_cast<int>(conn->angle());
    m_nodeAngle = static_cast<int>(360 - 90 * (std::round(m_nodeAngle / 90.0)));

    /* Assigning class attributes */
    m_elm1_id = conn->start()->graphicElement()->id();
    m_elm2_id = conn->end()->graphicElement()->id();

    m_c1_id = conn->id();
    m_c2_id = conn2->id();

    m_node_id = node->id();

    setText(tr("Wire split"));
}

QNEConnection *findConn(const int id)
{
    return dynamic_cast<QNEConnection *>(ElementFactory::itemById(id));
}

GraphicElement *findElm(const int id)
{
    return dynamic_cast<GraphicElement *>(ElementFactory::itemById(id));
}

void SplitCommand::redo()
{
    qCDebug(zero) << "REDO " + text();
    auto *conn1 = findConn(m_c1_id);
    auto *conn2 = findConn(m_c2_id);
    auto *node = findElm(m_node_id);
    auto *elm1 = findElm(m_elm1_id);
    auto *elm2 = findElm(m_elm2_id);
    if (!conn2) {
        conn2 = ElementFactory::buildConnection();
        ElementFactory::updateItemId(conn2, m_c2_id);
    }
    if (!node) {
        node = ElementFactory::buildElement(ElementType::Node);
        ElementFactory::updateItemId(node, m_node_id);
    }
    if (conn1 && conn2 && elm1 && elm2 && node) {
        node->setPos(m_nodePos);
        node->setRotation(m_nodeAngle);

        // auto *startPort = c1->start();
        auto *endPort = conn1->end();
        conn2->setStart(node->output());
        conn2->setEnd(endPort);
        conn1->setEnd(node->input());

        m_scene->addItem(node);
        m_scene->addItem(conn2);

        conn1->updatePosFromPorts();
        conn1->updatePath();
        conn2->updatePosFromPorts();
        conn2->updatePath();
    } else {
        throw Pandaception(tr("Error trying to redo ") + text());
    }
    m_scene->setCircuitUpdateRequired();
}

void SplitCommand::undo()
{
    qCDebug(zero) << "UNDO " + text();
    auto *conn1 = findConn(m_c1_id);
    auto *conn2 = findConn(m_c2_id);
    auto *node = findElm(m_node_id);
    auto *elm1 = findElm(m_elm1_id);
    auto *elm2 = findElm(m_elm2_id);
    if (conn1 && conn2 && elm1 && elm2 && node) {
        conn1->setEnd(conn2->end());

        conn1->updatePosFromPorts();
        conn1->updatePath();

        m_scene->removeItem(conn2);
        m_scene->removeItem(node);

        delete conn2;
        delete node;
    } else {
        throw Pandaception(tr("Error trying to undo ") + text());
    }
    m_scene->setCircuitUpdateRequired();
}

MorphCommand::MorphCommand(const QVector<GraphicElement *> &elements, ElementType type, Scene *scene, QUndoCommand *parent)
    : QUndoCommand(parent)
{
    m_newtype = type;
    m_scene = scene;
    m_ids.reserve(elements.size());
    m_types.reserve(elements.size());
    for (auto *oldElm : elements) {
        m_ids.append(oldElm->id());
        m_types.append(oldElm->elementType());
    }
    setText(tr("Morph %1 elements to %2").arg(elements.size()).arg(elements.first()->objectName()));
}

void MorphCommand::undo()
{
    qCDebug(zero) << "UNDO " + text();
    QVector<GraphicElement *> newElms = findElements(m_ids).toVector();
    QVector<GraphicElement *> oldElms(newElms.size());
    for (int i = 0; i < m_ids.size(); ++i) {
        oldElms[i] = ElementFactory::buildElement(m_types[i]);
    }
    transferConnections(newElms, oldElms);
    m_scene->setCircuitUpdateRequired();
}

void MorphCommand::redo()
{
    qCDebug(zero) << "REDO " + text();
    QVector<GraphicElement *> oldElms = findElements(m_ids).toVector();
    QVector<GraphicElement *> newElms(oldElms.size());
    for (int i = 0; i < m_ids.size(); ++i) {
        newElms[i] = ElementFactory::buildElement(m_newtype);
    }
    transferConnections(oldElms, newElms);
    m_scene->setCircuitUpdateRequired();
}

void MorphCommand::transferConnections(QVector<GraphicElement *> from, QVector<GraphicElement *> to)
{
    for (int elm = 0; elm < from.size(); ++elm) {
        GraphicElement *oldElm = from[elm];
        GraphicElement *newElm = to[elm];
        newElm->setInputSize(oldElm->inputSize());

        newElm->setPos(oldElm->pos());
        if (newElm->rotatable() && oldElm->rotatable()) {
            newElm->setRotation(oldElm->rotation());
        }
        if ((newElm->elementType() == ElementType::Not) && (oldElm->elementType() == ElementType::Node)) {
            newElm->setRotation(oldElm->rotation() + 90.0);
        } else if ((newElm->elementType() == ElementType::Node) && (oldElm->elementType() == ElementType::Not)) {
            newElm->setRotation(oldElm->rotation() - 90.0);
        }
        if (newElm->hasLabel() && oldElm->hasLabel()) {
            newElm->setLabel(oldElm->label());
        }
        if (newElm->hasColors() && oldElm->hasColors()) {
            newElm->setColor(oldElm->color());
        }
        if (newElm->hasFrequency() && oldElm->hasFrequency()) {
            newElm->setFrequency(oldElm->frequency());
        }
        if (newElm->hasTrigger() && oldElm->hasTrigger()) {
            newElm->setTrigger(oldElm->trigger());
        }
        for (int in = 0; in < oldElm->inputSize(); ++in) {
            while (!oldElm->input(in)->connections().isEmpty()) {
                QNEConnection *conn = oldElm->input(in)->connections().first();
                if (conn->end() == oldElm->input(in)) {
                    conn->setEnd(newElm->input(in));
                }
            }
        }
        for (int out = 0; out < oldElm->outputSize(); ++out) {
            while (!oldElm->output(out)->connections().isEmpty()) {
                QNEConnection *conn = oldElm->output(out)->connections().first();
                if (conn->start() == oldElm->output(out)) {
                    conn->setStart(newElm->output(out));
                }
            }
        }
        int oldId = oldElm->id();
        m_scene->removeItem(oldElm);
        delete oldElm;

        ElementFactory::updateItemId(newElm, oldId);
        m_scene->addItem(newElm);
        newElm->updatePorts();
    }
}

ChangeInputSizeCommand::ChangeInputSizeCommand(const QVector<GraphicElement *> &elements, const int newInputSize, Scene *scene, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_scene(scene)
{
    m_elms.reserve(elements.size());
    for (auto *elm : elements) {
        m_elms.append(elm->id());
    }
    m_newInputSize = newInputSize;
    if (!elements.isEmpty()) {
        m_scene = static_cast<Scene *>(elements.first()->scene());
    }
    setText(tr("Change input size to %1").arg(newInputSize));
}

void ChangeInputSizeCommand::redo()
{
    qCDebug(zero) << "REDO " + text();
    const QVector<GraphicElement *> m_elements = findElements(m_elms).toVector();
    if (!m_elements.isEmpty() && m_elements.first()->scene()) {
        m_scene->clearSelection();
    }
    QVector<GraphicElement *> serializationOrder;
    serializationOrder.reserve(m_elements.size());
    m_oldData.clear();
    QDataStream stream(&m_oldData, QIODevice::WriteOnly);
    for (auto *elm : m_elements) {
        elm->save(stream);
        serializationOrder.append(elm);
        for (int in = m_newInputSize; in < elm->inputSize(); ++in) {
            for (auto *conn : elm->input(in)->connections()) {
                auto *otherPort = conn->otherPort(elm->input(in));
                otherPort->graphicElement()->save(stream);
                serializationOrder.append(otherPort->graphicElement());
            }
        }
    }
    for (auto *elm : m_elements) {
        for (int in = m_newInputSize; in < elm->inputSize(); ++in) {
            while (!elm->input(in)->connections().isEmpty()) {
                auto *conn = elm->input(in)->connections().first();
                conn->save(stream);
                m_scene->removeItem(conn);
                auto *otherPort = conn->otherPort(elm->input(in));
                elm->input(in)->disconnect(conn);
                otherPort->disconnect(conn);
            }
        }
        elm->setInputSize(m_newInputSize);
        elm->setSelected(true);
    }
    m_order.clear();
    for (auto *elm : serializationOrder) {
        m_order.append(elm->id());
    }
    m_scene->setCircuitUpdateRequired();
}

void ChangeInputSizeCommand::undo()
{
    qCDebug(zero) << "UNDO " + text();
    const QVector<GraphicElement *> m_elements = findElements(m_elms).toVector();
    const QVector<GraphicElement *> serializationOrder = findElements(m_order).toVector();
    if (!m_elements.isEmpty() && m_elements.first()->scene()) {
        m_scene->clearSelection();
    }
    QDataStream stream(&m_oldData, QIODevice::ReadOnly);
    double version = GlobalProperties::version;
    QMap<quint64, QNEPort *> portMap;
    for (auto *elm : serializationOrder) {
        elm->load(stream, portMap, version);
    }
    for (auto *elm : m_elements) {
        for (int in = m_newInputSize; in < elm->inputSize(); ++in) {
            auto *conn = ElementFactory::buildConnection();
            conn->load(stream, portMap);
            m_scene->addItem(conn);
        }
        elm->setSelected(true);
    }
    m_scene->setCircuitUpdateRequired();
}

FlipCommand::FlipCommand(const QList<GraphicElement *> &items, const int axis, Scene *scene, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_scene(scene)
    , m_axis(axis)
{
    setText(tr("Flip %1 elements in axis %2").arg(items.size(), axis));
    m_ids.reserve(items.size());
    m_positions.reserve(items.size());
    double xmin;
    double xmax;
    double ymin;
    double ymax;
    if (!items.empty()) {
        xmin = xmax = items.first()->pos().rx();
        ymin = ymax = items.first()->pos().ry();
        for (auto *item : items) {
            m_positions.append(item->pos());
            item->setPos(item->pos());
            m_ids.append(item->id());
            xmin = qMin(xmin, item->pos().rx());
            xmax = qMax(xmax, item->pos().rx());
            ymin = qMin(ymin, item->pos().ry());
            ymax = qMax(ymax, item->pos().ry());
        }
        m_minPos = QPointF(xmin, ymin);
        m_maxPos = QPointF(xmax, ymax);
    }
}

void FlipCommand::undo()
{
    qCDebug(zero) << "UNDO " + text();
    redo();
}

void FlipCommand::redo()
{
    qCDebug(zero) << "REDO " + text();
    // TODO: this might detach the QList
    auto list = findElements(m_ids);
    auto *scene = list[0]->scene();
    scene->clearSelection();
    for (auto *elm : qAsConst(list)) {
        auto pos = elm->pos();
        if (m_axis == 0) {
            pos.setX(m_minPos.rx() + (m_maxPos.rx() - pos.rx()));
        } else {
            pos.setY(m_minPos.ry() + (m_maxPos.ry() - pos.ry()));
        }
        elm->setPos(pos);
        elm->update();
        elm->setSelected(true);
        if (elm->rotatable()) {
            elm->setRotation(elm->rotation() + 180);
        }
    }
    m_scene->setAutosaveRequired();
}

ChangeOutputSizeCommand::ChangeOutputSizeCommand(const QVector<GraphicElement *> &elements, const int newOutputSize, Scene *scene, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_scene(scene)
{
    m_elms.reserve(elements.size());
    for (auto *elm : elements) {
        m_elms.append(elm->id());
    }
    m_newOutputSize = newOutputSize;
    if (!elements.isEmpty()) {
        m_scene = static_cast<Scene *>(elements.first()->scene());
    }
    setText(tr("Change input size to %1").arg(newOutputSize));
}

void ChangeOutputSizeCommand::redo()
{
    qCDebug(zero) << "REDO " + text();
    const QVector<GraphicElement *> m_elements = findElements(m_elms).toVector();
    if (!m_elements.isEmpty() && m_elements.first()->scene()) {
        m_scene->clearSelection();
    }
    QVector<GraphicElement *> serializationOrder;
    m_oldData.clear();
    QDataStream stream(&m_oldData, QIODevice::WriteOnly);
    serializationOrder.reserve(m_elements.size());
    for (auto *elm : m_elements) {
        elm->save(stream);
        serializationOrder.append(elm);
        for (int out = m_newOutputSize; out < elm->outputSize(); ++out) {
            for (auto *conn : elm->output(out)->connections()) {
                auto *otherPort = conn->otherPort(elm->output(out));
                otherPort->graphicElement()->save(stream);
                serializationOrder.append(otherPort->graphicElement());
            }
        }
    }
    for (auto *elm : m_elements) {
        for (int out = m_newOutputSize; out < elm->outputSize(); ++out) {
            while (!elm->output(out)->connections().isEmpty()) {
                auto *conn = elm->output(out)->connections().first();
                conn->save(stream);
                m_scene->removeItem(conn);
                auto *otherPort = conn->otherPort(elm->output(out));
                elm->output(out)->disconnect(conn);
                otherPort->disconnect(conn);
            }
        }
        elm->setOutputSize(m_newOutputSize);
        elm->setSelected(true);
    }
    m_order.clear();
    for (auto *elm : serializationOrder) {
        m_order.append(elm->id());
    }
    m_scene->setCircuitUpdateRequired();
}

void ChangeOutputSizeCommand::undo()
{
    qCDebug(zero) << "UNDO " + text();
    const auto elements = findElements(m_elms);
    const auto serializationOrder = findElements(m_order);
    if (!elements.isEmpty() && elements.first()->scene()) {
        m_scene->clearSelection();
    }
    QDataStream stream(&m_oldData, QIODevice::ReadOnly);
    double version = GlobalProperties::version;
    QMap<quint64, QNEPort *> portMap;
    for (auto *elm : serializationOrder) {
        elm->load(stream, portMap, version);
    }
    for (auto *elm : elements) {
        for (int out = m_newOutputSize; out < elm->outputSize(); ++out) {
            auto *conn = ElementFactory::buildConnection();
            conn->load(stream, portMap);
            m_scene->addItem(conn);
        }
        elm->setSelected(true);
    }
    m_scene->setCircuitUpdateRequired();
}
