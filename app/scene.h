/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "icmanager.h"
#include "qneport.h"
#include "simulationcontroller.h"

#include <QElapsedTimer>
#include <QGraphicsScene>
#include <QUndoCommand>

class GraphicElement;
class QNEConnection;

class Scene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit Scene(QObject *parent = nullptr);

    QAction *redoAction() const;
    QAction *undoAction() const;
    QUndoStack *undoStack();
    QVector<GraphicElement *> elements();
    QVector<GraphicElement *> elements(const QRectF &rect);
    QVector<GraphicElement *> selectedElements();
    QVector<GraphicElement *> visibleElements();
    SimulationController *simulationController();
    int gridSize() const;
    void copyAction();
    void cutAction();
    void deleteAction();
    void flipHorizontally();
    void flipVertically();
    void mute(const bool mute = true);
    void pasteAction();
    void receiveCommand(QUndoCommand *cmd);
    void rotateLeft();
    void rotateRight();
    void selectAll();
    void setAutosaveRequired();
    void setCircuitUpdateRequired();
    void setHandlingEvents(const bool value);
    void showGates(const bool checked);
    void showWires(const bool checked);
    void updateTheme();

signals:
    void circuitAppearenceHasChanged();
    void circuitHasChanged();
    void contextMenuPos(QPoint screenPos); // TODO: instead of sending position send item?

protected:
    void dragEnterEvent(QGraphicsSceneDragDropEvent *event) override;
    void dragMoveEvent(QGraphicsSceneDragDropEvent *event) override;
    void dropEvent(QGraphicsSceneDragDropEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    static constexpr int m_gridSize = 16;

    QGraphicsItem *itemAt(const QPointF pos);
    QList<QGraphicsItem *> itemsAt(const QPointF pos);
    QNEConnection *editedConnection() const;
    QNEPort *hoverPort();
    QVector<QNEConnection *> connections();
    void checkUpdateRequest();
    void cloneDrag(const QPointF pos);
    void contextMenu(const QPoint screenPos);
    void copy(const QList<QGraphicsItem *> &items, QDataStream &stream);
    void cut(const QList<QGraphicsItem *> &items, QDataStream &stream);
    void deleteEditedConnection();
    void detachConnection(QNEInputPort *endPort);
    void drawBackground(QPainter *painter, const QRectF &rect) override;
    void handleHoverPort();
    void makeConnection(QNEConnection *connection);
    void paste(QDataStream &stream);
    void redoSimulationController();
    void releaseHoverPort();
    void resizeScene();
    void setDots(const QPen &dots);
    void setEditedConnection(QNEConnection *connection);
    void setHoverPort(QNEPort *port);
    void startNewConnection(QNEInputPort *endPort);
    void startNewConnection(QNEOutputPort *startPort);
    void startSelectionRect();

    // TODO: store a pointer to the view?
    QAction *m_redoAction;
    QAction *m_undoAction;
    QElapsedTimer m_timer;
    QGraphicsRectItem m_selectionRect;
    QList<GraphicElement *> m_movedElements;
    QList<QPointF> m_oldPositions;
    QPen m_dots;
    QPointF m_mousePos;
    QPointF m_selectionStartPoint;
    QUndoStack m_undoStack;
    SimulationController m_simulationController;
    bool m_autosaveRequired = false;
    bool m_circuitUpdateRequired = false;
    bool m_draggingElement = false;
    bool m_handlingEvents = true;
    bool m_markingSelectionBox = false;
    bool m_showGates = true;
    bool m_showWires = true;
    int m_editedConnection_id = 0;
    int m_hoverPortElm_id = 0;
    int m_hoverPort_nbr = 0;
};

