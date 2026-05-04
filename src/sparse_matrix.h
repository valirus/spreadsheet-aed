#pragma once
#include "cell.h"
#include <string>

class SparseMatrix {
private:
    RowHeader* rowList;   // lista enlazada de cabeceras de fila (ordenada)
    ColHeader* colList;   // lista enlazada de cabeceras de columna (ordenada)

    // Helpers internos
    RowHeader* findRowHeader(int row) const;
    ColHeader* findColHeader(int col) const;
    RowHeader* getOrCreateRowHeader(int row);
    ColHeader* getOrCreateColHeader(int col);
    void removeRowHeaderIfEmpty(RowHeader* rh);
    void removeColHeaderIfEmpty(ColHeader* ch);

public:
    SparseMatrix();
    ~SparseMatrix();

    // --- Operaciones básicas de celda ---
    // Insertar/actualizar: si ya existe, modifica. Si no, crea nodo. O(k)
    void insertCell(int row, int col, const std::string& value);
    // Consultar: devuelve "" si no existe. O(k)
    std::string queryCell(int row, int col) const;
    // Modificar (alias de insert con nodo existente)
    void modifyCell(int row, int col, const std::string& value);
    // Eliminar: ajusta punteros en fila Y columna. O(k)
    void deleteCell(int row, int col);

    // --- Operaciones sobre filas/columnas/rangos ---
    void deleteRow(int row);
    void deleteCol(int col);
    void deleteRange(int r1, int c1, int r2, int c2);

    // --- Acceso para operaciones y GUI ---
    RowHeader* getRowList() const { return rowList; }
    ColHeader* getColList() const { return colList; }
};