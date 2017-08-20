#include "boxnonsequential.h"
#include "graphicelement.h"
#include "input.h"
#include <QDebug>
#include <c++/7/bitset>
#include <cmath>

BoxImplNonSequential::BoxImplNonSequential( QString fileName,
                                            const QVector< GraphicElement* > &elements ) : BoxPrototype( fileName ) {
  QVector< QNEPort* > inputMap, outputMap;
  updateAttributes( elements, inputMap, outputMap );

  int num_iter = pow( 2, inputMap.size( ) );
  qDebug( ) << "Generating optimized box with " << inputSize( ) << " inputs, and " << outputSize( ) << " outputs.";

  int outputCount = outputMap.size( );

  results = QVector< QVector< char > >( num_iter, QVector< char >( outputCount ) );
  for( int itr = 0; itr < num_iter; ++itr ) {
    std::bitset< std::numeric_limits< unsigned int >::digits > bs( itr );
    for( int in = 0; in < inputMap.size( ); ++in ) {
      char val = bs[ in ];
      inputMap[ in ]->setValue( val );
    }
    for( GraphicElement *elm : elements ) {
      elm->updateLogic( );
    }
    for( int out = 0; out < outputMap.size( ); ++out ) {
      results[ itr ][ out ] = outputMap[ out ]->value( );
    }
  }
  qDebug( ) << "End..";
}

BoxImplNonSequential::~BoxImplNonSequential( ) {

}

QVector< char > BoxImplNonSequential::updateLogic( const QVector< char > &input ) {
  std::bitset< std::numeric_limits< unsigned int >::digits > bs( 0 );
  if( input.size( ) != inputSize( ) ) {
    throw std::runtime_error( ERRORMSG( "Invalid input." ) );
  }
  for( int in = 0; in < input.size( ); ++in ) {
    if( input[ inputSize( ) - in - 1 ] == -1 ) {
      return( QVector< char >( outputSize( ), -1 ) );
    }
    else {
      bs[ in ] = input[ in ];
    }
  }
  int itr = bs.to_ulong( );
//  if( itr != 0 ) {
//    qDebug( ) << itr;
//    for( int out = 0; out < outputSize( ); ++out ) {
//      qDebug( ) << ( int ) results[ itr ][ out ];
//    }
//  }
  return( results[ itr ] );
}

BoxPrototype* BoxImplNonSequential::clone( ) {
  return( new BoxImplNonSequential( *this ) );
}
