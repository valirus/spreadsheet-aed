#pragma once
#include <SFML/Graphics.hpp>
#include "sparse_matrix.h"
#include "operations.h"
#include <string>

using namespace std;

class GUI {
public:
    GUI(SparseMatrix& sm);
    void run();

private:
    SparseMatrix& sheet;
    sf::RenderWindow window;
    sf::Font font;

    int selectedRow, selectedCol;
    int anchorRow, anchorCol;  
    int scrollRow, scrollCol;
    string inputBuffer;
    bool editing;
    string statusMsg;

    static const int CELL_W   = 90;
    static const int CELL_H   = 28;
    static const int HDR_W    = 45;
    static const int HDR_H    = 28;
    static const int VIS_ROWS = 20;
    static const int VIS_COLS = 9;
    static const int PANEL_X  = HDR_W + VIS_COLS * CELL_W + 10;

    void handleEvents();
    void render();
    void renderGrid();
    void renderRightPanel();
    void renderStatusBar();
    void commitInput();

    // Helpers
    string colLabel(int col) const;
    string getCellDisplay(int row, int col) const;
    void drawText(const string& s, float x, float y, int size,
                  sf::Color color = sf::Color(30, 30, 30));
    void drawRect(float x, float y, float w, float h,
                  sf::Color fill, sf::Color outline = sf::Color(180,180,180), float outlineW = 0.5f);
    bool tryLoadFont();
};