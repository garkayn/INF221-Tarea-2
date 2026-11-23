// Mide un algoritmo de grid pathfinding para una grilla de entrada.
// Auxiliar para medir cada algoritmo en un proceso independiente.
// Fuente: enunciado, apartado de mediciones. Se cronometra solo el solver;
// la memoria es el máximo residente del proceso, incluyendo entrada y runtime.
// PSAPI: https://learn.microsoft.com/en-us/windows/win32/api/psapi/ns-psapi-process_memory_counters
// Matías Jorquera, rol 202310127-3, INF-221, Tarea 2, 2026-2, Casa Central.

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#else
#include <sys/resource.h>
#endif

struct PathResult {
    bool found;
    long long score;
    std::string path;
    int remaining_lives;
};

PathResult solveBruteForce(int n, int m, int v, int p,
                           const std::vector<std::vector<int>>& types,
                           const std::vector<std::vector<long long>>& values);
PathResult solveDP(int n, int m, int v, int p,
                   const std::vector<std::vector<int>>& types,
                   const std::vector<std::vector<long long>>& values);
PathResult solveGreedy(int n, int m, int v, int p,
                       const std::vector<std::vector<int>>& types,
                       const std::vector<std::vector<long long>>& values);

long long peakMemoryBytes() {
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS counters{};
    counters.cb = sizeof(counters);
    if (GetProcessMemoryInfo(GetCurrentProcess(), &counters, sizeof(counters))) {
        return static_cast<long long>(counters.PeakWorkingSetSize);
    }
#else
    rusage usage{};
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
#ifdef __APPLE__
        return static_cast<long long>(usage.ru_maxrss);
#else
        return static_cast<long long>(usage.ru_maxrss) * 1024;
#endif
    }
#endif
    return -1;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Uso: benchmark_runner <archivo_grilla> <bruteforce|dp|greedy>\n";
        return 1;
    }

    std::ifstream input(argv[1]);
    int n, m, v, p;
    if (!(input >> n >> m >> v >> p)) {
        std::cerr << "No se pudo leer la cabecera de la grilla.\n";
        return 1;
    }

    std::vector<std::vector<int>> types(n, std::vector<int>(m));
    std::vector<std::vector<long long>> values(n, std::vector<long long>(m));
    for (int row = 0; row < n; ++row) {
        for (int col = 0; col < m; ++col) {
            if (!(input >> types[row][col])) {
                std::cerr << "No se pudo leer la matriz de tipos.\n";
                return 1;
            }
        }
    }
    for (int row = 0; row < n; ++row) {
        for (int col = 0; col < m; ++col) {
            if (!(input >> values[row][col])) {
                std::cerr << "No se pudo leer la matriz de valores.\n";
                return 1;
            }
        }
    }

    const std::string algorithm = argv[2];
    using Solver = PathResult (*)(int, int, int, int,
        const std::vector<std::vector<int>>&,
        const std::vector<std::vector<long long>>&);
    Solver solve;
    if (algorithm == "bruteforce") {
        if (n + m > 32) {
            std::cerr << "Fuerza bruta requiere N+M <= 32.\n";
            return 1;
        }
        solve = solveBruteForce;
    } else if (algorithm == "dp") {
        solve = solveDP;
    } else if (algorithm == "greedy") {
        solve = solveGreedy;
    } else {
        std::cerr << "Algoritmo no valido: " << algorithm << "\n";
        return 1;
    }
    const int repetitions = (algorithm == "bruteforce" && n + m == 32) ? 1 : 3;
    std::vector<double> times;
    PathResult result{false, 0, "", 0};
    for (int trial = 0; trial < repetitions; ++trial) {
        const auto start = std::chrono::steady_clock::now();
        result = solve(n, m, v, p, types, values);
        const auto end = std::chrono::steady_clock::now();
        times.push_back(std::chrono::duration<double, std::micro>(end - start).count());
    }
    std::sort(times.begin(), times.end());
    const double elapsedUs = times[times.size() / 2];

    std::cout << (result.found ? 1 : 0) << ',' << result.score << ','
              << result.remaining_lives << ',' << elapsedUs << ','
              << peakMemoryBytes() << ',' << repetitions << '\n'
              << result.path << '\n';
    return 0;
}
