#include "multable.hh"

MulTable::MulTable(int order)
{
  content.resize(order);

  QList<int> any;
  for (int i = 0; i < order; ++i) {
    any << i;
  }
  for (int i = 0; i < content.size(); ++i) {
    content[i].fill(any, order);
  }
}

bool MulTable::setProduct(int a, int b, int c)
{
  if (content[a][b].size() == 1 && content[a][b].first() == c)
    return false;

  bool empty = content[a][b].isEmpty();
  content[a][b].clear();
  content[a][b] << c;
  return !empty;
}

bool MulTable::isComplete() const
{
  for (int i = 0; i < order(); ++i) {
    for (int j = 0; j < order(); ++j) {
      if (content[i][j].size() != 1) return false;
    }
  }
  return true;
}

int MulTable::isGroup() const
{
  if (hasEmptyEntry()) return -1;

  // identity
  QVector<int> is;
  for (int i = 0; i < order(); ++i) {
    bool canBeTheInverse = true;
    for (int j = 0; j < order(); ++j) {
      if (!productList(i,j).contains(j) || !productList(j,i).contains(j)) {
        canBeTheInverse = false;
      }
    }
    if (canBeTheInverse) is.append(i);
  }
  if (is.isEmpty()) {
    return -1;
  }

  // inverse
  if (is.size() == 1) {
    int e = is.first();
    for (int i = 0; i < order(); ++i) {
      bool hasInverse = false;
      for (int j = 0; j < order(); ++j) {
        if (product(i,j) == e && !productList(j,i).contains(e)) {
          return -1;
        }
        if (productList(i,j).contains(e)) hasInverse = true;
      }
      if (!hasInverse) return -1;
    }
  }
  // associativity
  for (int i = 0; i < order(); ++i) {
    for (int j = 0; j < order(); ++j) {
      for (int k = 0; k < order(); ++k) {
        // (ij)k = i(jk)
        int a = product(i,j);
        int b = product(j,k);
        if (a != -1 && b != -1) {
          a = product(a,k);
          b = product(i,b);
          if (a != -1 && b != -1 && a != b) {
            return -1;
          }
        }
      }
    }
  }

  // column/row
  for (int i = 0; i < order(); ++i) {
    QVector<int> rs, cs;
    for (int j = 0; j < order(); ++j) {
      int a = product(i,j);
      if (a != -1) {
        if (rs.contains(a)) {
          return -1;
        }
        rs << a;
      }
      a = product(j,i);
      if (a != -1) {
        if (cs.contains(a)) {
          return -1;
        }
        cs << a;
      }
    }
  }

  return isComplete() ? 1 : 0;
}

bool MulTable::reduce()
{
  bool didit = false;

  QVector<int> is;
  for (int i = 0; i < order(); ++i) {
    bool canBeTheInverse = true;
    for (int j = 0; j < order(); ++j) {
      if (!productList(i,j).contains(j) || !productList(j,i).contains(j)) {
        canBeTheInverse = false;
      }
    }
    if (canBeTheInverse) is.append(i);
  }
  int e = -1;
  if (is.size() == 1) e = is.first();

  if (e != -1) {
    for (int i = 0; i < order(); ++i) {
      didit = setProduct(i,e,i) || didit;
      didit = setProduct(e,i,i) || didit;
    }
  }

  // apear only once in a row/column
  for (int i = 0; i < order(); ++i) {
    for (int j = 0; j < order(); ++j) {
      int a = product(i,j);
      if (a != -1) {
        for (int k = 0; k < order(); ++k) {
          if (k != j)
            didit = content[i][k].removeAll(a) > 0 || didit;
          if (k != i)
            didit = content[k][j].removeAll(a) > 0 || didit;
        }
        if (e != -1) {
          // only one inverse
          if (a == e) {
            didit = setProduct(j,i,e) || didit;
          }
        }
      }
    }
  }

  // associativity
  for (int i = 0; i < order(); ++i) {
    for (int j = 0; j < order(); ++j) {
      for (int k = 0; k < order(); ++k) {
        int a = product(i,j);
        int b = product(j,k);
        if (a != -1 && b != -1) {
          int c = product(a, k);
          if (c != -1) {
            didit = setProduct(i, b, c) || didit;
          }
        }
      }
    }
  }

  return didit;
}

QList<MulTable> MulTable::brute()
{
  QList<MulTable> solutions;

  for (int i = 0; i < 4; ++i) reduce();

  int r = isGroup();
  if (r == -1) return solutions;
  if (r == 1) {
    solutions << *this;
    return solutions;
  }

  int minEntry = order();
  int ii = 0, jj = 0;
  for (int i = 0; i < order(); ++i) {
    for (int j = 0; j < order(); ++j) {
      if (content[i][j].size() > 1 && content[i][j].size() < minEntry) {
        minEntry = content[i][j].size();
        ii = i;
        jj = j;
      }
    }
  }

  QList<int> as = content[ii][jj];
  for (int a : as) {
    MulTable table(*this);
    table.setProduct(ii, jj, a);
    solutions.append(table.brute());
    if (solutions.size() > 20) break;
  }

  return solutions;
}

bool MulTable::hasEmptyEntry() const
{
  for (int i = 0; i < order(); ++i) {
    for (int j = 0; j < order(); ++j) {
      if (content[i][j].isEmpty()) {
        return true;
      }
    }
  }
  return false;
}
