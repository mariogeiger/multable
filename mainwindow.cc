#include "mainwindow.hh"
#include <QHBoxLayout>
#include <QStatusBar>
#include <QMenuBar>
#include <QMenu>
#include <QWidgetAction>
#include <QSpinBox>
#include <QLabel>
#include <QStringList>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	table = new QTableWidget(this);
	QObject::connect(table, SIGNAL(itemSelectionChanged()), this, SLOT(valueSelected()));

	QMenu *menu = menuBar()->addMenu("Menu");

	menu->addAction("Clear table", this, SLOT(clearTable()), QKeySequence("Ctrl+X"));
	menu->addAction("Auto-complete", this, SLOT(autoComplete()), QKeySequence("Ctrl+S"));

	QWidget *widget = new QWidget(menu);
	QHBoxLayout *layout = new QHBoxLayout(widget);

	QSpinBox *spin = new QSpinBox(this);
	spin->setRange(1, 99);
	spin->setValue(3);
	QObject::connect(spin, SIGNAL(valueChanged(int)), this, SLOT(setOrder(int)));

	layout->addWidget(new QLabel("Order :", widget));
	layout->addWidget(spin);

	QWidgetAction *action = new QWidgetAction(menu);
	action->setDefaultWidget(widget);
	menu->addAction(action);

	QAction *grow = menu->addAction("Grow table");
	grow->setShortcut(QKeySequence("Ctrl+K"));
	QObject::connect(grow, &QAction::triggered, [=]
					 { spin->setValue(spin->value() + 1); });

	QAction *decrease = menu->addAction("Decrease table");
	decrease->setShortcut(QKeySequence("Ctrl+J"));
	QObject::connect(decrease, &QAction::triggered, [=]
					 { spin->setValue(spin->value() - 1); });

	QAction *eqclass = menu->addAction("Equivalent class of selected column");
	eqclass->setShortcut(QKeySequence("Ctrl+E"));
	QObject::connect(eqclass, &QAction::triggered, this, &MainWindow::getEqClass);

	menu->addAction("Clear selection", this, SLOT(clearSelection()), QKeySequence(Qt::Key_Delete));

	setCentralWidget(table);

	setOrder(3);
}

MainWindow::~MainWindow()
{
}

void MainWindow::setOrder(int order)
{
	if (order < 1)
		return;

	QObject::disconnect(table, SIGNAL(cellChanged(int, int)), this, SLOT(valueChanged()));

	table->setColumnCount(order);
	table->setRowCount(order);

	QStringList labels = QStringList();
	for (int i = 0; i < order; ++i)
	{
		labels.append(QString::number(i));
	}

	table->setHorizontalHeaderLabels(labels);
	table->setVerticalHeaderLabels(labels);

	int size = table->rowHeight(0);
	for (int i = 0; i < table->rowCount(); ++i)
	{
		table->setRowHeight(i, size);
		table->setColumnWidth(i, size);

		for (int j = 0; j < table->columnCount(); ++j)
		{
			if (table->item(i, j) == 0)
			{
				QTableWidgetItem *item = new QTableWidgetItem();
				table->setItem(i, j, item);

				item->setTextAlignment(Qt::AlignCenter);
			}
		}
	}

	QObject::connect(table, SIGNAL(cellChanged(int, int)), this, SLOT(valueChanged()));
}

void MainWindow::valueChanged()
{
	statusBar()->clearMessage();
	QObject::disconnect(table, SIGNAL(cellChanged(int, int)), this, SLOT(valueChanged()));

	for (int i = 0; i < table->rowCount(); ++i)
	{
		for (int j = 0; j < table->columnCount(); ++j)
		{
			auto *item = table->item(i, j);
			if (item)
			{
				item->setData(Qt::ToolTipRole, "");
			}
		}
	}

	checkTable(readTable());

	QObject::connect(table, SIGNAL(cellChanged(int, int)), this, SLOT(valueChanged()));
}

void MainWindow::valueSelected()
{
	QList<QTableWidgetItem *> selected = table->selectedItems();
	if (selected.size() == 1)
	{
		QString tooltip = selected.first()->data(Qt::ToolTipRole).toString();
		if (!tooltip.isEmpty())
		{
			statusBar()->showMessage(QString("Possible entries : %1").arg(tooltip));
		}
		else
		{
			if (statusBar()->currentMessage().startsWith("Possible entries"))
				statusBar()->clearMessage();
		}
	}
}

void MainWindow::autoComplete()
{
	QObject::disconnect(table, SIGNAL(cellChanged(int, int)), this, SLOT(valueChanged()));
	statusBar()->clearMessage();

	MulTable t = readTable();
	int result = t.isGroup();

	if (result == -1)
	{
		statusBar()->showMessage("Auto-complete : error in the table, this is not a group");
	}
	else if (result == 0)
	{
		statusBar()->showMessage("Searching...");
		statusBar()->update();

		int limit = 2000;
		QList<MulTable> solutions = t.brute(limit);

		writeTable(t);
		if (limit > 0)
		{
			for (int i = 0; i < table->rowCount(); ++i)
			{
				for (int j = 0; j < table->columnCount(); ++j)
				{
					QTableWidgetItem *item = table->item(i, j);

					int a = t.product(i, j);
					if (a == -1)
					{
						QStringList sl;
						for (MulTable &x : solutions)
						{
							sl << QString::number(x.product(i, j));
						}
						QSet<QString> set(sl.begin(), sl.end());
						sl = QStringList(set.begin(), set.end());
						item->setData(Qt::ToolTipRole, sl.join(","));
					}
				}
			}
			statusBar()->showMessage(QString("Auto-complete : the total number of solution is %1").arg(solutions.size()));
		}
		else
		{
			statusBar()->showMessage(QString("Auto-complete : stopped after %1 solution(s) found").arg(solutions.size()));
		}
	}

	QObject::connect(table, SIGNAL(cellChanged(int, int)), this, SLOT(valueChanged()));
}

void MainWindow::clearTable()
{
	for (int i = 0; i < table->rowCount(); ++i)
	{
		for (int j = 0; j < table->columnCount(); ++j)
		{
			auto *item = table->item(i, j);
			if (item)
			{
				item->setText("");
				item->setData(Qt::ToolTipRole, "");
			}
		}
	}
}

void MainWindow::clearSelection()
{
	for (QTableWidgetItem *item : table->selectedItems())
	{
		item->setText("");
		item->setData(Qt::ToolTipRole, "");
	}
}

void MainWindow::getEqClass()
{
	if (table->selectedItems().isEmpty())
		return;
	QTableWidgetItem *item = table->selectedItems().first();
	int a = table->column(item);
	QList<int> eq = readTable().eqclass(a);
	if (eq.isEmpty())
		return;

	QStringList sl;
	for (int x : eq)
		sl << QString::number(x + 1);

	QString text = "The eq. class of %1 is {%2}";
	text = text.arg(a + 1).arg(sl.join(", "));
	statusBar()->showMessage(text);
}

void MainWindow::checkTable(MulTable t)
{
	t.reduce();
	int result = t.isGroup();

	statusBar()->clearMessage();
	if (result == -1)
	{
		statusBar()->showMessage("There is an error in the table");
	}
}

MulTable MainWindow::readTable()
{
	MulTable t(table->columnCount());
	for (int i = 0; i < table->rowCount(); ++i)
	{
		for (int j = 0; j < table->columnCount(); ++j)
		{
			QTableWidgetItem *item = table->item(i, j);
			if (!item)
				continue;

			QString text = item->text();
			bool ok;
			int value = text.toInt(&ok);

			if (ok && value >= 0 && value < t.order())
			{
				t.setProduct(i, j, value);
			}
			else
			{
				item->setText("");
			}
			item->setBackground(QBrush());
		}
	}
	return t;
}

void MainWindow::writeTable(const MulTable &t)
{
	for (int i = 0; i < table->rowCount(); ++i)
	{
		for (int j = 0; j < table->columnCount(); ++j)
		{
			QTableWidgetItem *item = table->item(i, j);
			if (!item)
			{
				table->setItem(i, j, new QTableWidgetItem());
			}

			int a = t.product(i, j);
			if (a != -1)
			{
				QString text = item->text();
				bool ok;
				int value = text.toInt(&ok);
				if (ok && value >= 1 && value <= t.order())
				{
					;
				}
				else
				{
					item->setText(QString::number(a));
					item->setBackground(QBrush(Qt::green));
				}
				item->setData(Qt::ToolTipRole, "");
			}
			else
			{
				QStringList sl;
				for (int x : t.productList(i, j))
				{
					sl << QString::number(x + 1);
				}
				item->setData(Qt::ToolTipRole, sl.join(","));
			}
		}
	}
}
