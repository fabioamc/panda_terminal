#ifndef SERIALIZATIONFUNCTIONS_H
#define SERIALIZATIONFUNCTIONS_H

#include <QGraphicsItem>

class Editor;
class Scene;
class GraphicElement;

class SerializationFunctions {
public:
  static void serialize( const QList< QGraphicsItem* > &items, QDataStream &ds );
  static QList< QGraphicsItem* > deserialize(QDataStream &ds,
                                              double version,
                                              QString parentFile,
                                              QMap< quint64, QNEPort* > portMap = QMap< quint64, QNEPort* >( ) );
  static QList< QGraphicsItem* > load( Editor *editor, QDataStream &ds, QString parentFile, Scene *scene = nullptr );
  static QVector< GraphicElement* > duplicate(const QVector< GraphicElement* > &elements );
};

#endif /* SERIALIZATIONFUNCTIONS_H */
