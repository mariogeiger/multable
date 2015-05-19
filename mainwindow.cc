#include "mainwindow.hh"
#include <QHBoxLayout>
#include <QStatusBar>
#include <QMenuBar>
#include <QMenu>
#include <QWidgetAction>
#include <QSpinBox>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
{
  table = new QTableWidget(this);

  QMenu* menu = menuBar()->addMenu("Menu");
  menu->addAction("Auto-complete", this, SLOT(autoComplete()), QKeySequence(tr("Ctrl+S")));


  QWidget* widget = new QWidget(menu);
  QHBoxLayout* layout = new QHBoxLayout(widget);

  QSpinBox* spin = new QSpinBox(this); spin->setRange(1, 99); spin->setValue(3);
  QObject::connect(spin, SIGNAL(valueChanged(int)), this, SLOT(setOrder(int)));

  layout->addWidget(new QLabel("Order :", widget));
  layout->addWidget(spin);

  QWidgetAction *action = new QWidgetAction(menu);
  action->setDefaultWidget(widget);
  menu->addAction(action);

  setCentralWidget(table);

  setOrder(3);
}

MainWindow::~MainWindow()
{

}

void MainWindow::setOrder(int order)
{
  if (order < 1) return;

  QObject::disconnect(table, SIGNAL(cellChanged(int,int)), this, SLOT(valueChanged()));

  table->setColumnCount(order);
  table->setRowCount(order);

  int size = table->rowHeight(0);
  for (int i = 0; i < table->rowCount(); ++i) {
    table->setRowHeight(i, size);
    table->setColumnWidth(i, size);

    for (int j = 0; j < table->columnCount(); ++j) {
      if (table->item(i, j) == 0) {
        QTableWidgetItem* item = new QTableWidgetItem();
        table->setItem(i, j, item);

        item->setTextAlignment(Qt::AlignCenter);
      }
    }
  }

  QObject::connect(table, SIGNAL(cellChanged(int,int)), this, SLOT(valueChanged()));
}

void MainWindow::valueChanged()
{
  statusBar()->clearMessage();
  QObject::disconnect(table, SIGNAL(cellChanged(int,int)), this, SLOT(valueChanged()));

  checkTable(readTable());

  QObject::connect(table, SIGNAL(cellChanged(int,int)), this, SLOT(valueChanged()));
}

void MainWindow::autoComplete()
{
  QObject::disconnect(table, SIGNAL(cellChanged(int,int)), this, SLOT(valueChanged()));
  statusBar()->clearMessage();

  MulTable t = readTable();
  int result = t.isGroup();

  if (result == -1) {
    statusBar()->showMessage("Auto-complete : error in the table");
  } else if (result == 0) {

    QList<MulTable> solutions = t.brute();

    if (solutions.size() == 1)
    {
      writeTable(solutions.first());
      statusBar()->showMessage("Auto-complete : solution found !");
    }

    if (solutions.isEmpty()) {
      statusBar()->showMessage("Auto-complete : error detected in the table");
    }

    if (solutions.size() > 1) {
      if (solutions.size() > 20)
        statusBar()->showMessage("Auto-complete : more than 20 solutions found");
      else
        statusBar()->showMessage(QString("Auto-complete : %1 solutions found").arg(solutions.size()));

      writeTable(t);
    }
  }

  QObject::connect(table, SIGNAL(cellChanged(int,int)), this, SLOT(valueChanged()));
}

void MainWindow::checkTable(MulTable t)
{
  t.reduce();
  int result = t.isGroup();

  statusBar()->clearMessage();
  if (result == -1) {
    statusBar()->showMessage("There is an error in the table");
  }
}

MulTable MainWindow::readTable()
{
  MulTable t(table->columnCount());
  for (int i = 0; i < table->rowCount(); ++i) {
    for (int j = 0; j < table->columnCount(); ++j) {
      QTableWidgetItem* item = table->item(i, j);
      if (!item) continue;

      QString text = item->text();
      bool ok;
      int value = text.toInt(&ok);

      if (ok && value >= 1 && value <= t.order()) {
        t.setProduct(i, j, value - 1);
      } else {
        item->setText("");
      }
      item->setBackground(QBrush());
    }
  }
  return t;
}

void MainWindow::writeTable(const MulTable& t)
{
  for (int i = 0; i < table->rowCount(); ++i) {
    for (int j = 0; j < table->columnCount(); ++j) {
      int a = t.product(i,j);
      if (a != -1) {
        QTableWidgetItem* item = table->item(i, j);
        if (!item) {
          table->setItem(i, j, new QTableWidgetItem());
        }

        QString text = item->text();
        bool ok;
        int value = text.toInt(&ok);
        if (ok && value >= 1 && value <= t.order()) {
          ;
        } else {
          item->setText(QString::number(a+1));
          item->setBackground(QBrush(Qt::green));
        }
      }
    }
  }
}
