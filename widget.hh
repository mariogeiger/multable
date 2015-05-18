#ifndef WIDGET_HH
#define WIDGET_HH

#include <QWidget>
#include <QTableWidget>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>
#include "multable.hh"

class Widget : public QWidget
{
  Q_OBJECT

public:
  Widget(QWidget *parent = 0);
  ~Widget();

private slots:
  void setOrder(int order);
  void valueChanged();
  void autoComplete();

private:
  void checkTable(MulTable& t);
  MulTable readTable();
  void writeTable(const MulTable& t);

protected:
  QTableWidget* table;
  QSpinBox* spin;
  QLabel* label;
  QPushButton* button;
};

#endif // WIDGET_HH
