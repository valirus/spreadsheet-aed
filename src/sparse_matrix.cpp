#include "sparse_matrix.h"
#include <vector>

SparseMatrix::SparseMatrix() : rowList(nullptr), colList(nullptr) {}

SparseMatrix::~SparseMatrix() {
    // Liberar todos los nodos recorriendo por filas
    RowHeader* rh = rowList;
    while (rh) {
        Cell* cell = rh->first;
        while (cell) {
            Cell* next = cell->nextInRow;
            delete cell;
            cell = next;
        }
        RowHeader* nextRh = rh->next;
        delete rh;
        rh = nextRh;
    }
    // Liberar cabeceras de columna (celdas ya liberadas)
    ColHeader* ch = colList;
    while (ch) {
        ColHeader* next = ch->next;
        delete ch;
        ch = next;
    }
}

// Busca la cabecera de fila. Retorna nullptr si no existe.
RowHeader* SparseMatrix::findRowHeader(int row) const {
    RowHeader* rh = rowList;
    while (rh && rh->row < row) rh = rh->next;
    if (rh && rh->row == row) return rh;
    return nullptr;
}

// Busca la cabecera de columna. Retorna nullptr si no existe.
ColHeader* SparseMatrix::findColHeader(int col) const {
    ColHeader* ch = colList;
    while (ch && ch->col < col) ch = ch->next;
    if (ch && ch->col == col) return ch;
    return nullptr;
}

// Obtiene o crea una cabecera de fila, insertando en orden.
RowHeader* SparseMatrix::getOrCreateRowHeader(int row) {
    if (!rowList || rowList->row > row) {
        RowHeader* rh = new RowHeader(row);
        rh->next = rowList;
        rowList = rh;
        return rh;
    }
    RowHeader* curr = rowList;
    while (curr->next && curr->next->row < row) curr = curr->next;
    if (curr->next && curr->next->row == row) return curr->next;
    RowHeader* rh = new RowHeader(row);
    rh->next = curr->next;
    curr->next = rh;
    return rh;
}

// Igual pero para columnas.
ColHeader* SparseMatrix::getOrCreateColHeader(int col) {
    if (!colList || colList->col > col) {
        ColHeader* ch = new ColHeader(col);
        ch->next = colList;
        colList = ch;
        return ch;
    }
    ColHeader* curr = colList;
    while (curr->next && curr->next->col < col) curr = curr->next;
    if (curr->next && curr->next->col == col) return curr->next;
    ColHeader* ch = new ColHeader(col);
    ch->next = curr->next;
    curr->next = ch;
    return ch;
}

void SparseMatrix::removeRowHeaderIfEmpty(RowHeader* rh) {
    if (!rh || rh->first) return;
    if (rowList == rh) { rowList = rh->next; delete rh; return; }
    RowHeader* prev = rowList;
    while (prev->next != rh) prev = prev->next;
    prev->next = rh->next;
    delete rh;
}

void SparseMatrix::removeColHeaderIfEmpty(ColHeader* ch) {
    if (!ch || ch->first) return;
    if (colList == ch) { colList = ch->next; delete ch; return; }
    ColHeader* prev = colList;
    while (prev->next != ch) prev = prev->next;
    prev->next = ch->next;
    delete ch;
}

// INSERTAR/ACTUALIZAR celda. Complejidad: O(k) donde k = celdas en fila/columna
void SparseMatrix::insertCell(int row, int col, const std::string& value) {
    if (value.empty()) { deleteCell(row, col); return; }

    // ¿Ya existe? Solo actualiza el valor, sin crear nodo nuevo.
    RowHeader* existingRh = findRowHeader(row);
    if (existingRh) {
        Cell* c = existingRh->first;
        while (c && c->col < col) c = c->nextInRow;
        if (c && c->col == col) { c->value = value; return; }
    }

    // Crear nuevo nodo
    Cell* newCell = new Cell(row, col, value);

    // Insertar en lista de fila (ordenado por columna)
    RowHeader* rowHdr = getOrCreateRowHeader(row);
    if (!rowHdr->first || rowHdr->first->col > col) {
        newCell->nextInRow = rowHdr->first;
        rowHdr->first = newCell;
    } else {
        Cell* curr = rowHdr->first;
        while (curr->nextInRow && curr->nextInRow->col < col)
            curr = curr->nextInRow;
        newCell->nextInRow = curr->nextInRow;
        curr->nextInRow = newCell;
    }

    // Insertar en lista de columna (ordenado por fila)
    ColHeader* colHdr = getOrCreateColHeader(col);
    if (!colHdr->first || colHdr->first->row > row) {
        newCell->nextInCol = colHdr->first;
        colHdr->first = newCell;
    } else {
        Cell* curr = colHdr->first;
        while (curr->nextInCol && curr->nextInCol->row < row)
            curr = curr->nextInCol;
        newCell->nextInCol = curr->nextInCol;
        curr->nextInCol = newCell;
    }
}

// CONSULTAR celda. Devuelve "" si no existe (nunca crash). O(k)
std::string SparseMatrix::queryCell(int row, int col) const {
    RowHeader* rh = findRowHeader(row);
    if (!rh) return "";
    Cell* c = rh->first;
    while (c && c->col < col) c = c->nextInRow;
    if (c && c->col == col) return c->value;
    return "";
}

// MODIFICAR: upsert (inserta si no existe, actualiza si existe)
void SparseMatrix::modifyCell(int row, int col, const std::string& value) {
    insertCell(row, col, value);
}

// ELIMINAR celda. Ajusta punteros en AMBAS listas. O(k)
// Esta es la operación más delicada — presta atención a los punteros.
void SparseMatrix::deleteCell(int row, int col) {
    RowHeader* rh = findRowHeader(row);
    if (!rh) return; // celda no existe → no crashear

    // Encontrar y desenlazar de la lista de fila
    Cell* target = nullptr;
    if (rh->first && rh->first->col == col) {
        target = rh->first;
        rh->first = target->nextInRow;
    } else {
        Cell* prev = rh->first;
        while (prev && prev->nextInRow && prev->nextInRow->col != col)
            prev = prev->nextInRow;
        if (!prev || !prev->nextInRow) return; // no existe
        target = prev->nextInRow;
        prev->nextInRow = target->nextInRow;
    }

    // Desenlazar de la lista de columna
    ColHeader* ch = findColHeader(col);
    if (ch) {
        if (ch->first == target) {
            ch->first = target->nextInCol;
        } else {
            Cell* prev = ch->first;
            while (prev && prev->nextInCol != target)
                prev = prev->nextInCol;
            if (prev) prev->nextInCol = target->nextInCol;
        }
        removeColHeaderIfEmpty(ch);
    }

    removeRowHeaderIfEmpty(rh);
    delete target;
}

// ELIMINAR FILA: recorre la fila y elimina cada celda de sus columnas. O(k)
void SparseMatrix::deleteRow(int row) {
    RowHeader* rh = findRowHeader(row);
    if (!rh) return; // fila vacía → no crashear

    Cell* cell = rh->first;
    while (cell) {
        Cell* next = cell->nextInRow;
        // Desenlazar de columna
        ColHeader* ch = findColHeader(cell->col);
        if (ch) {
            if (ch->first == cell) {
                ch->first = cell->nextInCol;
            } else {
                Cell* prev = ch->first;
                while (prev && prev->nextInCol != cell) prev = prev->nextInCol;
                if (prev) prev->nextInCol = cell->nextInCol;
            }
            removeColHeaderIfEmpty(ch);
        }
        delete cell;
        cell = next;
    }

    // Eliminar cabecera de fila
    if (rowList == rh) {
        rowList = rh->next;
    } else {
        RowHeader* prev = rowList;
        while (prev->next != rh) prev = prev->next;
        prev->next = rh->next;
    }
    delete rh;
}

// ELIMINAR COLUMNA: análogo a deleteRow. O(k)
void SparseMatrix::deleteCol(int col) {
    ColHeader* ch = findColHeader(col);
    if (!ch) return;

    Cell* cell = ch->first;
    while (cell) {
        Cell* next = cell->nextInCol;
        RowHeader* rh = findRowHeader(cell->row);
        if (rh) {
            if (rh->first == cell) {
                rh->first = cell->nextInRow;
            } else {
                Cell* prev = rh->first;
                while (prev && prev->nextInRow != cell) prev = prev->nextInRow;
                if (prev) prev->nextInRow = cell->nextInRow;
            }
            removeRowHeaderIfEmpty(rh);
        }
        delete cell;
        cell = next;
    }

    if (colList == ch) {
        colList = ch->next;
    } else {
        ColHeader* prev = colList;
        while (prev->next != ch) prev = prev->next;
        prev->next = ch->next;
    }
    delete ch;
}

// ELIMINAR RANGO rectangular. O(k*m) donde k,m = celdas en el rango.
void SparseMatrix::deleteRange(int r1, int c1, int r2, int c2) {
    // Recolectar coordenadas primero para no modificar mientras iteramos
    std::vector<std::pair<int,int>> toDelete;
    RowHeader* rh = rowList;
    while (rh) {
        if (rh->row >= r1 && rh->row <= r2) {
            Cell* c = rh->first;
            while (c) {
                if (c->col >= c1 && c->col <= c2)
                    toDelete.push_back({c->row, c->col});
                c = c->nextInRow;
            }
        }
        rh = rh->next;
    }
    for (auto& [r, c] : toDelete) deleteCell(r, c);
}