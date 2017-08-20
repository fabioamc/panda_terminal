#ifndef UTIL_H
#define UTIL_H

#include "graphicelement.h"

#include <QVector>


namespace Util {
  enum SortingKind { INCREASING, DECREASING, POSITION };

  QVector< GraphicElement* > sortElements( QVector< GraphicElement* > elms, bool &notSequential );
  QVector< GraphicElement* > sortElements( QVector< GraphicElement* > elms );

  QVector<GraphicElement *> sortElements(QVector<GraphicElement *> elements,
                     QVector< GraphicElement* > &inputs,
                     QVector< GraphicElement* > &outputs,
                     SortingKind sorting = SortingKind::POSITION );

  int calculatePriority( GraphicElement *elm,
                         QMap< GraphicElement*, bool > &beingvisited,
                         QMap< GraphicElement*, int > &priority,
                         bool &isNotSequential );

  QVector< GraphicElement* > elementList( QVector< QGraphicsItem* > items );

  QVector< GraphicElement* > elementList( QList< QGraphicsItem* > items );

  QVector< QNEConnection* > getConnections( const QVector< GraphicElement* > &elements );


}
#endif // UTIL_H
