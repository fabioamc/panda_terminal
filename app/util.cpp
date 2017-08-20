#include "qneconnection.h"
#include "qneport.h"
#include "util.h"

namespace Util {
  QVector< GraphicElement* > sortElements( QVector< GraphicElement* > elements,
                                           QVector< GraphicElement* > &inputs,
                                           QVector< GraphicElement* > &outputs,
                                           Util::SortingKind sorting ) {
    elements = Util::sortElements( elements );
    inputs.clear( );
    outputs.clear( );
    for( GraphicElement *elm : elements ) {
      if( elm->elementGroup( ) == ElementGroup::INPUT ) {
        inputs.append( elm );
      }
      else if( elm->elementGroup( ) == ElementGroup::OUTPUT ) {
        outputs.append( elm );
      }
    }
    std::stable_sort( inputs.begin( ), inputs.end( ), [ ]( GraphicElement *elm1, GraphicElement *elm2 ) {
      return( elm1->pos( ).ry( ) < elm2->pos( ).ry( ) );
    } );
    std::stable_sort( outputs.begin( ), outputs.end( ), [ ]( GraphicElement *elm1, GraphicElement *elm2 ) {
      return( elm1->pos( ).ry( ) < elm2->pos( ).ry( ) );
    } );

    std::stable_sort( inputs.begin( ), inputs.end( ), [ ]( GraphicElement *elm1, GraphicElement *elm2 ) {
      return( elm1->pos( ).rx( ) < elm2->pos( ).rx( ) );
    } );
    std::stable_sort( outputs.begin( ), outputs.end( ), [ ]( GraphicElement *elm1, GraphicElement *elm2 ) {
      return( elm1->pos( ).rx( ) < elm2->pos( ).rx( ) );
    } );
    if( sorting == SortingKind::INCREASING ) {
      std::stable_sort( inputs.begin( ), inputs.end( ), [ ]( GraphicElement *elm1, GraphicElement *elm2 ) {
        return( strcasecmp( elm1->getLabel( ).toUtf8( ), elm2->getLabel( ).toUtf8( ) ) <= 0 );
      } );
      std::stable_sort( outputs.begin( ), outputs.end( ), [ ]( GraphicElement *elm1, GraphicElement *elm2 ) {
        return( strcasecmp( elm1->getLabel( ).toUtf8( ), elm2->getLabel( ).toUtf8( ) ) <= 0 );
      } );
    }
    else if( sorting == SortingKind::DECREASING ) {
      std::stable_sort( inputs.begin( ), inputs.end( ), [ ]( GraphicElement *elm1, GraphicElement *elm2 ) {
        return( strcasecmp( elm1->getLabel( ).toUtf8( ), elm2->getLabel( ).toUtf8( ) ) >= 0 );
      } );
      std::stable_sort( outputs.begin( ), outputs.end( ), [ ]( GraphicElement *elm1, GraphicElement *elm2 ) {
        return( strcasecmp( elm1->getLabel( ).toUtf8( ), elm2->getLabel( ).toUtf8( ) ) >= 0 );
      } );
    }
    return( elements );
  }

  int calculatePriority( GraphicElement *elm,
                         QMap< GraphicElement*, bool > &beingvisited,
                         QMap< GraphicElement*, int > &priority,
                         bool &notSequential ) {
    if( !elm ) {
      return( 0 );
    }
    if( elm->elementGroup( ) == ElementGroup::MEMORY ) {
      notSequential = false;
    }
    if( beingvisited.contains( elm ) ) {
      notSequential = false;
      return( 0 );
    }
    if( priority.contains( elm ) ) {
      return( priority[ elm ] );
    }
    beingvisited[ elm ] = true;
    int max = 0;
    for( QNEPort *port : elm->outputs( ) ) {
      for( QNEConnection *conn : port->connections( ) ) {
        QNEPort *sucessor = conn->otherPort( port );
        if( sucessor ) {
          max = qMax( calculatePriority( sucessor->graphicElement( ), beingvisited, priority, notSequential ), max );
        }
      }
    }
    int p = max + 1;
    priority[ elm ] = p;
    beingvisited.remove( elm );
    return( p );
  }

  QVector< GraphicElement* > sortElements( QVector< GraphicElement* > elms, bool &notSequential ) {
    notSequential = true;
    QMap< GraphicElement*, bool > beingvisited;
    QMap< GraphicElement*, int > priority;
    for( GraphicElement *elm : elms ) {
      calculatePriority( elm, beingvisited, priority, notSequential );
    }
    std::sort( elms.begin( ), elms.end( ), [ priority ]( GraphicElement *e1, GraphicElement *e2 ) {
      return( priority[ e2 ] < priority[ e1 ] );
    } );

    return( elms );
  }

  QVector< GraphicElement* > sortElements( QVector< GraphicElement* > elms ) {
    bool notSeq;
    return( sortElements( elms, notSeq ) );
  }

  QVector< GraphicElement* > elementList( QVector< QGraphicsItem* > items ) {
    QVector< GraphicElement* > elements;
    for( QGraphicsItem *item : items ) {
      if( item->type( ) == GraphicElement::Type ) {
        GraphicElement *elm = qgraphicsitem_cast< GraphicElement* >( item );
        if( elm ) {
          elements.append( elm );
        }
      }
    }
    return( elements );
  }

  QVector< GraphicElement* > elementList( QList< QGraphicsItem* > items ) {
    QVector< GraphicElement* > elements;
    for( QGraphicsItem *item : items ) {
      if( item->type( ) == GraphicElement::Type ) {
        GraphicElement *elm = qgraphicsitem_cast< GraphicElement* >( item );
        if( elm ) {
          elements.append( elm );
        }
      }
    }
    return( elements );
  }

  QVector< QNEConnection* > getConnections( const QVector< GraphicElement* > &elements ) {
    QVector< QNEConnection* > connections;
    for( GraphicElement *elm : elements ) {
      for( QNEInputPort *port : elm->inputs( ) ) {
        for( QNEConnection *conn : port->connections( ) ) {
          if( !connections.contains( conn ) ) {
            connections.append( conn );
          }
        }
      }
      for( QNEOutputPort *port : elm->outputs( ) ) {
        for( QNEConnection *conn : port->connections( ) ) {
          if( !connections.contains( conn ) ) {
            connections.append( conn );
          }
        }
      }
    }
    return( connections );
  }
}
