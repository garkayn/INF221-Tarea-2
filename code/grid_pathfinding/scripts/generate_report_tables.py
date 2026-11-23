"""Genera las tablas del informe desde el mismo CSV de los gráficos."""

from pathlib import Path
import pandas as pd

BASE_DIR = Path(__file__).resolve().parent.parent
SECTIONS_DIR = BASE_DIR.parent.parent / "report" / "sections"


def main():
    data = pd.read_csv(BASE_DIR / "data" / "measurements" / "results.csv")
    data["time_ms"] = data.time_us / 1000
    data["memory_mib"] = data.peak_memory_bytes / (1024 * 1024)
    costs = data.groupby(["n", "m", "algorithm"])[["time_ms", "memory_mib"]].mean()
    lines = [r"\begin{tabular}{lrrrrr}", r"\hline",
             r"$N\times M$ & BF (ms) & DP (ms) & Greedy (ms) & DP (MiB) & Greedy (MiB) \\",
             r"\hline"]
    for n, m in [(4, 4), (8, 16), (16, 16), (256, 256), (1024, 1024), (1024, 2048)]:
        bf = f"{costs.loc[(n, m, 'bruteforce'), 'time_ms']:.4f}" if (n, m, "bruteforce") in costs.index else "--"
        dp = costs.loc[(n, m, "dp")]
        greedy = costs.loc[(n, m, "greedy")]
        lines.append(f"${n}\\times{m}$ & {bf} & {dp.time_ms:.4f} & "
                     f"{greedy.time_ms:.4f} & {dp.memory_mib:.2f} & {greedy.memory_mib:.2f} " + r"\\")
    lines += [r"\hline", r"\end{tabular}"]
    (SECTIONS_DIR / "generated_cost_table.tex").write_text("\n".join(lines) + "\n", encoding="utf-8")

    greedy = data[data.algorithm == "greedy"]
    lines = [r"\begin{tabular}{llrrr}", r"\hline",
             r"Tipo & Dominio & Fallas / casos & Fallas (\%) & $\overline{\Delta}$ (éxitos) \\", r"\hline"]
    for grid_type in ["libre", "murallas", "minas", "mixta"]:
        for domain in ["D10", "D1000"]:
            group = greedy[(greedy.type == grid_type) & (greedy.domain == domain)]
            failures = int((group.found == 0).sum())
            lines.append(f"{grid_type.capitalize()} & {domain} & {failures}/{len(group)} & "
                         f"{100 * failures / len(group):.1f} & {group.gap.mean():.1f} " + r"\\")
    lines += [r"\hline", r"\end{tabular}"]
    (SECTIONS_DIR / "generated_quality_table.tex").write_text("\n".join(lines) + "\n", encoding="utf-8")
    print("Tablas del informe generadas desde results.csv")


if __name__ == "__main__":
    main()
