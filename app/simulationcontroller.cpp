#include "simulationcontroller.h"

#include <QDebug>
#include <limits>

#include <element/clock.h>

#include "util.h"
#include <QStack>
#include <nodes/qneconnection.h>

SimulationController::SimulationController( Scene *scn ) : QObject( dynamic_cast< QObject* >( scn ) ), timer( this ) {
  scene = scn;
  timer.setInterval( GLOBALCLK );
  connect( &timer, &QTimer::timeout, this, &SimulationController::update );
}

SimulationController::~SimulationController( ) {

}


void SimulationController::update( ) {
  QVector< GraphicElement* > elements = scene->getElements( );
  if( Clock::reset ) {
    for( GraphicElement *elm : elements ) {
      if( elm->elementType( ) == ElementType::CLOCK ) {
        Clock *clk = dynamic_cast< Clock* >( elm );
        if( clk ) {
          clk->resetClock( );
        }
      }
    }
    Clock::reset = false;
  }
  if( elements.isEmpty( ) ) {
    return;
  }
  for( GraphicElement *elm : sortedElements ) {
    elm->updateLogic( );
  }
}

void SimulationController::stop( ) {
  timer.stop( );
}

void SimulationController::start( ) {
  timer.start( );
  Clock::reset = true;
  reSortElms( );
}

void SimulationController::reSortElms( ) {
  COMMENT( "Re-sorting elements", 3 );
  QVector< GraphicElement* > elements = scene->getElements( );
  sortedElements = Util::sortElements( elements );
}
