#pragma once
#include <string>

using namespace std;

struct Cell {
    int row, col;
    string value;
    Cell* nextInRow;
    Cell* nextInCol;

    Cell(int r, int c, const string& v)
        : row(r), col(c), value(v), nextInRow(nullptr), nextInCol(nullptr) {}
};

struct RowHeader {
    int row;
    Cell* first;
    RowHeader* next;

    RowHeader(int r) : row(r), first(nullptr), next(nullptr) {}
};

struct ColHeader {
    int col;
    Cell* first;
    ColHeader* next;

    ColHeader(int c) : col(c), first(nullptr), next(nullptr) {}
};