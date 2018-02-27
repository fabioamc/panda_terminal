#include "testarduino.h"

#include <arduino/arduinoelement.h>

#include <and.h>

TestArduino::TestArduino( QObject *parent ) : QObject( parent ) {

}

void TestArduino::init( ) {
  ArduinoElement::counter = 0;
}

void TestArduino::cleanup( ) {

}

void TestArduino::testFactory( ) {
  And elm;
  ArduinoElement *ardElm = ArduinoElement::generate( &elm );

  delete ardElm;
}

void TestArduino::testAnd( ) {
  And elm;
  ArduinoElement *ardElm = ArduinoElement::generate( &elm );
  QCOMPARE( ardElm->getOutput( ), QString( "aux_and_0" ) );
  QCOMPARE( ardElm->type( ), ( int ) ArduinoLogic::Type );

  delete ardElm;
}
