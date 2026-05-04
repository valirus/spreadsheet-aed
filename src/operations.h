#pragma once
#include "sparse_matrix.h"
#include <string>
#include <optional>
#include <utility>
#include <unordered_set>


using namespace std;

class Operations {
public:
    // --- Agregaciones ---
    static double sumRow(const SparseMatrix& sm, int row);
    static double sumCol(const SparseMatrix& sm, int col);
    static double sumRange(const SparseMatrix& sm, int r1, int c1, int r2, int c2);

    static double avgRow(const SparseMatrix& sm, int row);
    static double avgCol(const SparseMatrix& sm, int col);
    static double avgRange(const SparseMatrix& sm, int r1, int c1, int r2, int c2);

    static optional<double> maxRange(const SparseMatrix& sm, int r1, int c1, int r2, int c2);
    static optional<double> minRange(const SparseMatrix& sm, int r1, int c1, int r2, int c2);

    
    // --- Fórmulas ---
    static string evaluateFormula(const SparseMatrix& sm, const string& formula, int depth = 0);

    // Helpers
    static bool isNumeric(const string& s);
    static pair<int,int> parseCellRef(const string& ref);

private:
    static double toDouble(const string& s);
};