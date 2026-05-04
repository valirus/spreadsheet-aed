#pragma once
#include <string>

// Nodo principal: representa UNA celda no vacía
// Vive simultáneamente en dos listas enlazadas: su fila y su columna
struct Cell {
    int row, col;          // coordenadas (0-indexed)
    std::string value;     // contenido: "42", "hola", "=A1+B1"
    Cell* nextInRow;       // → siguiente celda en la misma fila
    Cell* nextInCol;       // ↓ siguiente celda en la misma columna

    Cell(int r, int c, const std::string& v)
        : row(r), col(c), value(v), nextInRow(nullptr), nextInCol(nullptr) {}
};

// Cabecera de fila: guarda el primer nodo de esa fila
// Las cabeceras están ordenadas por número de fila
struct RowHeader {
    int row;
    Cell* first;        // primer nodo de esta fila
    RowHeader* next;    // siguiente cabecera de fila

    RowHeader(int r) : row(r), first(nullptr), next(nullptr) {}
};

// Cabecera de columna: guarda el primer nodo de esa columna
struct ColHeader {
    int col;
    Cell* first;        // primer nodo de esta columna
    ColHeader* next;    // siguiente cabecera de columna

    ColHeader(int c) : col(c), first(nullptr), next(nullptr) {}
};