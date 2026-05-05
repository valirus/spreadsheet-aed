# Spreadsheet CS2023 AED - Matriz Dispersa Ortogonal

Este proyecto consiste en una hoja de cálculo funcional desarrollada en **C++17**, utilizando una **Matriz Dispersa Ortogonal**. La estructura está implementada manualmente mediante nodos y punteros propios, cumpliendo con los requisitos de la asignatura de **Algoritmos y Estructuras de Datos**. La interfaz gráfica utiliza la biblioteca **SFML** para permitir una interacción fluida.

---

## 🛠️ Requisitos e Instalación

Para compilar y ejecutar este proyecto en Linux o WSL, asegúrese de tener instaladas las dependencias necesarias:

```bash
sudo apt update
sudo apt install g++ make libsfml-dev
```

---

## ⚙️ Compilación y Ejecución

**Compilar el proyecto:**
```bash
make
```

**Ejecutar la aplicación:**
```bash
./spreadsheet
```

**Limpiar archivos temporales:**
```bash
make clean
```

---

## ⌨️ Leyenda de Controles

| Acción | Control |
|---|---|
| Navegación | Flechas del teclado o Clic izquierdo en celda |
| Selección de Rango | Mantener Clic Izquierdo y arrastrar mouse |
| Editar Celda | `Enter` o `F2` sobre la celda |
| Confirmar Cambios | `Enter` después de escribir |
| Cancelar Edición | `Esc` durante la escritura |
| Borrar Celda/Rango | `Delete` (Suprimir) |
| Eliminar Fila | `Ctrl + D` |
| Eliminar Columna | `Ctrl + K` |

> [!IMPORTANT]
> Toda fórmula debe iniciar con el signo `=`. Soporta operaciones básicas (`+`, `-`, `*`, `/`) y referencias (ej. `=A1+5`). El sistema detecta automáticamente divisiones por cero y referencias circulares, mostrando `#ERROR`.

---

## 📚 Detalles de Implementación

**Estructura Principal:** Matriz ortogonal basada en `RowHeader` y `ColHeader` con punteros `nextInRow` y `nextInCol`. Solo se asigna memoria a las celdas que contienen datos, optimizando el uso de recursos.

**Gestión de Memoria:** Se aplica la *Rule of Five* bloqueando copias superficiales (`= delete`) para evitar fugas de memoria y garantizar la integridad del puntero al heap durante la destrucción de objetos.

**Complejidad Algorítmica:**
- **Espacio:** $O(K)$, donde $K$ es el número de celdas con datos.
- **Tiempo:** Inserción y búsqueda en $O(R + C + N)$, siendo $R$ y $C$ la cantidad de cabeceras y $N$ los nodos existentes en la línea de búsqueda.

**Análisis en Tiempo Real:** El panel lateral derecho calcula dinámicamente **Suma**, **Promedio**, **Máximo** y **Mínimo** del rango de celdas seleccionado actualmente.
