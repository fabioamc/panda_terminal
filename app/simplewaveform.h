#ifndef SIMPLEWAVEFORM_H
#define SIMPLEWAVEFORM_H

#include "editor.h"
#include "util.h"

#include <QChart>
#include <QChartView>
#include <QDialog>
#include <QTextStream>

using namespace QtCharts;

namespace Ui {
  class SimpleWaveform;
}

class SimpleWaveform : public QDialog {
  Q_OBJECT

public:
  explicit SimpleWaveform( Editor *editor, QWidget *parent = 0 );
  ~SimpleWaveform( );

  void showWaveform( );

  static bool saveToTxt( QTextStream &outStream, Editor *editor );
private slots:
  void on_radioButton_Position_clicked( );

  void on_radioButton_Increasing_clicked( );

  void on_radioButton_Decreasing_clicked( );

  void on_pushButton_Copy_clicked( );

private:
  Ui::SimpleWaveform *ui;
  QChart chart;
  QChartView *chartView;
  Editor *editor;

  Util::SortingKind sortingKind;
};

#endif /* SIMPLEWAVEFORM_H */
