#ifndef BOXPROTOTYPE_H
#define BOXPROTOTYPE_H

#include <QVector>
class GraphicElement;
class QNEPort;

class BoxPrototype {
  QString m_fileName;

  QVector< QString > m_inputLabels, m_outputLabels;
  QVector< bool > m_inputRequired;
  QVector< char > m_inputValue;

public:
  explicit BoxPrototype( QString fname );
  virtual ~BoxPrototype( );
  virtual QVector< char > updateLogic( const QVector< char > &inputs ) = 0;
  virtual BoxPrototype* clone( ) = 0;
  QString fileName( ) const;
  int inputSize( ) const;
  int outputSize( ) const;
  QVector< QString > inputLabels( ) const;
  QVector< QString > outputLabels( ) const;

  void updateAttributes( const QVector< GraphicElement* > &elements,
                         QVector< QNEPort* > &inputPorts,
                         QVector< QNEPort* > &outputPorts );
  const QVector< int > &inputMap( ) const;
  const QVector< int > &outputMap( ) const;
  const QVector< bool > &inputRequired( ) const;
  const QVector< char > &inputValue( ) const;
};



#endif // BOXPROTOTYPE_H
