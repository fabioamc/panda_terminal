#ifndef TESTARDUINO_H
#define TESTARDUINO_H

#include <QObject>
#include <QTest>


class TestArduino : public QObject {
  Q_OBJECT
public:
  explicit TestArduino( QObject *parent = nullptr );

private slots:

  void init( );

  void cleanup( );

  void testFactory( );

  void testAnd( );

};

#endif // TESTARDUINO_H
