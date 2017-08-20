#include "box.h"
#include "boxmanager.h"
#include "boxprototype.h"
#include "editor.h"
#include "globalproperties.h"
#include "serializationfunctions.h"
#include "util.h"
#include <QApplication>
#include <QDebug>
#include <QFileInfo>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QMessageBox>
#include <QPointF>
#include <QProcess>
#include <c++/7/bitset>
#include <cmath>
#include <inputswitch.h>
#include <iostream>
#include <nodes/qneconnection.h>

Box::Box( QGraphicsItem *parent ) : GraphicElement( 0, 0, 0, 0, parent ) {
  setHasLabel( true );
  /*
   *  QTransform transform;
   *  transform.translate(pixmap.size().width() / 2, pixmap.size().height() / 2);
   *  transform.rotate(-90);
   *  transform.translate(-pixmap.size().width() / 2, -pixmap.size().height() / 2);
   */

  /*
   *  setRotatable(false);
   *  setRotation(90);
   */

  /*  setPixmap(pixmap.transformed(transform)); */
  setPixmap( ":/basic/box.png" );
  setOutputsOnTop( true );
  setPortName( "BOX" );
  m_parentBox = nullptr;
}

Box::~Box( ) {
}


void Box::save( QDataStream &ds ) {
  GraphicElement::save( ds );
  ds << m_file;
}

void Box::load( QDataStream &ds, QMap< quint64, QNEPort* > &portMap, double version ) {
  GraphicElement::load( ds, portMap, version );
  if( version >= 1.2 ) {
    ds >> m_file;
  }
}

void Box::updateLogic( ) {
  QVector< char > inputvals( inputSize( ) );
  for( int in = 0; in < inputSize( ); ++in ) {
    inputvals[ in ] = input( in )->value( );
  }
  QVector< char > outputvals = m_boxImpl->updateLogic( inputvals );
  for( int out = 0; out < outputSize( ); ++out ) {
    output( out )->setValue( outputvals[ out ] );
  }
}

QString Box::getParentFile( ) const {
  return( m_parentFile );
}

void Box::setParentFile( const QString &value ) {
  m_parentFile = value;
}



QString Box::getFile( ) const {
  return( m_file );
}

//QVector< GraphicElement* > Box::getElements( ) const {
//  return( elements );
//}


Box* Box::getParentBox( ) const {
  return( m_parentBox );
}

void Box::setParentBox( Box *value ) {
  setParentFile( value->getFile( ) );
  m_parentBox = value;
}

BoxPrototype* Box::getBoxImpl( ) const {
  return( m_boxImpl );
}

void Box::setBoxImpl( BoxPrototype *boxImpl ) {
  m_boxImpl = boxImpl;
}

void Box::mouseDoubleClickEvent( QGraphicsSceneMouseEvent *event ) {
  if( event->button( ) == Qt::LeftButton ) {
    QMessageBox msgBox;
    /*    msgBox.setParent(  ); */
    msgBox.setLocale( QLocale::Portuguese );
    msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
    msgBox.setText( tr( "Do you want to load this file?<br>%1" ).arg( m_file ) );
    msgBox.setWindowModality( Qt::ApplicationModal );
    msgBox.setDefaultButton( QMessageBox::Yes );
    if( msgBox.exec( ) == QMessageBox::Yes ) {
      QProcess *wPanda = new QProcess( scene( ) );
      QStringList args;
      args << m_file;
      wPanda->start( QCoreApplication::applicationFilePath( ), args );
    }
  }
}
