# Grid pathfinding - Tarea 2

Matías Jorquera, rol 202310127-3, INF-221, 2026-2, Casa Central.

## Requisitos

- GCC con C++17 y `make`.
- Python 3 con NumPy, pandas y Matplotlib: `python -m pip install -r requirements.txt`.

Los siguientes comandos se ejecutan desde `code/grid_pathfinding/`.

## Generar entradas

```bash
python scripts/grid_generator.py --seed 2026
```

## Programa principal

```bash
make
./grid_pathfinding
```

Sin `make`, la compilación equivalente es:

```bash
g++ -std=c++17 -O2 -Wall -Wextra grid_pathfinding.cpp algorithms/brute_force.cpp algorithms/dynamic_programming.cpp algorithms/greedy.cpp -o grid_pathfinding
```

En Windows con MinGW/MSYS2, agrega `-lpsapi` al final para enlazar la medición
del máximo residente.

El programa recorre `data/grid_input/`. No recibe argumentos.
Ejecuta fuerza bruta únicamente si `N + M <= 32`; DP y greedy
se ejecutan para todos los tamaños. También registra medidas en
`data/measurements/main_results.csv`. Las medidas aisladas que alimentan
los gráficos se obtienen con el auxiliar descrito más abajo.


Cada salida `data/grid_output/{caso}_{algoritmo}_out.txt` contiene puntaje,
secuencia R/D y vidas restantes en tres líneas. Si no hay camino, contiene solo
`SIN SOLUCION`. Los nombres de algoritmo son `bruteforce`, `dp` y `greedy`.
Un resultado válido puede tener puntaje negativo.

## Algoritmos y decisiones

| Algoritmo | Decisión | Tiempo del solver | Memoria auxiliar |
|---|---|---|---|
| Fuerza bruta | DFS R antes de D, sin memorización ni cotas de puntaje | O(L · C) | O(N + M) |
| DP | Tabulación por celda y vidas, con predecesores | O(N · M · V) | O(N · M · V) |
| Greedy | Mayor ganancia inmediata; empate hacia R | O(N + M) | O(N + M) |

## Mediciones y comprobaciones

```bash
python scripts/run_measurements.py
```

El programa principal comprueba que BF y DP coincidan en existencia y puntaje
cuando `N + M <= 32`. También escribe `data/measurements/main_results.csv`, con
tiempo de solver y máximo residente observado después de cada llamada. Como los
algoritmos corren secuencialmente en el mismo proceso, ese máximo es acumulativo.

El script compila `scripts/benchmark_runner.cpp` junto con los tres algoritmos,
los mide en procesos separados y escribe `data/measurements/results.csv`.
Este CSV contiene las mediciones aisladas usadas por los gráficos.

Las columnas del CSV son:

| Columnas | Significado |
|---|---|
| `n,m,type,domain,sample,algorithm` | Identificación del caso y algoritmo |
| `found` | 1 si encontró un camino, 0 si falló |
| `score,remaining_lives` | Puntaje y vidas, vacíos si falló |
| `time_us` | Mediana de tiempo del solver en microsegundos |
| `peak_memory_bytes` | máximo residente del proceso en bytes |
| `repetitions` | Número de llamadas cronometradas |
| `gap` | DP menos greedy para éxitos de greedy; vacío en otro caso |
| `path_valid` | 1 para un camino exitoso validado; 0 si no hubo camino |
| `exact_match` | 1 en DP cuando se comprobó coincidencia con BF; vacío si no se comparó |

El script aborta si BF y DP difieren en existencia o puntaje, si una trayectoria
exitosa tiene movimientos, puntaje o vidas incorrectos, o si greedy supera a DP.
Se comprobaron 120 coincidencias exactas y 658 trayectorias en el conjunto.
Al terminar, el script actualiza `environment.json` y `validation_summary.json`
con la fecha de generación, equipo, versiones, huellas de entradas y CSV y los
resultados de esa ejecución. El CSV anterior se conserva si la medición falla
antes de completarse; los hashes permiten confirmar que el resumen corresponde
al archivo generado.

## Gráficos y tablas

```bash
python scripts/plot_generator.py
python scripts/generate_report_tables.py
```

Los seis PNG se guardan en `data/plots/`:

- `execution_time.png`: tiempo por tamaño y tipo; media entre dominios y muestras.
- `memory_usage.png`: memoria total por tamaño y tipo.
- `score_gap.png`: brecha media por tamaño, tipo y dominio, solo entre éxitos.
- `greedy_failure_rate.png`: porcentaje de fallas sobre las tres muestras de cada grupo.
- `cost_comparison.png`: resumen de tiempo y memoria, promediando 24 casos por tamaño.
- `greedy_quality.png`: resumen de brecha y fallas para el informe.

También se generan `cost_by_size.csv`, `quality_by_size.csv` y las tablas LaTeX
`report/sections/generated_cost_table.tex` y `generated_quality_table.tex`.
Las fallas no se cuentan como brecha cero. No se calcula un cociente porcentual
de puntajes, porque pueden ser negativos o cero. En un grupo sin éxitos no se
dibuja una brecha; la curva queda interrumpida.

`make measurements` y `make plots` ejecutan los scripts equivalentes.

## Fuentes

Las reglas y parámetros provienen del enunciado en `assignment_statement/`.
Las decisiones de cada algoritmo están documentadas al inicio de su fuente.
