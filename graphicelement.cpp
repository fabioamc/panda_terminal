#include "graphicelement.h"

#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QMessageBox>
#include <QPainter>

#include <nodes/qneconnection.h>

GraphicElement::GraphicElement(QPixmap pixmap, QGraphicsItem *parent) : QGraphicsObject(parent), pixmapItem(new QGraphicsPixmapItem(pixmap, ( QGraphicsItem * ) this)) {
  setFlag(QGraphicsItem::ItemIsMovable);
  setFlag(QGraphicsItem::ItemIsSelectable);
  setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
  setFlag(QGraphicsItem::ItemSendsGeometryChanges);
  m_topPosition = 0;
  m_bottomPosition = 64;
  m_maxInputSz = 8;
  m_maxOutputSz = 1;
  m_outputsOnTop = true;
  m_rotatable = true;
  m_changed = true;
  m_visited = false;
  m_beingVisited = false;
}

GraphicElement::GraphicElement(int minInputSz, int maxInputSz, int minOutputSz, int maxOutputSz, QGraphicsItem * parent) : QGraphicsObject(parent), pixmapItem(new QGraphicsPixmapItem( ( QGraphicsItem * ) this)) {
//  setFlag(QGraphicsItem::ItemIsMovable);
  setFlag(QGraphicsItem::ItemIsSelectable);
  setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
  setFlag(QGraphicsItem::ItemSendsGeometryChanges);
  m_bottomPosition = 64;
  m_topPosition = 0;
  m_minInputSz = minInputSz;
  m_minOutputSz = minOutputSz;
  m_maxInputSz = maxInputSz;
  m_maxOutputSz = maxOutputSz;
  m_changed = true;
  m_visited = false;
  m_beingVisited = false;
  m_rotatable = true;
  for(int i = 0; i < minInputSz; i++) {
    addInputPort();
  }
  for(int i = 0; i < minOutputSz; i++) {
    addOutputPort();
  }
  m_outputsOnTop = true;
}

GraphicElement::~GraphicElement() {

}

int GraphicElement::id() const {
  return m_id;
}

void GraphicElement::setId(int value) {
  m_id = value;
}

void GraphicElement::setPixmap(const QPixmap & pixmap) {
  pixmapItem->setPixmap(pixmap);
}

QVector<QNEPort *> GraphicElement::outputs() const {
  return m_outputs;
}

void GraphicElement::setOutputs(const QVector<QNEPort *> & outputs) {
  m_outputs = outputs;
}

void GraphicElement::save(QDataStream &ds) {
  ds << pos();
  ds << rotation();
  ds << (quint64) m_inputs.size();
  foreach (QNEPort * port, m_inputs) {
    ds << (quint64) port;
    ds << port->portName();
    ds << port->portFlags();
  }
  ds << (quint64) m_outputs.size();
  foreach (QNEPort * port, m_outputs) {
    ds << (quint64) port;
    ds << port->portName();
    ds << port->portFlags();
  }
}

void GraphicElement::load(QDataStream &ds, QMap<quint64, QNEPort *> & portMap) {
  QPointF p;
  ds >> p;
  qreal angle;
  ds >> angle;
  setRotation(angle);
  setPos(p);
  quint64 inputSz, outputSz;
  ds >> inputSz;
  for( size_t port = 0; port < inputSz; ++port ) {
    QString name;
    int flags;

    quint64 ptr;
    ds >> ptr;
    ds >> name;
    ds >> flags;
    if( port < (size_t) minInputSz() ) {
      portMap[ptr] = m_inputs[port];
      m_inputs[port]->setName(name);
      m_inputs[port]->setPortFlags(flags);
      m_inputs[port]->setPtr(ptr);
    } else {
      portMap[ptr] = addPort(name,false,flags,ptr);
    }
  }
  ds >> outputSz;
  for( size_t port = 0; port < outputSz; ++port ) {
    QString name;
    int flags;
    quint64 ptr;
    ds >> ptr;
    ds >> name;
    ds >> flags;
    if( port < (size_t) minOutputSz() ) {
      portMap[ptr] = m_outputs[port];
      m_outputs[port]->setName(name);
      m_outputs[port]->setPortFlags(flags);
      m_outputs[port]->setPtr(ptr);
    } else {
      portMap[ptr] = addPort(name,true,flags,ptr);
    }
  }
}

QVector<QNEPort *> GraphicElement::inputs() const {
  return m_inputs;
}

void GraphicElement::setInputs(const QVector<QNEPort *> & inputs) {
  m_inputs = inputs;
}


QRectF GraphicElement::boundingRect() const {
  return (pixmapItem->boundingRect());
}

void GraphicElement::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget) {
  Q_UNUSED(option)
  Q_UNUSED(widget)
  if(isSelected()) {
    painter->setBrush(Qt::darkGray);
    painter->setPen(QPen(Qt::black));
    painter->drawRoundedRect(boundingRect(),16,16);
  }
}

QNEPort *GraphicElement::addPort(const QString & name, bool isOutput, int flags, int ptr) {
  if(isOutput && m_outputs.size() >= m_maxOutputSz) {
    return NULL;
  } else if(!isOutput && m_inputs.size() >= m_maxInputSz) {
    return NULL;
  }
  QNEPort *port = new QNEPort(this);
  port->setName(name);
  port->setIsOutput(isOutput);
  port->setGraphicElement(this);
  port->setPortFlags(flags);
  port->setPtr(ptr);
  if(isOutput) {
    m_outputs.push_back(port);
  } else {
    m_inputs.push_back(port);
  }
  this->updatePorts();
  port->show();
  return port;
}

void GraphicElement::addInputPort(const QString & name ) {
  addPort(name, false);
}

void GraphicElement::addOutputPort(const QString & name) {
  addPort(name, true);
}

void GraphicElement::updatePorts() {
  int inputPos = m_topPosition;
  int outputPos = m_bottomPosition;
  if(m_outputsOnTop) {
    inputPos = m_bottomPosition;
    outputPos = m_topPosition;
  }
  if(!m_outputs.isEmpty()) {
    int step = qMax(32/m_outputs.size(), 6);
    int x = 32 - m_outputs.size()*step + step;
    foreach (QNEPort * port, m_outputs) {
      port->setPos(x,outputPos);
      x+= step * 2;
    }
  }
  if(!m_inputs.isEmpty()) {
    int step = qMax(32/m_inputs.size(),6);
    int x = 32 - m_inputs.size()*step + step;
    foreach (QNEPort * port, m_inputs) {
      port->setPos(x,inputPos);
      x+= step * 2;
    }
  }
}

void GraphicElement::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * e) {
  if(e->button() == Qt::LeftButton) {
    addOutputPort();
  } else {
    addInputPort();
  }
}

QVariant GraphicElement::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant & value) {
  if(change == ItemScenePositionHasChanged ||  change == ItemRotationHasChanged ||  change == ItemTransformHasChanged) {
    foreach (QNEPort * port, m_outputs) {
      port->updateConnections();
    }
    foreach (QNEPort * port, m_inputs) {
      port->updateConnections();
    }
  }
  return value;
}

void GraphicElement::updateLogic() {

}

bool GraphicElement::visited() const {
  return m_visited;
}

void GraphicElement::setVisited(bool visited) {
  m_visited = visited;
}

bool GraphicElement::isValid() {
  bool valid = true;
  foreach (QNEPort * input, inputs()) {
    if((input->connections().size() != 1)) {
      foreach (QNEConnection *conn, input->connections()) {
        conn->setStatus(QNEConnection::Invalid);
      }
      valid = false;
    }
  }
  return valid;
}

bool GraphicElement::beingVisited() const {
  return m_beingVisited;
}

void GraphicElement::setBeingVisited(bool beingVisited) {
  m_beingVisited = beingVisited;
}

bool GraphicElement::changed() const {
  return m_changed;
}

void GraphicElement::setChanged(bool changed) {
  m_changed = changed;
}

bool GraphicElement::hasColors() const {
  return m_hasColors;
}

void GraphicElement::setHasColors(bool hasColors) {
  m_hasColors = hasColors;
}

bool GraphicElement::hasFrequency() const {
  return m_hasFrequency;
}

void GraphicElement::setHasFrequency(bool hasFrequency) {
  m_hasFrequency = hasFrequency;
}

bool GraphicElement::hasLabel() const {
  return m_hasLabel;
}

void GraphicElement::setHasLabel(bool hasLabel) {
  m_hasLabel = hasLabel;
}

bool GraphicElement::rotatable() const {
  return m_rotatable;
}

void GraphicElement::setRotatable(bool rotatable) {
  m_rotatable = rotatable;
}

int GraphicElement::minOutputSz() const {
  return m_minOutputSz;
}

void GraphicElement::setMinOutputSz(int minOutputSz) {
  m_minOutputSz = minOutputSz;
}

int GraphicElement::minInputSz() const {
  return m_minInputSz;
}

void GraphicElement::setMinInputSz(int minInputSz) {
  m_minInputSz = minInputSz;
}


int GraphicElement::maxOutputSz() const {
  return m_maxOutputSz;
}

void GraphicElement::setMaxOutputSz(int maxOutputSz) {
  m_maxOutputSz = maxOutputSz;
}

int GraphicElement::maxInputSz() const {
  return m_maxInputSz;
}

void GraphicElement::setMaxInputSz(int maxInputSz) {
  m_maxInputSz = maxInputSz;
}

int GraphicElement::bottomPosition() const {
  return m_bottomPosition;
}

void GraphicElement::setBottomPosition(int bottomPosition) {
  m_bottomPosition = bottomPosition;
  updatePorts();
}

int GraphicElement::topPosition() const {
  return m_topPosition;
}

void GraphicElement::setTopPosition(int topPosition) {
  m_topPosition = topPosition;
  updatePorts();
}

bool GraphicElement::outputsOnTop() const {
  return m_outputsOnTop;
}

void GraphicElement::setOutputsOnTop(bool outputsOnTop) {
  m_outputsOnTop = outputsOnTop;
  updatePorts();
}
