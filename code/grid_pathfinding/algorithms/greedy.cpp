// Implementado según lo descrito en el enunciado
// Matías Jorquera, rol 202310127-3, INF-221, Tarea 2, 2026-2, Casa Central.

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

PathResult solveGreedy(
    int n, int m, int v, int p,
    const std::vector<std::vector<int>>& types,
    const std::vector<std::vector<long long>>& values) {
    const PathResult noSolution{false, 0, "", 0};
    if (v <= 0 || !hasValidDimensions(n, m, types, values) ||
        types[0][0] != 1) {
        return noSolution;
    }

    int row = 0;
    int col = 0;
    int lives = v;
    PathResult result{true, values[0][0], "", v};

    while (row != n - 1 || col != m - 1) {
        bool hasCandidate = false;
        int chosenRow = row;
        int chosenCol = col;
        int chosenType = 0;
        char chosenMove = '\0';
        long long chosenGain = 0;

        // Se evalúa primero R; al reemplazar solo ante una ganancia estricta,
        // un empate entre derecha y abajo se resuelve hacia la derecha.
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
            if (cellType == 0 || (cellType != 1 && cellType != -1) ||
                (cellType == -1 && lives == 1)) {
                continue;
            }

            const long long gain =
                (cellType == -1) ? -static_cast<long long>(p)
                                 : values[nextRow][nextCol];
            if (!hasCandidate || gain > chosenGain) {
                hasCandidate = true;
                chosenRow = nextRow;
                chosenCol = nextCol;
                chosenType = cellType;
                chosenMove = moves[direction];
                chosenGain = gain;
            }
        }

        if (!hasCandidate) {
            return noSolution;
        }

        result.path.push_back(chosenMove);
        row = chosenRow;
        col = chosenCol;
        if (chosenType == -1) {
            --lives;
            result.score -= p;
        } else {
            result.score += values[row][col];
        }
    }

    result.remaining_lives = lives;
    return result;
}
