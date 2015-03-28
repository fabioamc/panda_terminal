#ifndef GRAPHICELEMENT_H
#define GRAPHICELEMENT_H

#include <QGraphicsPixmapItem>
#include <QGraphicsItem>

#include "nodes/qneport.h"

enum class ElementType {
  EMPTY, UNKNOWN, BUTTON, SWITCH, LED, NOT, AND, OR, NAND, NOR, CLOCK, XOR, XNOR, VCC, GND,
  WIRE, DLATCH, SRLATCH, SCRLATCH
};

class GraphicElement : public QObject, public QGraphicsItem {
  Q_OBJECT
public:
  enum { Type = QGraphicsItem::UserType + 3 };

  explicit GraphicElement(QPixmap pixmap, QGraphicsItem * parent = 0);
  explicit GraphicElement(int minInputSz, int maxInputSz, int minOutputSz, int maxOutputSz, QGraphicsItem * parent = 0);
  ~GraphicElement();

private:
  QGraphicsPixmapItem *pixmapItem;
  int m_id;
  // QGraphicsItem interface
public:
  virtual QRectF boundingRect() const;
  virtual void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget);
  QNEPort * addPort(bool isOutput);
  // QGraphicsItem interface

  int type() const {
    return Type;
  }

  int topPosition() const;

  int bottomPosition() const;

  int maxInputSz() const;

  int maxOutputSz() const;

  bool outputsOnTop() const;

  QVector<QNEPort *> inputs() const;
  void setInputs(const QVector<QNEPort *> & inputs);

  QVector<QNEPort *> outputs() const;
  void setOutputs(const QVector<QNEPort *> & outputs);

  int minInputSz() const;

  int minOutputSz() const;

  int id() const;
  void setId(int value);

  void setPixmap(const QPixmap &pixmap);

  void updatePorts();

  bool rotatable() const;

  bool hasLabel() const;

  bool hasFrequency() const;

  bool hasColors() const;

protected:
  void setRotatable(bool rotatable);
  void setHasLabel(bool hasLabel);
  void setHasFrequency(bool hasFrequency);
  void setHasColors(bool hasColors);
  void setMinInputSz(int minInputSz);
  void setMinOutputSz(int minOutputSz);
  void setOutputsOnTop(bool outputsOnTop);
  void setMaxOutputSz(int maxOutputSz);
  void setMaxInputSz(int maxInputSz);
  void setTopPosition(int topPosition);
  void setBottomPosition(int bottomPosition);

  virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *e);
  QVariant itemChange(GraphicsItemChange change, const QVariant &value);
private:
  int m_topPosition;
  int m_bottomPosition;
  int m_maxInputSz;
  int m_maxOutputSz;
  int m_minInputSz;
  int m_minOutputSz;
  bool m_outputsOnTop;
  bool m_rotatable;
  bool m_hasLabel;
  bool m_hasFrequency;
  bool m_hasColors;
  QVector<QNEPort*> m_inputs;
  QVector<QNEPort*> m_outputs;
};

#endif // GRAPHICELEMENT_H