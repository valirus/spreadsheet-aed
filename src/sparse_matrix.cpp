#include "sparse_matrix.h"
#include <vector>

using namespace std;

SparseMatrix::SparseMatrix() : rowList(nullptr), colList(nullptr) {}

SparseMatrix::~SparseMatrix() {
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
    ColHeader* ch = colList;
    while (ch) {
        ColHeader* next = ch->next;
        delete ch;
        ch = next;
    }
}

RowHeader* SparseMatrix::findRowHeader(int row) const {
    RowHeader* rh = rowList;
    while (rh && rh->row < row) rh = rh->next;
    if (rh && rh->row == row) return rh;
    return nullptr;
}

ColHeader* SparseMatrix::findColHeader(int col) const {
    ColHeader* ch = colList;
    while (ch && ch->col < col) ch = ch->next;
    if (ch && ch->col == col) return ch;
    return nullptr;
}

RowHeader* SparseMatrix::getOrCreateRowHeader(int row) {
    if (!rowList || rowList->row > row) {
        RowHeader* rh = new RowHeader(row);
        rh->next = rowList;
        rowList = rh;
        return rh;
    }
    if (rowList->row == row) return rowList; 

    RowHeader* curr = rowList;
    while (curr->next && curr->next->row < row) curr = curr->next;
    if (curr->next && curr->next->row == row) return curr->next;
    RowHeader* rh = new RowHeader(row);
    rh->next = curr->next;
    curr->next = rh;
    return rh;
}

ColHeader* SparseMatrix::getOrCreateColHeader(int col) {
    if (!colList || colList->col > col) {
        ColHeader* ch = new ColHeader(col);
        ch->next = colList;
        colList = ch;
        return ch;
    }
    if (colList->col == col) return colList;

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
    while (prev->next != nullptr && prev->next != rh) {
        prev = prev->next;
    }
    
    if (prev->next != nullptr) {
        prev->next = rh->next;
        delete rh;
    }
}

void SparseMatrix::removeColHeaderIfEmpty(ColHeader* ch) {
    if (!ch || ch->first) return;
    if (colList == ch) { colList = ch->next; delete ch; return; }
    
    ColHeader* prev = colList;
    while (prev->next != nullptr && prev->next != ch) {
        prev = prev->next;
    }
    
    if (prev->next != nullptr) {
        prev->next = ch->next;
        delete ch;
    }
}

// INSERTAR/ACTUALIZAR celda
void SparseMatrix::insertCell(int row, int col, const string& value) {
    if (value.empty()) { deleteCell(row, col); return; }

    RowHeader* existingRh = findRowHeader(row);
    if (existingRh) {
        Cell* c = existingRh->first;
        while (c && c->col < col) c = c->nextInRow;
        if (c && c->col == col) { c->value = value; return; }
    }

    Cell* newCell = new Cell(row, col, value);

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

// CONSULTAR celda
string SparseMatrix::queryCell(int row, int col) const {
    RowHeader* rh = findRowHeader(row);
    if (!rh) return "";
    Cell* c = rh->first;
    while (c && c->col < col) c = c->nextInRow;
    if (c && c->col == col) return c->value;
    return "";
}

// MODIFICAR celda
void SparseMatrix::modifyCell(int row, int col, const string& value) {
    insertCell(row, col, value);
}

// ELIMINAR celda
void SparseMatrix::deleteCell(int row, int col) {
    RowHeader* rh = findRowHeader(row);
    if (!rh) return;

    Cell* target = nullptr;
    if (rh->first && rh->first->col == col) {
        target = rh->first;
        rh->first = target->nextInRow;
    } else {
        Cell* prev = rh->first;
        while (prev && prev->nextInRow && prev->nextInRow->col != col)
            prev = prev->nextInRow;
        if (!prev || !prev->nextInRow) return;
        target = prev->nextInRow;
        prev->nextInRow = target->nextInRow;
    }

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

// ELIMINAR FILA
void SparseMatrix::deleteRow(int row) {
    RowHeader* rh = findRowHeader(row);
    if (!rh) return;

    Cell* cell = rh->first;
    while (cell) {
        Cell* next = cell->nextInRow;
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

    if (rowList == rh) {
        rowList = rh->next;
    } else {
        RowHeader* prev = rowList;
        while (prev->next != rh) prev = prev->next;
        prev->next = rh->next;
    }
    delete rh;
}

// ELIMINAR COLUMNA
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

// ELIMINAR RANGO
void SparseMatrix::deleteRange(int r1, int c1, int r2, int c2) {
    vector<pair<int,int>> toDelete;
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