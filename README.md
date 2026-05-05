# Spreadsheet CS2023 AED - Matriz Dispersa Ortogonal

Este proyecto consiste en una hoja de cálculo funcional desarrollada en **C++17**, que utiliza una **Matriz Dispersa Ortogonal** implementada manualmente mediante nodos y punteros. La interfaz gráfica está construida con la biblioteca **SFML**, permitiendo una interacción fluida con el mouse y teclado.

## 🚀 Características Principales

*   **Estructura Eficiente**: Implementación de matriz dispersa basada en listas doblemente enlazadas (filas y columnas), optimizando el uso de memoria al almacenar solo celdas con datos.
*   **Motor de Fórmulas**: Evaluación recursiva de expresiones matemáticas (`+`, `-`, `*`, `/`) con soporte para referencias de celdas (ej. `=A1+B2`).
*   **Seguridad de Memoria**: Prevención de fugas mediante gestión estricta de destructores y bloqueo de copias superficiales (Rule of Three/Five).
*   **Robustez**: Control de errores para divisiones por cero y detección de referencias circulares para evitar desbordamiento de pila (Stack Overflow).

## 🛠️ Requisitos del Sistema

Para compilar y ejecutar este proyecto en Linux (o WSL), asegúrese de tener instaladas las siguientes dependencias:

```bash
sudo apt update
sudo apt install g++ make libsfml-dev
