#pragma once
#include <SFML/Graphics.hpp>
#include "sparse_matrix.h"
#include "operations.h"
#include <string>

class GUI {
public:
    GUI(SparseMatrix& sm);
    void run(); // loop principal: handle events → render

private:
    SparseMatrix& sheet;
    sf::RenderWindow window;
    sf::Font font;

    // Estado de la grilla
    int selectedRow, selectedCol;
    int scrollRow, scrollCol;
    std::string inputBuffer;   // texto que el usuario está escribiendo
    bool editing;              // true = modo edición de celda
    std::string statusMsg;     // mensaje de resultado en barra inferior

    // Dimensiones de la grilla
    static const int CELL_W   = 90;
    static const int CELL_H   = 28;
    static const int HDR_W    = 45;  // ancho cabecera de fila (números)
    static const int HDR_H    = 28;  // alto cabecera de columna (letras)
    static const int VIS_ROWS = 20;
    static const int VIS_COLS = 9;
    static const int PANEL_X  = HDR_W + VIS_COLS * CELL_W + 10; // panel derecho

    void handleEvents();
    void render();
    void renderGrid();
    void renderRightPanel();
    void renderStatusBar();
    void commitInput();

    // Helpers
    std::string colLabel(int col) const;
    std::string getCellDisplay(int row, int col) const; // evalúa fórmulas
    void drawText(const std::string& s, float x, float y, int size,
                  sf::Color color = sf::Color(30, 30, 30));
    void drawRect(float x, float y, float w, float h,
                  sf::Color fill, sf::Color outline = sf::Color(180,180,180), float outlineW = 0.5f);
    bool tryLoadFont();
};