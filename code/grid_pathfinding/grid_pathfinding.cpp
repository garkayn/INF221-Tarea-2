#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <filesystem>

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

int main() {
    const fs::path& path = "data/grid_input";
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
        // Ejecuta y guarda Brute Force (si N+M <= 32), DP y Greedy midiendo tiempo y memoria. 
        if (N+M <= 32) {
            PathResult bruteForce=solveBruteForce(N,M,V,P,matrixTypes,matrixValues);
            write_output_file("data/grid_output/"+stem+"_bruteforce_out.txt",bruteForce);
        }
        PathResult dp=solveDP(N,M,V,P,matrixTypes,matrixValues);
        write_output_file("data/grid_output/"+stem+"_dp_out.txt",dp);

        PathResult greedy=solveGreedy(N,M,V,P,matrixTypes,matrixValues);
        write_output_file("data/grid_output/"+stem+"_greedy_out.txt",greedy);
        
        // 5. Guardar tiempos, memoria, puntajes y brechas en data/measurements/.
    }
    return 0;
}
