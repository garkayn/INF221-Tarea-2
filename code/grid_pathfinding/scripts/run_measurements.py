"""Mediciones de la Tarea 2; conserva el programa principal y sus salidas.

Fuente: enunciado, Implementaciones y Casos de prueba. Cada algoritmo se mide
en un proceso separado. Se comprueba la coincidencia BF/DP y cada trayectoria.
"""

import csv
import ctypes
from datetime import datetime, timezone
import hashlib
import importlib.metadata
import os
import platform
import subprocess
import sys
import shutil

import numpy as np

BASE_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
INPUT_DIR = os.path.join(BASE_DIR, "data", "grid_input")
OUTPUT_DIR = os.path.join(BASE_DIR, "data", "measurements")
RUNNER_SOURCE = os.path.join(os.path.dirname(__file__), "benchmark_runner.cpp")
ALGORITHMS = ("bruteforce", "dp", "greedy")


def build_runner(output_path):
    sources = [
        RUNNER_SOURCE,
        os.path.join(BASE_DIR, "algorithms", "brute_force.cpp"),
        os.path.join(BASE_DIR, "algorithms", "dynamic_programming.cpp"),
        os.path.join(BASE_DIR, "algorithms", "greedy.cpp"),
    ]
    compiler_paths = [os.path.relpath(path, BASE_DIR).replace("\\", "/")
                      for path in sources]
    compiler_output = os.path.relpath(output_path, BASE_DIR).replace("\\", "/")
    command = ["g++", "-std=c++17", "-O2", *compiler_paths,
               "-o", compiler_output]
    if sys.platform == "win32":
        command.append("-lpsapi")
        # GCC de MSYS2 necesita encontrar sus DLL al lanzar cc1plus y el runner.
        compiler = shutil.which("g++")
        if compiler:
            os.environ["PATH"] = os.path.dirname(compiler) + os.pathsep + os.environ["PATH"]
    subprocess.run(command, check=True, cwd=BASE_DIR)


def parse_case(filename):
    n, m, grid_type, domain, sample = filename[:-4].split("_")
    return int(n), int(m), grid_type, domain, sample


def validate_path(path, score, lives, n, m, v, p, types, values):
    if len(path) != n + m - 2:
        raise ValueError("Longitud incorrecta del camino")
    row = col = 0
    actual_score = int(values[0, 0])
    actual_lives = v
    for move in path:
        if move == "R":
            col += 1
        elif move == "D":
            row += 1
        else:
            raise ValueError("Movimiento inválido")
        if row >= n or col >= m or types[row, col] == 0:
            raise ValueError("Camino fuera de la grilla o sobre una muralla")
        if types[row, col] == -1:
            actual_lives -= 1
            actual_score -= p
        else:
            actual_score += int(values[row, col])
        if actual_lives <= 0:
            raise ValueError("El camino agota las vidas")
    if (row, col, actual_score, actual_lives) != (n - 1, m - 1, score, lives):
        raise ValueError("Destino, puntaje o vidas incorrectos")


def physical_memory_gib():
    if sys.platform == "win32":
        class MemoryStatus(ctypes.Structure):
            _fields_ = [
                ("dwLength", ctypes.c_ulong), ("dwMemoryLoad", ctypes.c_ulong),
                ("ullTotalPhys", ctypes.c_ulonglong),
                ("ullAvailPhys", ctypes.c_ulonglong),
                ("ullTotalPageFile", ctypes.c_ulonglong),
                ("ullAvailPageFile", ctypes.c_ulonglong),
                ("ullTotalVirtual", ctypes.c_ulonglong),
                ("ullAvailVirtual", ctypes.c_ulonglong),
                ("ullAvailExtendedVirtual", ctypes.c_ulonglong),
            ]

        status = MemoryStatus()
        status.dwLength = ctypes.sizeof(status)
        if ctypes.windll.kernel32.GlobalMemoryStatusEx(ctypes.byref(status)):
            return round(status.ullTotalPhys / (1024 ** 3), 1)
    elif hasattr(os, "sysconf"):
        try:
            return round(os.sysconf("SC_PAGE_SIZE") * os.sysconf("SC_PHYS_PAGES") /
                         (1024 ** 3), 1)
        except (ValueError, OSError):
            pass
    return None


def input_fingerprint(filenames):
    digest = hashlib.sha256()
    for filename in filenames:
        digest.update(filename.encode("utf-8"))
        with open(os.path.join(INPUT_DIR, filename), "rb") as source:
            for chunk in iter(lambda: source.read(1024 * 1024), b""):
                digest.update(chunk)
    return digest.hexdigest()


def file_sha256(path):
    digest = hashlib.sha256()
    with open(path, "rb") as source:
        for chunk in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def write_metadata(input_files, records):
    fingerprint = input_fingerprint(input_files)
    results_path = os.path.join(OUTPUT_DIR, "results.csv")
    results_fingerprint = file_sha256(results_path)
    results_modified = datetime.fromtimestamp(
        os.path.getmtime(results_path), timezone.utc
    ).isoformat(timespec="seconds")
    cases = {tuple(row[key] for key in ("n", "m", "type", "domain", "sample"))
             for row in records}
    greedy = [row for row in records if row["algorithm"] == "greedy"]
    validation = {
        "instances": len(cases),
        "measurements": len(records),
        "inputs_sha256": fingerprint,
        "results_sha256": results_fingerprint,
        "measurement_run_complete": True,
        "bf_dp_comparisons": sum(row["exact_match"] == "1" for row in records),
        "successful_paths_validated": sum(row["path_valid"] == "1" for row in records),
        "greedy_failures": sum(row["found"] == "0" for row in greedy),
        "greedy_optimal_successes": sum(row["gap"] == "0" for row in greedy),
    }
    compiler = subprocess.run(["g++", "--version"], capture_output=True,
                              text=True, check=False).stdout.splitlines()
    environment = {
        "metadata_generated_utc": datetime.now(timezone.utc).isoformat(timespec="seconds"),
        "results_modified_utc": results_modified,
        "results_sha256": results_fingerprint,
        "cpu": platform.processor() or platform.machine(),
        "logical_processors": os.cpu_count(),
        "physical_ram_gib": physical_memory_gib(),
        "os": platform.platform(),
        "compiler": compiler[0] if compiler else "g++ version unavailable",
        "compiler_flags": "-std=c++17 -O2",
        "python": platform.python_version(),
        "numpy": np.__version__,
        "pandas": importlib.metadata.version("pandas"),
        "matplotlib": importlib.metadata.version("matplotlib"),
        "inputs": len(input_files),
        "inputs_sha256": fingerprint,
        "repetitions": 3,
        "bruteforce_16x16_repetitions": 1,
        "time_metric": "median solver time, microseconds",
        "memory_metric": "PeakWorkingSetSize, bytes, whole isolated process",
    }
    metadata_files = (("validation_summary.json", validation),
                      ("environment.json", environment))
    temporary_files = []
    for filename, content in metadata_files:
        path = os.path.join(OUTPUT_DIR, filename)
        temporary_path = path + ".tmp"
        with open(temporary_path, "w", encoding="utf-8") as target:
            import json
            json.dump(content, target, indent=2)
            target.write("\n")
        temporary_files.append((temporary_path, path))
    for temporary_path, path in temporary_files:
        os.replace(temporary_path, path)


def main():
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    runner = os.path.join(OUTPUT_DIR, "benchmark_runner.exe" if sys.platform == "win32"
                         else "benchmark_runner")
    build_runner(runner)

    input_files = sorted(name for name in os.listdir(INPUT_DIR) if name.endswith(".txt"))
    output_path = os.path.join(OUTPUT_DIR, "results.csv")
    temporary_output_path = output_path + ".tmp"
    records = []
    with open(temporary_output_path, "w", newline="", encoding="utf-8") as output:
        writer = csv.writer(output)
        writer.writerow([
            "n", "m", "type", "domain", "sample", "algorithm", "found",
            "score", "remaining_lives", "time_us", "peak_memory_bytes",
            "repetitions", "gap", "path_valid", "exact_match",
        ])

        for filename in input_files:
            n, m, grid_type, domain, sample = parse_case(filename)
            input_path = os.path.join(INPUT_DIR, filename)
            with open(input_path, encoding="utf-8") as source:
                _, _, v, p = map(int, source.readline().split())
                matrices = np.loadtxt(source, dtype=np.int64)
            types, values = matrices[:n], matrices[n:]
            results = {}
            for algorithm in ALGORITHMS:
                if algorithm == "bruteforce" and n + m > 32:
                    continue
                lines = subprocess.run(
                    [runner, input_path, algorithm],
                    check=True,
                    capture_output=True,
                    text=True,
                ).stdout.splitlines()
                found, score, lives, time_us, memory_bytes, repetitions = lines[0].split(",")
                found, score, lives = int(found), int(score), int(lives)
                path = lines[1] if len(lines) > 1 else ""
                if found:
                    validate_path(path, score, lives, n, m, v, p, types, values)
                results[algorithm] = (found, score)
                exact_match = ""
                if algorithm == "dp" and "bruteforce" in results:
                    exact_match = int(results["bruteforce"] == (found, score))
                    if not exact_match:
                        raise ValueError(f"BF y DP difieren en {filename}")
                gap = ""
                if algorithm == "greedy" and found and results["dp"][0]:
                    gap = results["dp"][1] - score
                    if gap < 0:
                        raise ValueError(f"Greedy supera al óptimo en {filename}")
                record = {
                    "n": str(n), "m": str(m), "type": grid_type,
                    "domain": domain, "sample": sample,
                    "algorithm": algorithm, "found": str(found),
                    "exact_match": str(exact_match), "path_valid": str(found),
                    "gap": str(gap),
                }
                records.append(record)
                writer.writerow([
                    n, m, grid_type, domain, sample, algorithm, found,
                    score if found else "", lives if found else "", time_us,
                    memory_bytes, repetitions, gap, found, exact_match,
                ])
            output.flush()
            print(f"Medido: {filename}", flush=True)

    os.replace(temporary_output_path, output_path)
    os.remove(runner)
    write_metadata(input_files, records)
    print(f"Mediciones guardadas en: {output_path}")


if __name__ == "__main__":
    main()
