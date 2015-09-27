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

void MulTable::setProduct(int a, int b, int c)
{
	content[a][b].clear();
	content[a][b] << c;
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
	MulTable original = *this;

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
			filterProduct(i,e,i);
			filterProduct(e,i,i);
		}
	}

	// apear only once in a row/column
	for (int i = 0; i < order(); ++i) {
		for (int j = 0; j < order(); ++j) {
			int a = product(i,j);
			if (a != -1) {
				for (int k = 0; k < order(); ++k) {
					if (k != j)
						content[i][k].removeAll(a);
					if (k != i)
						content[k][j].removeAll(a);
				}
				if (e != -1) {
					// only one inverse
					if (a == e) {
						filterProduct(j,i,e);
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
						filterProduct(i, b, c);
					}
				}
			}
		}
	}

	// if only 1 possibility in a row/column
	for (int i = 0; i < order(); ++i) { // foreach row
		for (int a = 0; a < order(); ++a) { // foreach value
			QList<int> occur;
			for (int j = 0; j < order(); ++j) {
				if (content[i][j].contains(a)) occur << j;
			}
			if (occur.size() == 1) {
				content[i][occur.first()].clear();
				content[i][occur.first()] << a;
			}
		}
	}
	for (int j = 0; j < order(); ++j) { // foreach column
		for (int a = 0; a < order(); ++a) { // foreach value
			QList<int> occur;
			for (int i = 0; i < order(); ++i) {
				if (content[i][j].contains(a)) occur << i;
			}
			if (occur.size() == 1) {
				content[occur.first()][j].clear();
				content[occur.first()][j] << a;
			}
		}
	}

	return original != *this;
}

QList<MulTable> MulTable::brute(int& limit, bool root)
{
	QList<MulTable> solutions;
	if (limit <= 0) {
		return solutions;
	}

	while (reduce());

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
		limit--;
		MulTable table(*this);
		table.setProduct(ii, jj, a);
		solutions.append(table.brute(limit, false));
		if (limit <= 0) {
			break;
		}
	}

	if (root && limit > 0) {
		for (int i = 0; i < order(); ++i) {
			for (int j = 0; j < order(); ++j) {
				int k = -1;
				for (const MulTable& x : solutions) {
					int kk = x.product(i, j);
					if (k == -1) k = kk;
					if (k != kk) k = -2;
				}
				if (k >= 0) setProduct(i, j, k);
			}
		}
	}

	return solutions;
}

QList<int> MulTable::eqclass(int a) const
{
	QList<int> list;
	if (isGroup() != 1) return list;

	for (int b = 0; b < order(); ++b) {
		for (int i = 0; i < order(); ++i) {
			if (product(i, a) == product(b, i)) {
				list.push_back(b);
				break;
			}
		}
	}
	return list;
}

bool MulTable::operator !=(const MulTable& other) const
{
	return content != other.content;
}

bool MulTable::operator ==(const MulTable& other) const
{
	return content == other.content;
}

void MulTable::filterProduct(int i, int j, int r)
{
	if (content[i][j].contains(r)) {
		content[i][j].clear();
		content[i][j] << r;
	} else {
		content[i][j].clear();
	}
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
