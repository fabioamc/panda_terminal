#ifndef ARDUINOELEMENT_H
#define ARDUINOELEMENT_H

#include <QString>
#include <graphicelement.h>

class ArduinoElement {
public:
  static int counter;

  static ArduinoElement* generate( GraphicElement *elm );

  virtual ~ArduinoElement( );

  static QString clearString( QString input );

  int outputSize( );

  QString getOutput( int index = 0 );

  void mapOutputs( QMap< QNEPort*, QString > &varMap );

  virtual int type( ) = 0;

protected:
  GraphicElement *elm;

  QVector< QString > outputs;

  ArduinoElement( GraphicElement *elm );

};

class ArduinoLogic : public ArduinoElement {
public:
  enum { Type = 0 };

  int type( ) {
    return( Type );
  }


  ArduinoLogic( GraphicElement *elm );
  ArduinoLogic( GraphicElement *elm, QString defaultValue );
};

class ArduinoInput : public ArduinoElement {
public:
  enum { Type = 1 };

  int type( ) {
    return( Type );
  }
};

class ArduinoOutput : public ArduinoElement {
public:
  enum { Type = 2 };

  int type( ) {
    return( Type );
  }
};


#endif // ARDUINOELEMENT_H
