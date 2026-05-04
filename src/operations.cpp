#include "operations.h"
#include <sstream>
#include <cctype>
#include <vector>
#include <stdexcept>

using namespace std;

bool Operations::isNumeric(const string& s) {
    if (s.empty()) return false;
    try { stod(s); return true; }
    catch (...) { return false; }
}

double Operations::toDouble(const string& s) {
    try { return stod(s); }
    catch (...) { return 0.0; }
}

pair<int,int> Operations::parseCellRef(const string& ref) {
    if (ref.empty()) throw invalid_argument("Invalid ref");

    int col = 0, i = 0;

    while (i < (int)ref.size() && isalpha(ref[i])) {
        col = col * 26 + (toupper(ref[i]) - 'A' + 1);
        i++;
    }

    if (i == 0 || i >= (int)ref.size()) throw invalid_argument("Invalid ref");

    col--;

    int row = stoi(ref.substr(i)) - 1;
    if (row < 0 || col < 0) throw invalid_argument("Invalid ref");

    return {row, col};
}

// --- SUMA FILA ---
double Operations::sumRow(const SparseMatrix& sm, int row) {
    double total = 0.0;
    RowHeader* rh = sm.getRowList();

    while (rh && rh->row != row) rh = rh->next;
    if (!rh) return 0.0;

    Cell* c = rh->first;
    while (c) {
        if (isNumeric(c->value)) total += toDouble(c->value);
        c = c->nextInRow;
    }

    return total;
}

// --- SUMA COLUMNA ---
double Operations::sumCol(const SparseMatrix& sm, int col) {
    double total = 0.0;
    ColHeader* ch = sm.getColList();

    while (ch && ch->col != col) ch = ch->next;
    if (!ch) return 0.0;

    Cell* c = ch->first;
    while (c) {
        if (isNumeric(c->value)) total += toDouble(c->value);
        c = c->nextInCol;
    }

    return total;
}

// --- SUMA RANGO ---
double Operations::sumRange(const SparseMatrix& sm, int r1, int c1, int r2, int c2) {
    double total = 0.0;
    RowHeader* rh = sm.getRowList();

    while (rh) {
        if (rh->row >= r1 && rh->row <= r2) {
            Cell* c = rh->first;
            while (c) {
                if (c->col >= c1 && c->col <= c2 && isNumeric(c->value))
                    total += toDouble(c->value);
                c = c->nextInRow;
            }
        }
        rh = rh->next;
    }

    return total;
}

// --- PROMEDIO FILA ---
double Operations::avgRow(const SparseMatrix& sm, int row) {
    double total = 0.0;
    int count = 0;

    RowHeader* rh = sm.getRowList();
    while (rh && rh->row != row) rh = rh->next;
    if (!rh) return 0.0;

    Cell* c = rh->first;
    while (c) {
        if (isNumeric(c->value)) {
            total += toDouble(c->value);
            count++;
        }
        c = c->nextInRow;
    }

    return count ? total / count : 0.0;
}

// --- PROMEDIO COLUMNA ---
double Operations::avgCol(const SparseMatrix& sm, int col) {
    double total = 0.0;
    int count = 0;

    ColHeader* ch = sm.getColList();
    while (ch && ch->col != col) ch = ch->next;
    if (!ch) return 0.0;

    Cell* c = ch->first;
    while (c) {
        if (isNumeric(c->value)) {
            total += toDouble(c->value);
            count++;
        }
        c = c->nextInCol;
    }

    return count ? total / count : 0.0;
}

// --- PROMEDIO RANGO ---
double Operations::avgRange(const SparseMatrix& sm, int r1, int c1, int r2, int c2) {
    double total = 0.0;
    int count = 0;

    RowHeader* rh = sm.getRowList();
    while (rh) {
        if (rh->row >= r1 && rh->row <= r2) {
            Cell* c = rh->first;
            while (c) {
                if (c->col >= c1 && c->col <= c2 && isNumeric(c->value)) {
                    total += toDouble(c->value);
                    count++;
                }
                c = c->nextInRow;
            }
        }
        rh = rh->next;
    }

    return count ? total / count : 0.0;
}

// --- MAX ---
optional<double> Operations::maxRange(const SparseMatrix& sm, int r1, int c1, int r2, int c2) {
    optional<double> maxVal;

    RowHeader* rh = sm.getRowList();
    while (rh) {
        if (rh->row >= r1 && rh->row <= r2) {
            Cell* c = rh->first;
            while (c) {
                if (c->col >= c1 && c->col <= c2 && isNumeric(c->value)) {
                    double v = toDouble(c->value);
                    if (!maxVal || v > *maxVal) maxVal = v;
                }
                c = c->nextInRow;
            }
        }
        rh = rh->next;
    }

    return maxVal;
}

// --- MIN ---
optional<double> Operations::minRange(const SparseMatrix& sm, int r1, int c1, int r2, int c2) {
    optional<double> minVal;

    RowHeader* rh = sm.getRowList();
    while (rh) {
        if (rh->row >= r1 && rh->row <= r2) {
            Cell* c = rh->first;
            while (c) {
                if (c->col >= c1 && c->col <= c2 && isNumeric(c->value)) {
                    double v = toDouble(c->value);
                    if (!minVal || v < *minVal) minVal = v;
                }
                c = c->nextInRow;
            }
        }
        rh = rh->next;
    }

    return minVal;
}

// --- FORMULAS ---
string Operations::evaluateFormula(const SparseMatrix& sm, const string& formula) {
    if (formula.empty() || formula[0] != '=') return formula;

    string expr = formula.substr(1);
    int i = 0;
    int len = expr.size();

    auto skip = [&]() {
        while (i < len && isspace(expr[i])) i++;
    };

    auto read = [&]() -> double {
        skip();
        if (i >= len) throw runtime_error("error");

        string token;
        while (i < len && (isalnum(expr[i]) || expr[i] == '.'))
            token += expr[i++];

        if (token.empty()) throw runtime_error("error");

        if (isalpha(token[0])) {
            auto [r, c] = parseCellRef(token);
            string val = sm.queryCell(r, c);
            return val.empty() ? 0.0 : toDouble(val);
        }

        return stod(token);
    };

    try {
        vector<double> values;
        vector<char> ops;

        values.push_back(read());
        skip();

        while (i < len) {
            char op = expr[i++];
            if (op!='+' && op!='-' && op!='*' && op!='/')
                throw runtime_error("error");

            double val = read();

            if (op=='*' || op=='/') {
                double left = values.back();
                values.pop_back();
                values.push_back(op=='*' ? left*val : (val!=0 ? left/val : 0.0));
            } else {
                ops.push_back(op);
                values.push_back(val);
            }

            skip();
        }

        double res = values[0];
        for (int j=0;j<ops.size();j++) {
            if (ops[j]=='+') res += values[j+1];
            else res -= values[j+1];
        }

        ostringstream out;
        if (abs(res - (int)res) < 1e-9) out << (int)res;
        else out << res;

        return out.str();

    } catch (...) {
        return "#ERROR";
    }
}