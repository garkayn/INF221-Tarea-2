// Se usa tabulación y cada estado incluye celda y vidas.
// Para reconstruir el camino se guarda el movimiento usado para llegar a cada
// estado; los empates conservan la primera solución encontrada.
// Al elegir el estado final, un empate de puntaje prefiere más vidas.
// Matías Jorquera, rol 202310127-3, INF-221, Tarea 2, 2026-2, Casa Central.

#include <algorithm>
#include <limits>
#include <string>
#include <vector>

#ifndef PATH_RESULT_H
#define PATH_RESULT_H
struct PathResult {
    bool found;
    long long score;
    std::string path;
    int remaining_lives;
};
#endif

namespace {

bool hasValidDimensions(int n, int m,
                        const std::vector<std::vector<int>>& types,
                        const std::vector<std::vector<long long>>& values) {
    if (n <= 0 || m <= 0 || static_cast<int>(types.size()) != n ||
        static_cast<int>(values.size()) != n) {
        return false;
    }

    for (int i = 0; i < n; ++i) {
        if (static_cast<int>(types[i].size()) != m ||
            static_cast<int>(values[i].size()) != m) {
            return false;
        }
    }
    return true;
}

}  // namespace

PathResult solveDP(
    int n, int m, int v, int p,
    const std::vector<std::vector<int>>& types,
    const std::vector<std::vector<long long>>& values) {
    const PathResult noSolution{false, 0, "", 0};
    if (v <= 0 || !hasValidDimensions(n, m, types, values) ||
        types[0][0] != 1) {
        return noSolution;
    }

    const long long unreachable = std::numeric_limits<long long>::lowest();
    std::vector<std::vector<std::vector<long long>>> best(
        n, std::vector<std::vector<long long>>(m,
                                               std::vector<long long>(v + 1, unreachable)));
    std::vector<std::vector<std::vector<char>>> previousMove(
        n, std::vector<std::vector<char>>(m, std::vector<char>(v + 1, '\0')));

    best[0][0][v] = values[0][0];

    // Cada estado guarda el mejor puntaje para una celda y una cantidad exacta
    // de vidas. Como solo hay movimientos R y D, se puede recorrer la grilla
    // de arriba hacia abajo y de izquierda a derecha.
    for (int row = 0; row < n; ++row) {
        for (int col = 0; col < m; ++col) {
            for (int lives = 1; lives <= v; ++lives) {
                const long long currentScore = best[row][col][lives];
                if (currentScore == unreachable) {
                    continue;
                }

                const int nextRows[2] = {row, row + 1};
                const int nextCols[2] = {col + 1, col};
                const char moves[2] = {'R', 'D'};

                for (int direction = 0; direction < 2; ++direction) {
                    const int nextRow = nextRows[direction];
                    const int nextCol = nextCols[direction];
                    if (nextRow >= n || nextCol >= m) {
                        continue;
                    }

                    const int cellType = types[nextRow][nextCol];
                    if (cellType != 1 && cellType != -1) {
                        continue;
                    }

                    int nextLives = lives;
                    long long nextScore = currentScore;
                    if (cellType == -1) {
                        if (lives == 1) {
                            continue;
                        }
                        --nextLives;
                        nextScore -= p;
                    } else {
                        nextScore += values[nextRow][nextCol];
                    }

                    if (nextScore > best[nextRow][nextCol][nextLives]) {
                        best[nextRow][nextCol][nextLives] = nextScore;
                        previousMove[nextRow][nextCol][nextLives] = moves[direction];
                    }
                }
            }
        }
    }

    int bestLives = 0;
    long long bestScore = unreachable;
    for (int lives = v; lives >= 1; --lives) {
        if (best[n - 1][m - 1][lives] > bestScore) {
            bestScore = best[n - 1][m - 1][lives];
            bestLives = lives;
        }
    }

    if (bestLives == 0) {
        return noSolution;
    }

    std::string path;
    int row = n - 1;
    int col = m - 1;
    int lives = bestLives;
    while (row != 0 || col != 0) {
        const int destinationRow = row;
        const int destinationCol = col;
        const char move = previousMove[row][col][lives];
        path.push_back(move);
        if (move == 'R') {
            --col;
        } else {
            --row;
        }
        if (types[destinationRow][destinationCol] == -1) {
            ++lives;
        }
    }

    std::reverse(path.begin(), path.end());
    return PathResult{true, bestScore, path, bestLives};
}
