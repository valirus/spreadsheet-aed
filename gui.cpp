#include "gui.h"
#include <sstream>
#include <iomanip>
#include <vector>

GUI::GUI(SparseMatrix& sm)
    : sheet(sm), selectedRow(0), selectedCol(0),
      scrollRow(0), scrollCol(0), editing(false) {

    // Ancho: cabeceras + celdas + panel lateral + margen
    unsigned winW = HDR_W + VIS_COLS * CELL_W + 220;
    unsigned winH = HDR_H + VIS_ROWS * CELL_H + 60;
    window.create(sf::VideoMode(winW, winH), "Spreadsheet — CS2023 AED");
    window.setFramerateLimit(60);

    if (!tryLoadFont()) {
        // Si no hay fuente, la app corre pero sin texto renderizado
    }
}

bool GUI::tryLoadFont() {
    std::vector<std::string> paths = {
        "assets/arial.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
        "/usr/share/fonts/truetype/freefont/FreeMono.ttf",
        "/usr/share/fonts/truetype/ubuntu/UbuntuMono-R.ttf",
        "/usr/share/fonts/TTF/DejaVuSansMono.ttf",       // Arch
        "C:/Windows/Fonts/consola.ttf",                   // Windows nativo
        "C:/Windows/Fonts/cour.ttf"
    };
    for (auto& p : paths)
        if (font.loadFromFile(p)) return true;
    return false;
}

// Convierte índice 0-based a letra(s): 0→"A", 1→"B", 25→"Z", 26→"AA"
std::string GUI::colLabel(int col) const {
    std::string label;
    col++;
    while (col > 0) {
        col--;
        label = char('A' + col % 26) + label;
        col /= 26;
    }
    return label;
}

// Devuelve el valor a mostrar: si es fórmula la evalúa, si no devuelve raw.
std::string GUI::getCellDisplay(int row, int col) const {
    std::string val = sheet.queryCell(row, col);
    if (!val.empty() && val[0] == '=')
        return Operations::evaluateFormula(sheet, val);
    return val;
}

void GUI::drawText(const std::string& s, float x, float y, int size, sf::Color color) {
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
                // Navegación con flechas
                if (ev.key.code == sf::Keyboard::Up    && selectedRow > 0) { selectedRow--; statusMsg = ""; }
                if (ev.key.code == sf::Keyboard::Down)                      { selectedRow++; statusMsg = ""; }
                if (ev.key.code == sf::Keyboard::Left  && selectedCol > 0) { selectedCol--; statusMsg = ""; }
                if (ev.key.code == sf::Keyboard::Right)                     { selectedCol++; statusMsg = ""; }

                // Entrar en modo edición
                if (ev.key.code == sf::Keyboard::Enter || ev.key.code == sf::Keyboard::F2) {
                    editing = true;
                    inputBuffer = sheet.queryCell(selectedRow, selectedCol);
                }
                // Suprimir celda
                if (ev.key.code == sf::Keyboard::Delete) {
                    sheet.deleteCell(selectedRow, selectedCol);
                    statusMsg = "Celda eliminada";
                }
                // Ctrl+D = eliminar fila
                if (ev.key.code == sf::Keyboard::D && ev.key.control) {
                    sheet.deleteRow(selectedRow);
                    statusMsg = "Fila " + std::to_string(selectedRow + 1) + " eliminada";
                }
                // Ctrl+K = eliminar columna
                if (ev.key.code == sf::Keyboard::K && ev.key.control) {
                    sheet.deleteCol(selectedCol);
                    statusMsg = "Col " + colLabel(selectedCol) + " eliminada";
                }
                // Escape limpia status
                if (ev.key.code == sf::Keyboard::Escape) statusMsg = "";

            } else {
                // Modo edición
                if (ev.key.code == sf::Keyboard::Enter) {
                    commitInput();
                    editing = false;
                    selectedRow++; // avanzar fila al confirmar
                }
                if (ev.key.code == sf::Keyboard::Escape) {
                    editing = false;
                    inputBuffer = "";
                }
                if (ev.key.code == sf::Keyboard::BackSpace && !inputBuffer.empty())
                    inputBuffer.pop_back();
            }

            // Scroll para mantener la celda visible
            if (selectedRow < scrollRow) scrollRow = selectedRow;
            if (selectedRow >= scrollRow + VIS_ROWS) scrollRow = selectedRow - VIS_ROWS + 1;
            if (selectedCol < scrollCol) scrollCol = selectedCol;
            if (selectedCol >= scrollCol + VIS_COLS) scrollCol = selectedCol - VIS_COLS + 1;
        }

        // --- Texto ingresado (Unicode) ---
        if (ev.type == sf::Event::TextEntered && editing) {
            char c = static_cast<char>(ev.text.unicode);
            if (c >= 32 && c < 127) inputBuffer += c; // solo ASCII imprimible
        }

        // --- Click de mouse ---
        if (ev.type == sf::Event::MouseButtonPressed) {
            int mx = ev.mouseButton.x;
            int my = ev.mouseButton.y;
            if (mx > HDR_W && my > HDR_H && my < HDR_H + VIS_ROWS * CELL_H) {
                int clickCol = (mx - HDR_W) / CELL_W + scrollCol;
                int clickRow = (my - HDR_H) / CELL_H + scrollRow;
                if (editing) commitInput();
                editing = false;
                selectedRow = clickRow;
                selectedCol = clickCol;
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
    // Cabeceras de columna (letras)
    for (int c = 0; c < VIS_COLS; c++) {
        int x = HDR_W + c * CELL_W;
        bool isSelCol = (scrollCol + c == selectedCol);
        drawRect(x, 0, CELL_W - 1, HDR_H - 1,
                 isSelCol ? sf::Color(180, 200, 240) : sf::Color(210, 210, 225));
        drawText(colLabel(scrollCol + c), x + CELL_W/2 - 6, 6, 13);
    }

    // Filas
    for (int r = 0; r < VIS_ROWS; r++) {
        int y = HDR_H + r * CELL_H;
        int actualRow = scrollRow + r;
        bool isSelRow = (actualRow == selectedRow);

        // Cabecera de fila (número)
        drawRect(0, y, HDR_W - 1, CELL_H - 1,
                 isSelRow ? sf::Color(180, 200, 240) : sf::Color(210, 210, 225));
        drawText(std::to_string(actualRow + 1), 4, y + 7, 11);

        for (int c = 0; c < VIS_COLS; c++) {
            int x = HDR_W + c * CELL_W;
            int actualCol = scrollCol + c;
            bool isSel = (actualRow == selectedRow && actualCol == selectedCol);
            bool hasVal = !sheet.queryCell(actualRow, actualCol).empty();

            sf::Color fillColor = isSel    ? sf::Color(200, 225, 255)
                                : hasVal   ? sf::Color(255, 255, 255)
                                           : sf::Color(252, 252, 252);
            sf::Color outColor  = isSel    ? sf::Color(50, 120, 220)
                                           : sf::Color(200, 200, 200);
            float outW = isSel ? 1.5f : 0.5f;

            drawRect(x, y, CELL_W - 1, CELL_H - 1, fillColor, outColor, outW);

            // Contenido: si estamos editando esta celda, muestra buffer + cursor
            std::string display = (isSel && editing) ? (inputBuffer + "_")
                                                     : getCellDisplay(actualRow, actualCol);
            if (!display.empty()) {
                // Truncar si es muy largo para el ancho de la celda
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

    // Título
    drawText("Operaciones", px, py, 14, sf::Color(50, 50, 100));
    py += 24;

    // Celda seleccionada y valor
    std::string addr = colLabel(selectedCol) + std::to_string(selectedRow + 1);
    std::string rawVal = sheet.queryCell(selectedRow, selectedCol);
    std::string displayVal = getCellDisplay(selectedRow, selectedCol);

    drawText("Celda: " + addr, px, py, 12); py += 18;
    drawText("Raw: " + (rawVal.size()>16 ? rawVal.substr(0,16)+"..." : rawVal), px, py, 11, sf::Color(80,80,80)); py += 16;
    if (rawVal != displayVal)
        drawText("= " + displayVal, px, py, 11, sf::Color(0,100,0)); py += 16;

    py += 8;
    drawRect(px, py, pw, 0.5f, sf::Color(180,180,180)); py += 8;

    // Suma y promedio de la fila seleccionada
    double sr = Operations::sumRow(sheet, selectedRow);
    double ar = Operations::avgRow(sheet, selectedRow);
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << sr; drawText("SUMA fila " + std::to_string(selectedRow+1) + ": " + oss.str(), px, py, 11); py += 16;
    oss.str(""); oss << ar;
    drawText("PROM fila: " + oss.str(), px, py, 11); py += 16;

    // Suma y promedio de la columna seleccionada
    double sc = Operations::sumCol(sheet, selectedCol);
    double ac = Operations::avgCol(sheet, selectedCol);
    oss.str(""); oss << sc;
    drawText("SUMA col " + colLabel(selectedCol) + ": " + oss.str(), px, py, 11); py += 16;
    oss.str(""); oss << ac;
    drawText("PROM col: " + oss.str(), px, py, 11); py += 16;

    py += 6;
    drawRect(px, py, pw, 0.5f, sf::Color(180,180,180)); py += 10;

    // Instrucciones
    drawText("Controles:", px, py, 12, sf::Color(50,50,100)); py += 18;
    std::vector<std::pair<std::string,std::string>> controls = {
        {"Flechas",  "Navegar"},
        {"Enter/F2", "Editar celda"},
        {"Esc",      "Cancelar edición"},
        {"Delete",   "Borrar celda"},
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
    std::string status;
    if (!statusMsg.empty()) {
        status = statusMsg;
    } else if (editing) {
        status = "Editando " + colLabel(selectedCol) + std::to_string(selectedRow+1)
               + ": " + inputBuffer + "_  [Enter=confirmar Esc=cancelar]";
    } else {
        status = "Navegando — " + colLabel(selectedCol) + std::to_string(selectedRow+1);
    }
    drawText(status, 5, barY, 11, sf::Color(60,60,60));
}