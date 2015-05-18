#include "widget.hh"
#include <QVBoxLayout>

Widget::Widget(QWidget *parent)
  : QWidget(parent)
{
  table = new QTableWidget(this);
  spin = new QSpinBox(this); spin->setRange(1, 99); spin->setValue(3);
  label = new QLabel(this);
  button = new QPushButton("Auto-complete", this);

  QObject::connect(spin, SIGNAL(valueChanged(int)), this, SLOT(setOrder(int)));
  QObject::connect(button, SIGNAL(clicked()), this, SLOT(autoComplete()));

  QVBoxLayout* l = new QVBoxLayout(this);

  l->addWidget(spin);
  l->addWidget(table);
  l->addWidget(button);
  l->addWidget(label);

  setOrder(3);
}

Widget::~Widget()
{

}

void Widget::setOrder(int order)
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

void Widget::valueChanged()
{
  label->clear();
  QObject::disconnect(table, SIGNAL(cellChanged(int,int)), this, SLOT(valueChanged()));

  MulTable t = readTable();
  checkTable(t);

  QObject::connect(table, SIGNAL(cellChanged(int,int)), this, SLOT(valueChanged()));
}

void Widget::autoComplete()
{
  QObject::disconnect(table, SIGNAL(cellChanged(int,int)), this, SLOT(valueChanged()));
  label->clear();

  MulTable t = readTable();
  int result = t.isGroup();

  if (result == -1) {
    label->setText("Cannot auto-complete invalid table");
  } else if (result == 0) {

    QList<MulTable> solutions = t.brute();

    if (solutions.size() == 1)
    {
      writeTable(solutions.first());
      label->setText("Complete solution found !");
    }

    if (solutions.isEmpty()) {
      label->setText("There where an tricky error in the table");
    }

    if (solutions.size() > 1) {
      label->setText(QString("%1 solutions found").arg(solutions.size()));

      writeTable(t);
    }
  }

  QObject::connect(table, SIGNAL(cellChanged(int,int)), this, SLOT(valueChanged()));
}

void Widget::checkTable(MulTable& t)
{
  int result = t.isGroup();

  label->clear();
  if (result == 1) {
    label->setText("This is a group !");
  } else if (result == -1) {
    label->setText("There is an error somewhere !");
  }
}

MulTable Widget::readTable()
{
  MulTable t(spin->value());
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

void Widget::writeTable(const MulTable& t)
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
