#ifndef BOXMANAGER_H
#define BOXMANAGER_H

#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QMap>
#include <QObject>
#include <QVector>
#include <QWidget>

class BoxManager;
class BoxPrototype;
class Editor;
class Box;
class MainWindow;

class BoxWatcher {
public:
  explicit BoxWatcher( QString fname, BoxPrototype *proto );
  virtual ~BoxWatcher( );

  QString fileName;

  QList< Box* > assignedBoxes;

  BoxPrototype *prototype;

};



class BoxManager : public QObject {
  Q_OBJECT

  QMap< QString, BoxWatcher* > m_boxes;
  QFileSystemWatcher m_watcher;
public:
  BoxManager( Editor *editor, QWidget *m_mainWindow );

  bool replaceFile( QString fname, QString new_fname );

  Editor *m_editor;

  MainWindow *m_mainWindow;

  bool loadFile(Box *box , QString fileName);

  static QFileInfo findFile( QString fname, QString parentFile );

  void assignWatcher( QFileSystemWatcher &m_watcher );

  void verifyRecursion( Box *box );

public slots:
  void fileChanged( QString file );

public:
  static BoxManager *globalMngr;
};

#endif // BOXMANAGER_H
