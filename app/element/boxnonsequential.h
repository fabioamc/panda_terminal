#ifndef BOXNONSEQUENTIAL_H
#define BOXNONSEQUENTIAL_H

#include "boxprototype.h"

#include <boxmanager.h>

class BoxImplNonSequential : public BoxPrototype {
public:
  explicit BoxImplNonSequential( QString fileName, const QVector< GraphicElement* > &elements );
  ~BoxImplNonSequential( );
  QVector< QVector< char > > results;
  // BoxImpl interface
public:
  QVector< char > updateLogic( const QVector< char > &inputs );
  BoxPrototype* clone( );
};

#endif // BOXNONSEQUENTIAL_H
