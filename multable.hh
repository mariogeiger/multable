#ifndef MULTABLE_HH
#define MULTABLE_HH

#include <QVector>
#include <QList>
#include <QString>

class MulTable
{
public:
  MulTable(int order = 0);

  bool setProduct(int a, int b, int c);
  void setProductList(int a, int b, QList<int> c) {
    content[a][b] = c;
  }

  int product(int a, int b) const {
    if (content[a][b].size() == 1)
      return content[a][b].first();
    return -1;
  }
  QList<int> productList(int a, int b) const {
    return content[a][b];
  }

  int order() const { return content.size(); }

  bool isComplete() const;
  int isGroup() const;

  bool reduce();
  QList<MulTable> brute();
  bool hasEmptyEntry() const;
private:
  QVector<QVector<QList<int>>> content;
};

#endif // MULTABLE_HH
