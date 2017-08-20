#include "box.h"
#include "boxmanager.h"
#include "editor.h"
#include "globalproperties.h"
#include "graphicelement.h"
#include "qneconnection.h"
#include "serializationfunctions.h"
#include "util.h"
#include <QApplication>
#include <QDebug>
#include <QGraphicsView>
#include <QMessageBox>
#include <iostream>
#include <stdexcept>
void SerializationFunctions::serialize( const QList< QGraphicsItem* > &items, QDataStream &ds ) {
  for( QGraphicsItem *item: items ) {
    if( item->type( ) == GraphicElement::Type ) {
      GraphicElement *elm = qgraphicsitem_cast< GraphicElement* >( item );
      ds << item->type( );
      ds << static_cast< quint64 >( elm->elementType( ) );
      elm->save( ds );
    }
  }
  for( QGraphicsItem *item: items ) {
    if( item->type( ) == QNEConnection::Type ) {
      QNEConnection *conn = qgraphicsitem_cast< QNEConnection* >( item );
      ds << item->type( );
      conn->save( ds );
    }
  }
}

QList< QGraphicsItem* > SerializationFunctions::deserialize( QDataStream &ds,
                                                             double version,
                                                             QString parentFile,
                                                             QMap< quint64, QNEPort* > portMap ) {
  QList< QGraphicsItem* > itemList;
  while( !ds.atEnd( ) ) {
    int type;
    ds >> type;
    if( type == GraphicElement::Type ) {
      quint64 elmType;
      ds >> elmType;
      COMMENT( "Building " << ElementFactory::typeToText( ( ElementType ) elmType ).toStdString( ) << " element.", 4 );
      GraphicElement *elm = ElementFactory::buildElement( ( ElementType ) elmType );
      if( elm ) {
        itemList.append( elm );
        elm->load( ds, portMap, version );
        if( elm->elementType( ) == ElementType::BOX ) {
          Box *box = qgraphicsitem_cast< Box* >( elm );
          qDebug( ) << box->getFile( ) << " = " << box->inputSize( ) << " : " << box->outputSize( );
          box->setParentFile( parentFile );
          BoxManager::globalMngr->loadFile( box, box->getFile( ) );
          qDebug( ) << box->getFile( ) << " = " << box->inputSize( ) << " : " << box->outputSize( );
        }
        elm->setSelected( true );
      }
      else {
        throw( std::runtime_error( ERRORMSG( "Could not build element." ) ) );
      }
    }
    else if( type == QNEConnection::Type ) {
      QNEConnection *conn = ElementFactory::buildConnection( );
      conn->setSelected( true );
      if( !conn->load( ds, portMap ) ) {
        delete conn;
      }
      else {
        itemList.append( conn );
      }
    }
    else {
      qDebug( ) << type;
      throw( std::runtime_error( ERRORMSG( "Invalid type. Data is possibly corrupted." ) ) );
    }
  }
  return( itemList );
}


QList< QGraphicsItem* > SerializationFunctions::load( Editor *editor, QDataStream &ds, QString parentFile,
                                                      Scene *scene ) {
  QString str;
  ds >> str;
  if( !str.startsWith( QApplication::applicationName( ) ) ) {
    throw( std::runtime_error( ERRORMSG( "Invalid file format." ) ) );
  }
  bool ok;
  double version = GlobalProperties::toDouble( str.split( " " ).at( 1 ), &ok );
  if( !ok ) {
    throw( std::runtime_error( ERRORMSG( "Invalid version number." ) ) );
  }
  QRectF rect;
  if( version >= 1.4 ) {
    ds >> rect;
  }
  QList< QGraphicsItem* > items = deserialize( ds, version, parentFile );
  if( scene ) {
    for( QGraphicsItem *item : items ) {
      scene->addItem( item );
    }
    scene->setSceneRect( scene->itemsBoundingRect( ) );
    if( !scene->views( ).empty( ) ) {
      QGraphicsView *view = scene->views( ).first( );
      rect = rect.united( view->rect( ) );
      rect.moveCenter( QPointF( 0, 0 ) );
      scene->setSceneRect( scene->sceneRect( ).united( rect ) );
      view->centerOn( scene->itemsBoundingRect( ).center( ) );
    }
  }
  return( items );
}

QVector< GraphicElement* > SerializationFunctions::duplicate( const QVector< GraphicElement* > &elements ) {
  QByteArray itemData;
  QDataStream ds( &itemData, QIODevice::ReadWrite );
  QMap< quint64, QNEPort* > portMap;
  double version = GlobalProperties::version;
  QVector< QNEConnection* > connections = Util::getConnections( elements );
  QVector< GraphicElement* > new_elms;
  for( GraphicElement *elm: elements ) {
    elm->save( ds );
    GraphicElement *new_elm = ElementFactory::buildElement( elm->elementType( ) );
    qDebug( ) << elm->objectName( ) << elm->inputSize( ) << elm->outputSize( );
    new_elm->load( ds, portMap, version );
    qDebug( ) << new_elm->objectName( ) << new_elm->inputSize( ) << new_elm->outputSize( );
    if( elm->elementType( ) == ElementType::BOX ) {
      Box *box = qgraphicsitem_cast< Box* >( elm );
      Box *new_box = qgraphicsitem_cast< Box* >( new_elm );
      new_box->setParentFile( box->getParentFile( ) );
      BoxManager::globalMngr->loadFile( new_box, new_box->getFile( ) );
    }
    new_elms.append( new_elm );
  }
  for( QNEConnection *conn: connections ) {
    QNEConnection *new_conn = ElementFactory::buildConnection( );
    conn->save( ds );
    new_conn->load( ds, portMap );
  }
  return( new_elms );
}
