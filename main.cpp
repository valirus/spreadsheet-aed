#include "gui.h"
#include <iostream>

int main() {
    SparseMatrix sheet;

    // Datos de demo precargados para la presentación
    sheet.insertCell(0, 0, "Producto");
    sheet.insertCell(0, 1, "Precio");
    sheet.insertCell(0, 2, "Stock");
    sheet.insertCell(1, 0, "Laptop");
    sheet.insertCell(1, 1, "2500");
    sheet.insertCell(1, 2, "10");
    sheet.insertCell(2, 0, "Mouse");
    sheet.insertCell(2, 1, "45");
    sheet.insertCell(2, 2, "100");
    sheet.insertCell(3, 0, "Teclado");
    sheet.insertCell(3, 1, "120");
    sheet.insertCell(3, 2, "50");
    // Celda con fórmula
    sheet.insertCell(4, 1, "=B2+B3+B4");  // suma de precios
    sheet.insertCell(4, 0, "TOTAL");

    GUI gui(sheet);
    gui.run();
    return 0;
}