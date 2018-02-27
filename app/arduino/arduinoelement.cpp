#include "arduinoelement.h"

int ArduinoElement::counter = 0;

ArduinoElement::ArduinoElement( GraphicElement *elm ) : elm( elm ) {

}

QString ArduinoElement::getOutput( int index ) {
  return( outputs.at( index ) );
}

void ArduinoElement::mapOutputs( QMap< QNEPort*, QString > &varMap ) {
  for( int i = 0; i < outputSize( ); ++i ) {
    QNEPort *port = elm->output( i );
    if( elm->elementType( ) == ElementType::VCC ) {
      varMap[ port ] = "HIGH";
      continue;
    }
    else if( elm->elementType( ) == ElementType::GND ) {
      varMap[ port ] = "LOW";
      continue;
    }
    else if( varMap[ port ].isEmpty( ) ) {
      varMap[ port ] = getOutput( i );
    }
  }
}

ArduinoElement* ArduinoElement::generate( GraphicElement *elm ) {
  switch( elm->elementType( ) ) {
      case ElementType::VCC:
      return( new ArduinoLogic( elm, "VCC" ) );
      case ElementType::GND:
      return( new ArduinoLogic( elm, "GND" ) );
      case ElementType::AND:
      return( new ArduinoLogic( elm ) );
      break;
      default:
      throw std::runtime_error( "ElementType not supported!" );
  }
}

ArduinoElement::~ArduinoElement( ) {

}

QString ArduinoElement::clearString( QString input ) {
  return( input.toLower( ).trimmed( ).replace( " ", "_" ).replace( "-", "_" ).replace( QRegExp( "\\W" ), "" ) );
}

int ArduinoElement::outputSize( ) {
  return( outputs.size( ) );
}

ArduinoLogic::ArduinoLogic( GraphicElement *elm ) : ArduinoElement( elm ) {
  QString varName = QString( "aux_%1_%2" ).arg( clearString( elm->objectName( ) ) ).arg( counter++ );
  if( elm->outputs( ).size( ) == 1 ) {
    outputs.append( varName );
  }
  else {
    int portCounter = 0;
    for( QNEPort *port : elm->outputs( ) ) {
      QString portName = varName;
      portName.append( QString( "_%1" ).arg( portCounter++ ) );
      if( !port->getName( ).isEmpty( ) ) {
        portName.append( QString( "_%1" ).arg( clearString( port->getName( ) ) ) );
      }
      outputs.append( varName );
    }
  }
}

ArduinoLogic::ArduinoLogic( GraphicElement *elm, QString defaultValue ) : ArduinoElement( elm ) {
  outputs.append( defaultValue );
}
