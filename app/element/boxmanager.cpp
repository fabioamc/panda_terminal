#include "boxmanager.h"
#include "boxnonsequential.h"
#include "boxsequential.h"
#include "common.h"
#include "editor.h"
#include "globalproperties.h"
#include "serializationfunctions.h"
#include "util.h"
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QLocale>
#include <QMessageBox>
#include <QSettings>
#include <QString>
#include <box.h>
#include <boxnotfoundexception.h>
#include <mainwindow.h>
BoxManager*BoxManager::globalMngr = nullptr;

BoxManager::BoxManager( Editor *editor, QWidget *mainWindow ) :
  m_editor( editor ), m_mainWindow( dynamic_cast< MainWindow* >( mainWindow ) ) {

}

bool BoxManager::replaceFile( QString fname, QString new_fname ) {
  // FIXME
}

QFileInfo BoxManager::findFile( QString fname, QString parentFile ) {
  QFileInfo fileInfo( fname );
  qDebug( ) << "BoxManager::findFile" << fname;
  QString myFile = fileInfo.fileName( );
  COMMENT( "Trying to load (1): " << fileInfo.absoluteFilePath( ).toStdString( ), 1 );
  if( !fileInfo.exists( ) ) {
    fileInfo.setFile( QDir::current( ), fileInfo.fileName( ) );
    qDebug( ) << "Trying to load (2): " << fileInfo.absoluteFilePath( );
    if( !fileInfo.exists( ) ) {
      fileInfo.setFile( QFileInfo( parentFile ).absoluteDir( ), myFile );
      qDebug( ) << "Parent file: " << parentFile;
      qDebug( ) << "Trying to load (3): " << fileInfo.absoluteFilePath( );
      if( !fileInfo.exists( ) ) {
        QFileInfo currentFile( GlobalProperties::currentFile );
        qDebug( ) << "Current file: " << currentFile.absoluteFilePath( );
        qDebug( ) << "Trying to load (4): " << fileInfo.absoluteFilePath( );
        fileInfo.setFile( currentFile.absoluteDir( ), myFile );
        if( !fileInfo.exists( ) ) {

          std::cerr << "Error: This file could not be found: " << fname.toStdString( ) << std::endl;
          throw( BoxNotFoundException( ERRORMSG( QString(
                                                   "Box linked file \"%1\" could not be found!\n"
                                                   "Do you want to find this file?" )
                                                 .arg( fname ).toStdString( ) ) ) );
        }
      }
    }
  }
  return( fileInfo );
}

void BoxManager::assignWatcher( QFileSystemWatcher &watcher ) {
  connect( &watcher, &QFileSystemWatcher::fileChanged, this, &BoxManager::fileChanged );
}



BoxWatcher::~BoxWatcher( ) {
  delete prototype;
}

void BoxManager::fileChanged( QString file ) {
  QMessageBox msgBox;
  /*    msgBox.setParent(  ); */
  msgBox.setLocale( QLocale::Portuguese );
  msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
  msgBox.setText( tr( "The file %1 changed, do you want to reload?" ).arg( file ) );
  msgBox.setWindowModality( Qt::ApplicationModal );
  msgBox.setDefaultButton( QMessageBox::Yes );
  if( msgBox.exec( ) == QMessageBox::Yes ) {
    for( Box *box: m_boxes[ file ]->assignedBoxes ) {
      loadFile( box, file );
    }
  }
}

void BoxManager::verifyRecursion( Box *box ) {
  std::string msg = "Oh no! I'm my own parent.\nSomething is not ok...";
  if( !box->getParentFile( ).isEmpty( ) && ( box->getFile( ) == box->getParentFile( ) ) ) {
    throw( std::runtime_error( ERRORMSG( msg ) ) );
  }
  for( Box *box_p = box->getParentBox( ); box_p != nullptr; box_p = box_p->getParentBox( ) ) {
    if( box_p->getFile( ) == box->getFile( ) ) {
      throw( std::runtime_error( ERRORMSG( msg ) ) );
    }
  }
}

bool BoxManager::loadFile( Box *box, QString fileName ) {
  qDebug( ) << "Loading file: " << fileName;
  if( fileName.isEmpty( ) ) {
    return( false );
  }
  if( box->getParentFile( ).isEmpty( ) ) {
    box->setParentFile( GlobalProperties::currentFile );
  }
  QFileInfo fileInfo = findFile( fileName, box->getParentFile( ) );
  fileName = fileInfo.absoluteFilePath( );
  qDebug( ) << "Loading file: " << fileName;
  if( !m_boxes.contains( fileName ) ) {
    try {
      verifyRecursion( box );

      QFile file( fileName );
      if( file.open( QFile::ReadOnly ) ) {
        QDataStream ds( &file );
        QList< QGraphicsItem* > items = SerializationFunctions::load( m_editor, ds, fileName );

        QVector< GraphicElement* > elements = Util::elementList( items );
        bool notSequential = true;
        elements = Util::sortElements( elements, notSequential );
        BoxPrototype *impl = nullptr;
        if( notSequential ) {
          impl = new BoxImplNonSequential( fileName, elements );
          qDeleteAll( elements );
        }
        else {
          impl = new BoxImplSequential( fileName, elements );
        }
        BoxWatcher *watcher = new BoxWatcher( fileName, impl );
        m_boxes.insert( fileName, watcher );
        m_watcher.addPath( fileName );
        m_editor->updateBoxes( fileName );
      }
      else {
        throw std::runtime_error( ERRORMSG( tr( "Could not open file \"%1\"." ).arg( fileName ).toStdString( ) ) );
      }
    }
    catch( BoxNotFoundException &err ) {
      qDebug( ) << "BoxNotFoundException thrown: " << err.what( );
      if( m_mainWindow ) {
        int ret = QMessageBox::warning( m_mainWindow, tr( "Error" ), QString::fromStdString(
                                          err.what( ) ), QMessageBox::Ok, QMessageBox::Cancel );
        if( ret == QMessageBox::Cancel ) {
          return( false );
        }
        else {
          QString new_fname = m_mainWindow->getOpenBoxFile( );
          if( new_fname.isEmpty( ) ) {
            return( false );
          }
          else {
            return( BoxManager::globalMngr->replaceFile( fileName, new_fname ) );
          }
        }
      }
      else {
        throw err;
      }
    }
  }
  if( m_boxes.contains( fileName ) ) {
    BoxPrototype *boxImpl = m_boxes[ fileName ]->prototype->clone( );
    box->setBoxImpl( boxImpl );
    box->m_file = fileName;
    if( box->getLabel( ).isEmpty( ) ) {
      QFileInfo fileInfo( fileName );
      box->setLabel( fileInfo.baseName( ).toUpper( ) );
    }
    box->setMinInputSz( boxImpl->inputSize( ) );
    box->setMaxInputSz( boxImpl->inputSize( ) );
    box->setInputSize( boxImpl->inputSize( ) );

    box->setMinOutputSz( boxImpl->outputSize( ) );
    box->setMaxOutputSz( boxImpl->outputSize( ) );
    box->setOutputSize( boxImpl->outputSize( ) );
    for( int in = 0; in < box->inputSize( ); ++in ) {
      box->input( in )->setName( boxImpl->inputLabels( )[ in ] );
      box->input( in )->setDefaultValue( boxImpl->inputValue( )[ in ] );
      box->input( in )->setValue( boxImpl->inputValue( )[ in ] );
      box->input( in )->setRequired( boxImpl->inputRequired( )[ in ] );
    }
    for( int out = 0; out < box->outputSize( ); ++out ) {
      box->output( out )->setName( boxImpl->outputLabels( )[ out ] );
    }
    return( true );
  }
  return( false );
}

BoxWatcher::BoxWatcher( QString fname, BoxPrototype *proto ) :
  fileName( fname ), prototype( proto ) {
}
