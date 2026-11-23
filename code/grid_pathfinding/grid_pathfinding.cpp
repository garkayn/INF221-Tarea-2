// Matías Jorquera, rol 202310127-3, INF-221, Tarea 2, 2026-2, Casa Central.

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <filesystem>
#include <iomanip>
#include <stdexcept>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#else
#include <sys/resource.h>
#endif

#ifndef PATH_RESULT_H
#define PATH_RESULT_H
struct PathResult {
    bool found;
    long long score;
    std::string path;
    int remaining_lives;
};
#endif

namespace fs = std::filesystem;

// Firmas de los algoritmos definidos en algorithms/
PathResult solveBruteForce(int n, int m, int v, int p,
                          const std::vector<std::vector<int>>& types,
                          const std::vector<std::vector<long long>>& values);

PathResult solveDP(int n, int m, int v, int p,
                   const std::vector<std::vector<int>>& types,
                   const std::vector<std::vector<long long>>& values);

PathResult solveGreedy(int n, int m, int v, int p,
                       const std::vector<std::vector<int>>& types,
                       const std::vector<std::vector<long long>>& values);

void write_output_file(const fs::path& output_path, const PathResult& result) {
    if (output_path.has_parent_path()) {
        fs::create_directories(output_path.parent_path());
    }

    std::ofstream out(output_path);
    if (!out.is_open()) {
        throw std::runtime_error("No se pudo abrir archivo de salida: " + output_path.string());
    }

    if (!result.found) {
        out << "SIN SOLUCION\n";
    } else {
        out << result.score << '\n'
            << result.path << '\n'
            << result.remaining_lives << '\n';
    }

    if (!out) {
        throw std::runtime_error("Error escribiendo en archivo de salida: " + output_path.string());
    }
}

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

void write_measurement(std::ofstream& output, const std::string& stem,
                       const std::string& algorithm, const PathResult& result,
                       double elapsedUs, long long peakBytes,
                       const std::string& gap = "") {
    output << stem << ',' << algorithm << ',' << (result.found ? 1 : 0) << ',';
    if (result.found) {
        output << result.score << ',' << result.remaining_lives;
    } else {
        output << ',';
    }
    output << ',' << std::fixed << std::setprecision(3) << elapsedUs
           << ',' << peakBytes << ',' << gap << '\n';
}

int main() {
    const fs::path& path = "data/grid_input";
    fs::create_directories("data/measurements");
    std::ofstream measurements("data/measurements/main_results.csv");
    if (!measurements) {
        throw std::runtime_error("No se pudo abrir data/measurements/main_results.csv");
    }
    measurements << "case,algorithm,found,score,remaining_lives,time_us,"
                    "process_peak_memory_bytes,gap\n";

    for(const auto& entry : fs::directory_iterator(path)) {
        // Abre el archivo y guarda los atributos N,M,V,P + matrices de tipo y valores
        if (!entry.is_regular_file()) {
            continue;
        }
        std::ifstream input(entry.path());
        if (!input) {
            throw std::runtime_error("No se pudo abrir " + path.string());
        }
        int N,M,V,P;
        input>>N>>M>>V>>P;
        std::vector<std::vector<int>> matrixTypes(N,std::vector<int>(M));
        std::vector<std::vector<long long>> matrixValues(N,std::vector<long long>(M));
        for(int i=0;i<N;i++) {
            for(int j=0;j<M;j++){
                input>>matrixTypes[i][j];
            }
        }
        for(int i=0;i<N;i++) {
            for(int j=0;j<M;j++){
                input>>matrixValues[i][j];
            }
        }
        if (!input) {
            throw std::runtime_error("Dato inválido en " + path.string());
        }

        const std::string stem = entry.path().stem().string();
        PathResult bruteForce{false, 0, "", 0};
        double bruteTimeUs = 0;
        long long bruteMemory = -1;
        if (N+M <= 32) {
            const auto start = std::chrono::steady_clock::now();
            bruteForce = solveBruteForce(N,M,V,P,matrixTypes,matrixValues);
            const auto end = std::chrono::steady_clock::now();
            bruteTimeUs = std::chrono::duration<double, std::micro>(end-start).count();
            bruteMemory = peakMemoryBytes();
        }

        const auto dpStart = std::chrono::steady_clock::now();
        PathResult dp=solveDP(N,M,V,P,matrixTypes,matrixValues);
        const auto dpEnd = std::chrono::steady_clock::now();
        const double dpTimeUs = std::chrono::duration<double, std::micro>(dpEnd-dpStart).count();
        const long long dpMemory = peakMemoryBytes();

        if (N+M <= 32 &&
            (bruteForce.found != dp.found ||
             (bruteForce.found && bruteForce.score != dp.score))) {
            throw std::runtime_error("Fuerza bruta y DP difieren en " + stem);
        }

        write_output_file("data/grid_output/"+stem+"_dp_out.txt",dp);

        const auto greedyStart = std::chrono::steady_clock::now();
        PathResult greedy=solveGreedy(N,M,V,P,matrixTypes,matrixValues);
        const auto greedyEnd = std::chrono::steady_clock::now();
        const double greedyTimeUs = std::chrono::duration<double, std::micro>(greedyEnd-greedyStart).count();
        const long long greedyMemory = peakMemoryBytes();
        write_output_file("data/grid_output/"+stem+"_greedy_out.txt",greedy);

        if (N+M <= 32) {
            write_output_file("data/grid_output/"+stem+"_bruteforce_out.txt",bruteForce);
            write_measurement(measurements, stem, "bruteforce", bruteForce,
                              bruteTimeUs, bruteMemory);
        }
        write_measurement(measurements, stem, "dp", dp, dpTimeUs, dpMemory);
        std::string gap;
        if (greedy.found && dp.found) {
            const long long delta = dp.score - greedy.score;
            if (delta < 0) {
                throw std::runtime_error("Greedy supera a DP en " + stem);
            }
            gap = std::to_string(delta);
        }
        write_measurement(measurements, stem, "greedy", greedy,
                          greedyTimeUs, greedyMemory, gap);
        measurements.flush();
    }
    return 0;
}
