"""
Gráficos de la Tarea 2 a partir de data/measurements/results.csv.

Los tiempos son medianas de cada ejecución, promediadas entre instancias.
La brecha excluye las fallas; la tasa de falla incluye todas las instancias de
greedy de cada grupo.
"""

from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import pandas as pd

BASE_DIR = Path(__file__).resolve().parent.parent
MEASUREMENTS_DIR = BASE_DIR / "data" / "measurements"
PLOTS_DIR = BASE_DIR / "data" / "plots"
TYPES = ["libre", "murallas", "minas", "mixta"]
ALGORITHMS = ["bruteforce", "dp", "greedy"]
LABELS = {"bruteforce": "Fuerza bruta", "dp": "DP", "greedy": "Greedy"}
COLORS = ["#3163a4", "#23815b", "#d06b2a", "#8056a0"]


def save(fig, name):
    fig.tight_layout()
    fig.savefig(PLOTS_DIR / name, dpi=180, bbox_inches="tight")
    plt.close(fig)


def cost_panel(ax, data, metric, title):
    for algorithm, color in zip(ALGORITHMS, COLORS):
        selected = data[data.algorithm == algorithm]
        means = selected.groupby("cells")[metric].mean()
        ax.plot(means.index, means, "o-", color=color,
                label=LABELS[algorithm], markersize=4)
    ax.set_xscale("log", base=2)
    ax.set_yscale("log")
    ax.set_title(title)
    ax.set_xlabel("Tamaño N × M (celdas)")
    ax.set_ylabel("Tiempo (ms)" if metric == "time_ms" else "máximo residente (MiB)")
    ax.grid(True, which="major", alpha=0.25)
    ax.legend(fontsize=8)


def quality_panel(ax, data, domain, metric):
    for grid_type, color in zip(TYPES, COLORS):
        selected = data[(data.domain == domain) & (data.type == grid_type)]
        # Las fallas tienen gap vacío: mean las excluye y deja NaN si no hubo
        # ningún éxito, interrumpiendo la curva en ese tamaño.
        means = selected.groupby("cells")[metric].mean()
        ax.plot(means.index, means, "o-", color=color,
                label=grid_type.capitalize(), markersize=4)
    ax.set_xscale("log", base=2)
    ax.set_title(domain)
    ax.set_xlabel("Tamaño N × M (celdas)")
    ax.set_ylabel("Brecha media Δ (solo éxitos)" if metric == "gap" else "Fallas (%)")
    if metric == "failure":
        ax.set_ylim(-3, 103)
    ax.grid(True, alpha=0.25)
    ax.legend(fontsize=8)


def main():
    PLOTS_DIR.mkdir(parents=True, exist_ok=True)
    data = pd.read_csv(MEASUREMENTS_DIR / "results.csv")
    data["cells"] = data.n * data.m
    data["time_ms"] = data.time_us / 1000
    data["memory_mib"] = data.peak_memory_bytes / (1024 * 1024)
    plt.rcParams.update({"font.size": 10, "axes.spines.top": False,
                         "axes.spines.right": False})

    for metric, filename in [("time_ms", "execution_time.png"),
                             ("memory_mib", "memory_usage.png")]:
        fig, axes = plt.subplots(2, 2, figsize=(10, 7))
        for ax, grid_type in zip(axes.flat, TYPES):
            cost_panel(ax, data[data.type == grid_type], metric, grid_type.capitalize())
        save(fig, filename)

    fig, axes = plt.subplots(1, 2, figsize=(10, 3.6))
    cost_panel(axes[0], data, "time_ms", "Tiempo: media entre tipos y dominios")
    cost_panel(axes[1], data, "memory_mib", "Memoria total del proceso")
    save(fig, "cost_comparison.png")

    greedy = data[data.algorithm == "greedy"].copy()
    greedy["failure"] = 100 * (1 - greedy.found)
    for metric, filename in [("gap", "score_gap.png"),
                             ("failure", "greedy_failure_rate.png")]:
        fig, axes = plt.subplots(1, 2, figsize=(10, 3.6))
        for ax, domain in zip(axes, ["D10", "D1000"]):
            quality_panel(ax, greedy, domain, metric)
        save(fig, filename)

    fig, axes = plt.subplots(2, 2, figsize=(10, 6.4))
    for col, domain in enumerate(["D10", "D1000"]):
        quality_panel(axes[0, col], greedy, domain, "gap")
        quality_panel(axes[1, col], greedy, domain, "failure")
    save(fig, "greedy_quality.png")

    data.groupby(["n", "m", "algorithm"])[["time_ms", "memory_mib"]].mean().to_csv(
        MEASUREMENTS_DIR / "cost_by_size.csv")
    greedy.groupby(["n", "m", "type", "domain"]).agg(
        cases=("found", "size"), successes=("found", "sum"),
        failure_percent=("failure", "mean"), mean_gap=("gap", "mean"),
    ).to_csv(MEASUREMENTS_DIR / "quality_by_size.csv")
    print(f"Se generaron seis gráficos en {PLOTS_DIR}")


if __name__ == "__main__":
    main()
