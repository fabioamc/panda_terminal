#include "boxprototype.h"
#include "graphicelement.h"
#include "qneport.h"

QString BoxPrototype::fileName( ) const {
  return( m_fileName );
}

int BoxPrototype::inputSize( ) const {
  return( m_inputLabels.size( ) );
}

int BoxPrototype::outputSize( ) const {
  return( m_outputLabels.size( ) );
}

QVector< QString > BoxPrototype::inputLabels( ) const {
  return( m_inputLabels );
}

QVector< QString > BoxPrototype::outputLabels( ) const {
  return( m_outputLabels );
}

void sortMap( QVector< QNEPort* > &map ) {
  std::stable_sort( map.begin( ), map.end( ), [ ]( const QNEPort *p1, const QNEPort *p2 ) {
    return( p1->graphicElement( )->pos( ).rx( ) < p2->graphicElement( )->pos( ).rx( ) );
  } );
  std::stable_sort( map.begin( ), map.end( ), [ ]( const QNEPort *p1, const QNEPort *p2 ) {
    return( p1->graphicElement( )->pos( ).ry( ) < p2->graphicElement( )->pos( ).ry( ) );
  } );
}

void BoxPrototype::updateAttributes( const QVector< GraphicElement* > &elements,
                                     QVector< QNEPort* > &inputPorts,
                                     QVector< QNEPort* > &outputPorts ) {
  for( GraphicElement *elm : elements ) {
    switch( elm->elementType( ) ) {
        case ElementType::BUTTON:
        case ElementType::SWITCH:
        case ElementType::CLOCK: {
        for( QNEOutputPort *port : elm->outputs( ) ) {
          inputPorts.append( port );
        }
        elm->disable( );
        break;
      }
        case ElementType::DISPLAY:
        case ElementType::LED: {
        for( QNEInputPort *port : elm->inputs( ) ) {
          outputPorts.append( port );
        }
        break;
      }
        default: {
        break;
      }
    }
  }
  m_inputLabels.resize( inputPorts.size( ) );
  m_inputRequired = QVector< bool >( inputPorts.size( ), true );
  m_inputValue = QVector< char >( inputPorts.size( ), -1 );

  m_outputLabels.resize( outputPorts.size( ) );

  sortMap( inputPorts );
  sortMap( outputPorts );
  for( int inport = 0; inport < inputPorts.size( ); ++inport ) {
    GraphicElement *elm = inputPorts.at( inport )->graphicElement( );
    QString lb = elm->getLabel( );
    if( lb.isEmpty( ) ) {
      if( elm->getTrigger( ).isEmpty( ) ) {
        lb = elm->objectName( );
      }
      else {
        lb = elm->getTrigger( ).toString( );
      }
    }
    if( !inputPorts.at( inport )->portName( ).isEmpty( ) ) {
      lb += " ";
      lb += inputPorts.at( inport )->portName( );
    }
    if( !elm->genericProperties( ).isEmpty( ) ) {
      lb += " [" + elm->genericProperties( ) + "]";
    }
    m_inputLabels[ inport ] = lb;
    if( elm->elementType( ) != ElementType::CLOCK ) {
      m_inputRequired[ inport ] = false;
      m_inputValue[ inport ] = inputPorts.at( inport )->value( );
    }
  }
  for( int outport = 0; outport < outputPorts.size( ); ++outport ) {
    GraphicElement *elm = outputPorts.at( outport )->graphicElement( );
    QString lb = elm->getLabel( );
    if( lb.isEmpty( ) ) {
      lb = elm->objectName( );
    }
    if( !outputPorts.at( outport )->portName( ).isEmpty( ) ) {
      lb += " ";
      lb += outputPorts.at( outport )->portName( );
    }
    if( !elm->genericProperties( ).isEmpty( ) ) {
      lb += " [" + elm->genericProperties( ) + "]";
    }
    m_outputLabels[ outport ] = lb;
  }
}


const QVector< bool > &BoxPrototype::inputRequired( ) const {
  return( m_inputRequired );
}

const QVector< char > &BoxPrototype::inputValue( ) const {
  return( m_inputValue );
}

BoxPrototype::BoxPrototype( QString fname ) : m_fileName( fname ) {

}

BoxPrototype::~BoxPrototype( ) {

}
