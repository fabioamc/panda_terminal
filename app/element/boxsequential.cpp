#include "boxsequential.h"
#include "graphicelement.h"
#include "serializationfunctions.h"
#include "util.h"

#include <QDebug>
BoxImplSequential::BoxImplSequential( QString fileName, const QVector< GraphicElement* > &elements ) :
  BoxPrototype( fileName ), m_elements( elements ) {
  updateAttributes( m_elements, m_inputMap, m_outputMap );
  qDebug( ) << "Generating sequential box with " << inputSize( ) << " inputs, and " << outputSize( ) << " outputs.";
}

BoxImplSequential::BoxImplSequential( const BoxImplSequential &other ) :
  BoxPrototype( other.fileName( ) ) {
  qDebug( ) << "Cloning Sequential Box Impl.";
  m_elements = SerializationFunctions::duplicate( other.m_elements );
  updateAttributes( m_elements, m_inputMap, m_outputMap );
}

BoxImplSequential::~BoxImplSequential( ) {
  qDeleteAll( m_elements );
}

QVector< char > BoxImplSequential::updateLogic( const QVector< char > &inputs ) {
  QVector< char > output( outputSize( ) );
//  std::cout << std::endl;
  for( int in = 0; in < inputSize( ); ++in ) {
    m_inputMap[ in ]->setValue( inputs[ in ] );
//    std::cout << inputLabels( )[ in ].toStdString( ) << " = " << ( int ) inputs[ in ] << ", ";
  }
//  std::cout << std::endl;
  for( GraphicElement *elm : m_elements ) {
    elm->updateLogic( );
  }
  for( int out = 0; out < outputSize( ); ++out ) {
    output[ out ] = m_outputMap[ out ]->value( );
//    std::cout << outputLabels( )[ out ].toStdString( ) << " = " << ( int ) output[ out ] << ", ";
  }
//  std::cout << std::endl;

  return( output );
}

BoxPrototype* BoxImplSequential::clone( ) {
  return( new BoxImplSequential( *this ) );
}
