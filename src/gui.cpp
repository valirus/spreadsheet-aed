#include "gui.h"
#include <sstream>
#include <iomanip>
#include <vector>
#include <algorithm>

using namespace std;

GUI::GUI(SparseMatrix& sm)
    : sheet(sm), selectedRow(0), selectedCol(0),
      anchorRow(0), anchorCol(0), 
      scrollRow(0), scrollCol(0), editing(false) {

    unsigned winW = HDR_W + VIS_COLS * CELL_W + 220;
    unsigned winH = HDR_H + VIS_ROWS * CELL_H + 60;
    window.create(sf::VideoMode(winW, winH), "Spreadsheet — CS2023 AED");
    window.setFramerateLimit(60);

    if (!tryLoadFont()) {}
}

bool GUI::tryLoadFont() {
    vector<string> paths = {
        "assets/arial.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
        "/usr/share/fonts/truetype/freefont/FreeMono.ttf",
        "/usr/share/fonts/truetype/ubuntu/UbuntuMono-R.ttf",
        "/usr/share/fonts/TTF/DejaVuSansMono.ttf",
        "C:/Windows/Fonts/consola.ttf",
        "C:/Windows/Fonts/cour.ttf"
    };
    for (auto& p : paths)
        if (font.loadFromFile(p)) return true;
    return false;
}

string GUI::colLabel(int col) const {
    string label;
    col++;
    while (col > 0) {
        col--;
        label = char('A' + col % 26) + label;
        col /= 26;
    }
    return label;
}

string GUI::getCellDisplay(int row, int col) const {
    string val = sheet.queryCell(row, col);
    if (!val.empty() && val[0] == '=')
        return Operations::evaluateFormula(sheet, val);
    return val;
}

void GUI::drawText(const string& s, float x, float y, int size, sf::Color color) {
    sf::Text t(s, font, size);
    t.setPosition(x, y);
    t.setFillColor(color);
    window.draw(t);
}

void GUI::drawRect(float x, float y, float w, float h,
                   sf::Color fill, sf::Color outline, float outlineW) {
    sf::RectangleShape r(sf::Vector2f(w, h));
    r.setPosition(x, y);
    r.setFillColor(fill);
    r.setOutlineColor(outline);
    r.setOutlineThickness(outlineW);
    window.draw(r);
}

void GUI::run() {
    while (window.isOpen()) {
        handleEvents();
        render();
    }
}

void GUI::handleEvents() {
    sf::Event ev;
    while (window.pollEvent(ev)) {
        if (ev.type == sf::Event::Closed)
            window.close();

        // --- Teclas ---
        if (ev.type == sf::Event::KeyPressed) {
            if (!editing) {
                bool isNavKey = false;
                if (ev.key.code == sf::Keyboard::Up    && selectedRow > 0) { selectedRow--; isNavKey = true; }
                if (ev.key.code == sf::Keyboard::Down)                      { selectedRow++; isNavKey = true; }
                if (ev.key.code == sf::Keyboard::Left  && selectedCol > 0) { selectedCol--; isNavKey = true; }
                if (ev.key.code == sf::Keyboard::Right)                     { selectedCol++; isNavKey = true; }

                if (isNavKey) {
                    statusMsg = "";
                    if (!ev.key.shift) {
                        anchorRow = selectedRow;
                        anchorCol = selectedCol;
                    }
                }

                if (ev.key.code == sf::Keyboard::Enter || ev.key.code == sf::Keyboard::F2) {
                    editing = true;
                    inputBuffer = sheet.queryCell(selectedRow, selectedCol);
                }
               if (ev.key.code == sf::Keyboard::Delete) {
                int r1 = min(anchorRow, selectedRow);
                int r2 = max(anchorRow, selectedRow);
                int c1 = min(anchorCol, selectedCol);
                int c2 = max(anchorCol, selectedCol);
                
                sheet.deleteRange(r1, c1, r2, c2);
                statusMsg = (r1 == r2 && c1 == c2) ? "Celda eliminada" : "Rango eliminado";
                }
                if (ev.key.code == sf::Keyboard::D && ev.key.control) {
                    sheet.deleteRow(selectedRow);
                    statusMsg = "Fila " + to_string(selectedRow + 1) + " eliminada";
                }
                if (ev.key.code == sf::Keyboard::K && ev.key.control) {
                    sheet.deleteCol(selectedCol);
                    statusMsg = "Col " + colLabel(selectedCol) + " eliminada";
                }
                if (ev.key.code == sf::Keyboard::Escape) statusMsg = "";

            } else {
                if (ev.key.code == sf::Keyboard::Enter) {
                    commitInput();
                    editing = false;
                    selectedRow++;
                    
                    anchorRow = selectedRow; 
                    anchorCol = selectedCol;
                }
                if (ev.key.code == sf::Keyboard::Escape) {
                    editing = false;
                    inputBuffer = "";
                }
                if (ev.key.code == sf::Keyboard::BackSpace && !inputBuffer.empty())
                    inputBuffer.pop_back();
            }

            if (selectedRow < scrollRow) scrollRow = selectedRow;
            if (selectedRow >= scrollRow + VIS_ROWS) scrollRow = selectedRow - VIS_ROWS + 1;
            if (selectedCol < scrollCol) scrollCol = selectedCol;
            if (selectedCol >= scrollCol + VIS_COLS) scrollCol = selectedCol - VIS_COLS + 1;
        }

        // --- Texto ---
        if (ev.type == sf::Event::TextEntered && editing) {
            char c = static_cast<char>(ev.text.unicode);
            if (c >= 32 && c < 127) inputBuffer += c;
        }

        // --- Click de mouse ---
        if (ev.type == sf::Event::MouseButtonPressed && ev.mouseButton.button == sf::Mouse::Left) {
            int mx = ev.mouseButton.x;
            int my = ev.mouseButton.y;
            if (mx > HDR_W && my > HDR_H && my < HDR_H + VIS_ROWS * CELL_H) {
                if (editing) commitInput();
                editing = false;
                selectedCol = (mx - HDR_W) / CELL_W + scrollCol;
                selectedRow = (my - HDR_H) / CELL_H + scrollRow;
                anchorRow = selectedRow;
                anchorCol = selectedCol;
            }
        }

        // --- Mouse ---
        if (ev.type == sf::Event::MouseMoved) {
            if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && !editing) {
                int mx = ev.mouseMove.x;
                int my = ev.mouseMove.y;
                if (mx > HDR_W && my > HDR_H && my < HDR_H + VIS_ROWS * CELL_H) {
                    selectedCol = (mx - HDR_W) / CELL_W + scrollCol;
                    selectedRow = (my - HDR_H) / CELL_H + scrollRow;
                }
            }
        }
    }
}

void GUI::commitInput() {
    if (!inputBuffer.empty())
        sheet.insertCell(selectedRow, selectedCol, inputBuffer);
    else
        sheet.deleteCell(selectedRow, selectedCol);
    inputBuffer = "";
}

void GUI::render() {
    window.clear(sf::Color(245, 245, 245));
    renderGrid();
    renderRightPanel();
    renderStatusBar();
    window.display();
}

void GUI::renderGrid() {
    for (int c = 0; c < VIS_COLS; c++) {
        int x = HDR_W + c * CELL_W;
        bool isSelCol = (scrollCol + c == selectedCol);
        drawRect(x, 0, CELL_W - 1, HDR_H - 1,
                 isSelCol ? sf::Color(180, 200, 240) : sf::Color(210, 210, 225));
        drawText(colLabel(scrollCol + c), x + CELL_W/2 - 6, 6, 13);
    }

    for (int r = 0; r < VIS_ROWS; r++) {
        int y = HDR_H + r * CELL_H;
        int actualRow = scrollRow + r;
        bool isSelRow = (actualRow == selectedRow);

        drawRect(0, y, HDR_W - 1, CELL_H - 1,
                 isSelRow ? sf::Color(180, 200, 240) : sf::Color(210, 210, 225));
        drawText(to_string(actualRow + 1), 4, y + 7, 11);

        for (int c = 0; c < VIS_COLS; c++) {
            int x = HDR_W + c * CELL_W;
            int actualCol = scrollCol + c;
            int r1 = min(anchorRow, selectedRow);
            int r2 = max(anchorRow, selectedRow);
            int c1 = min(anchorCol, selectedCol);
            int c2 = max(anchorCol, selectedCol);
            bool isSel = (actualRow >= r1 && actualRow <= r2 && actualCol >= c1 && actualCol <= c2);
            bool hasVal = !sheet.queryCell(actualRow, actualCol).empty();

            sf::Color fillColor = isSel    ? sf::Color(200, 225, 255)
                                : hasVal   ? sf::Color(255, 255, 255)
                                           : sf::Color(252, 252, 252);
            sf::Color outColor  = isSel    ? sf::Color(50, 120, 220)
                                           : sf::Color(200, 200, 200);
            float outW = isSel ? 1.5f : 0.5f;

            drawRect(x, y, CELL_W - 1, CELL_H - 1, fillColor, outColor, outW);

            string display = (isSel && editing) ? (inputBuffer + "_")
                                                     : getCellDisplay(actualRow, actualCol);
            if (!display.empty()) {
                while (display.size() > 11 && !display.empty()) display.pop_back();
                sf::Color textColor = (display == "#ERROR")
                                    ? sf::Color(200, 0, 0)
                                    : sf::Color(20, 20, 20);
                drawText(display, x + 3, y + 7, 12, textColor);
            }
        }
    }
}

void GUI::renderRightPanel() {
    float px = PANEL_X;
    float py = 10;
    float pw = 200;

    drawText("Operaciones", px, py, 14, sf::Color(50, 50, 100));
    py += 24;

    string addr = colLabel(selectedCol) + to_string(selectedRow + 1);
    string rawVal = sheet.queryCell(selectedRow, selectedCol);
    string displayVal = getCellDisplay(selectedRow, selectedCol);

    drawText("Celda: " + addr, px, py, 12); py += 18;
    drawText("Raw: " + (rawVal.size()>16 ? rawVal.substr(0,16)+"..." : rawVal), px, py, 11, sf::Color(80,80,80)); py += 16;
    if (rawVal != displayVal)
        drawText("= " + displayVal, px, py, 11, sf::Color(0,100,0)); py += 16;

    py += 8;
    drawRect(px, py, pw, 0.5f, sf::Color(180,180,180)); py += 8;

    double sr = Operations::sumRow(sheet, selectedRow);
    double ar = Operations::avgRow(sheet, selectedRow);
    ostringstream oss;
    oss << fixed << setprecision(2);
    oss << sr; drawText("SUMA fila " + to_string(selectedRow+1) + ": " + oss.str(), px, py, 11); py += 16;
    oss.str(""); oss << ar;
    drawText("PROM fila: " + oss.str(), px, py, 11); py += 16;

    double sc = Operations::sumCol(sheet, selectedCol);
    double ac = Operations::avgCol(sheet, selectedCol);
    oss.str(""); oss << sc;
    drawText("SUMA col " + colLabel(selectedCol) + ": " + oss.str(), px, py, 11); py += 16;
    oss.str(""); oss << ac;
    drawText("PROM col: " + oss.str(), px, py, 11); py += 16;

    // --- Métricas de rango ---
    int r1 = min(anchorRow, selectedRow);
    int r2 = max(anchorRow, selectedRow);
    int c1 = min(anchorCol, selectedCol);
    int c2 = max(anchorCol, selectedCol);

    double sRng = Operations::sumRange(sheet, r1, c1, r2, c2);
    double aRng = Operations::avgRange(sheet, r1, c1, r2, c2);
    auto maxRng = Operations::maxRange(sheet, r1, c1, r2, c2); 
    auto minRng = Operations::minRange(sheet, r1, c1, r2, c2); 
    
    py += 6;
    drawRect(px, py, pw, 0.5f, sf::Color(180,180,180)); py += 10;
    
    oss.str(""); oss << sRng;
    drawText("SUMA rango: " + oss.str(), px, py, 11, sf::Color(20, 100, 20)); py += 16;
    oss.str(""); oss << aRng;
    drawText("PROM rango: " + oss.str(), px, py, 11, sf::Color(20, 100, 20)); py += 16;

    if (maxRng.has_value()) {
        oss.str(""); oss << *maxRng;
        drawText("MAX rango: " + oss.str(), px, py, 11, sf::Color(20, 100, 20)); py += 16;
    } else {
        drawText("MAX rango: N/A", px, py, 11, sf::Color(100, 100, 100)); py += 16;
    }

    if (minRng.has_value()) {
        oss.str(""); oss << *minRng;
        drawText("MIN rango: " + oss.str(), px, py, 11, sf::Color(20, 100, 20)); py += 16;
    } else {
        drawText("MIN rango: N/A", px, py, 11, sf::Color(100, 100, 100)); py += 16;
    }

    py += 6;
    drawRect(px, py, pw, 0.5f, sf::Color(180,180,180)); py += 10;

    drawText("Controles:", px, py, 12, sf::Color(50,50,100)); py += 18;
    vector<pair<string,string>> controls = {
        {"Flechas",  "Navegar"},
        {"Arrastrar", "Selec. Rango"},
        {"Enter/F2", "Editar celda"},
        {"Esc",      "Cancelar edición"},
        {"Delete",   "Borrar celda/rango"},
        {"Ctrl+D",   "Borrar fila"},
        {"Ctrl+K",   "Borrar columna"},
        {"=A1+B1",   "Fórmula en celda"}
    };
    for (auto& [key, desc] : controls) {
        drawText(key + ": " + desc, px, py, 10, sf::Color(60,60,60));
        py += 15;
    }
} 

void GUI::renderStatusBar() {
    int barY = HDR_H + VIS_ROWS * CELL_H + 8;
    string status;
    if (!statusMsg.empty()) {
        status = statusMsg;
    } else if (editing) {
        status = "Editando " + colLabel(selectedCol) + to_string(selectedRow+1)
               + ": " + inputBuffer + "_  [Enter=confirmar Esc=cancelar]";
    } else {
        status = "Navegando — " + colLabel(selectedCol) + to_string(selectedRow+1);
    }
    drawText(status, 5, barY, 11, sf::Color(60,60,60));
}