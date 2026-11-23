# Informe - Tarea 2, INF-221

Matías Jorquera · 202310127-3 · Casa Central · 2026-2.

El informe analiza grid pathfinding con fuerza bruta, DP y greedy. Contiene
la recurrencia y justificación de DP, complejidades, metodología, tablas,
gráficos y discusión de tiempo, memoria, brecha y fallas por tamaño, tipo y dominio.

## Archivos

- `author.tex`: autor y datos de la tarea.
- `sections/introduccion.tex`: objetivo del estudio.
- `sections/experiment_intro.tex`: dataset y mediciones.
- `sections/experiment_results.tex`: análisis teórico y experimental.
- `sections/generated_cost_table.tex`, `sections/generated_quality_table.tex`: tablas generadas del CSV.
- `sections/conclusions.tex`: conclusiones respaldadas por los resultados.
- `references.bib`: libro de Roughgarden y documentación de memoria.
- `report.pdf`: informe compilado.

## Regenerar datos y figuras

Desde `code/grid_pathfinding/`, ejecutar:

```bash
python scripts/run_measurements.py
python scripts/plot_generator.py
python scripts/generate_report_tables.py
```

Los PNG se leen directamente desde `code/grid_pathfinding/data/plots/`.
Las tablas se generan desde `data/measurements/results.csv`.

## Compilar

```bash
pdflatex -interaction=nonstopmode -halt-on-error report.tex
biber report
pdflatex -interaction=nonstopmode -halt-on-error report.tex
pdflatex -interaction=nonstopmode -halt-on-error report.tex
```

También puede compilarse en Overleaf con el proyecto completo.
