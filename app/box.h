#ifndef BOX_H
#define BOX_H

#include "elementfactory.h"
#include "graphicelement.h"
#include "scene.h"
#include "simulationcontroller.h"

class Editor;
class BoxPrototype;
class BoxManager;

class Box : public GraphicElement {
  Q_OBJECT

  friend class CodeGenerator;
  friend class BoxManager;
public:
  Box( QGraphicsItem *parent = 0 );
  virtual ~Box( );
  /* GraphicElement interface */
  virtual ElementType elementType( ) {
    return( ElementType::BOX );
  }
  virtual ElementGroup elementGroup( ) {
    return( ElementGroup::BOX );
  }
  void save( QDataStream &ds );
  void load( QDataStream &ds, QMap< quint64, QNEPort* > &portMap, double version );
  void updateLogic( );

  QString getFile( ) const;

  QString getParentFile( ) const;
  void setParentFile( const QString &value );

  Box* getParentBox( ) const;
  void setParentBox( Box *value );

  void verifyRecursion( QString fname );

  BoxPrototype* getBoxImpl( ) const;
  void setBoxImpl( BoxPrototype *boxImpl );

private:
  QString m_file;

  QString m_parentFile;
  Box *m_parentBox;

  BoxPrototype *m_boxImpl;

  /* QGraphicsItem interface */
protected:
  void mouseDoubleClickEvent( QGraphicsSceneMouseEvent *event );
};

#endif /* BOX_H */
