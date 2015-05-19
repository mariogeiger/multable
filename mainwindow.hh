#ifndef WIDGET_HH
#define WIDGET_HH

#include <QMainWindow>
#include <QTableWidget>
#include "multable.hh"

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget *parent = 0);
  ~MainWindow();

private slots:
  void setOrder(int order);
  void valueChanged();
  void autoComplete();

private:
  void checkTable(MulTable t);
  MulTable readTable();
  void writeTable(const MulTable& t);

protected:
  QTableWidget* table;
};

#endif // WIDGET_HH
