#ifndef BOXSEQUENTIAL_H
#define BOXSEQUENTIAL_H

#include "boxprototype.h"

#include <boxmanager.h>
#include <qneport.h>

class BoxImplSequential : public BoxPrototype {
public:
  explicit BoxImplSequential( QString fileName, const QVector< GraphicElement* > &m_elements );
  explicit BoxImplSequential( const BoxImplSequential &other );
  virtual ~BoxImplSequential( );
private:
  QVector< GraphicElement* > m_elements;
  QVector< QNEPort* > m_inputMap, m_outputMap;


  // BoxImpl interface
private:
  QVector< char > updateLogic( const QVector< char > &inputs );
  BoxPrototype* clone( );
};

#endif // BOXSEQUENTIAL_H
