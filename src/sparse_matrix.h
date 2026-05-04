#pragma once
#include "cell.h"
#include <string>

using namespace std;

class SparseMatrix {
private:
    RowHeader* rowList;
    ColHeader* colList;

    RowHeader* findRowHeader(int row) const;
    ColHeader* findColHeader(int col) const;
    RowHeader* getOrCreateRowHeader(int row);
    ColHeader* getOrCreateColHeader(int col);
    void removeRowHeaderIfEmpty(RowHeader* rh);
    void removeColHeaderIfEmpty(ColHeader* ch);

public:
    SparseMatrix();
    ~SparseMatrix();


    SparseMatrix(const SparseMatrix&) = delete;
    SparseMatrix& operator=(const SparseMatrix&) = delete;
    // --- Operaciones básicas de celda ---
    void insertCell(int row, int col, const string& value);
    string queryCell(int row, int col) const;
    void modifyCell(int row, int col, const string& value);
    void deleteCell(int row, int col);

    // --- Operaciones sobre filas/columnas/rangos ---
    void deleteRow(int row);
    void deleteCol(int col);
    void deleteRange(int r1, int c1, int r2, int c2);

    // --- Acceso para operaciones y GUI ---
    RowHeader* getRowList() const { return rowList; }
    ColHeader* getColList() const { return colList; }
};