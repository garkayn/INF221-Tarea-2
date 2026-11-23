// Se usa DFS para enumerar todas las secuencias de R y D;
// solo se abandona una rama cuando entra a una muralla o agota las vidas.
// Matías Jorquera, rol 202310127-3, INF-221, Tarea 2, 2026-2, Casa Central.

#include <functional>
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

PathResult solveBruteForce(
    int n, int m, int v, int p,
    const std::vector<std::vector<int>>& types,
    const std::vector<std::vector<long long>>& values) {

    const PathResult noSolution{false, 0, "", 0};
    if (v <= 0) return noSolution;

    PathResult best{false, 0, "", 0};
    std::string currentPath;

    std::function<void(int, int, int, long long)> explore = [&](int row, int col, int lives, long long score) {
            if (row == n - 1 && col == m - 1) {
                if (!best.found || score > best.score) {
                    best = PathResult{true, score, currentPath, lives};
                }
                return;
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
                if (cellType == 0 || (cellType != 1 && cellType != -1)) {
                    continue;
                }

                int nextLives = lives;
                long long nextScore = score;
                if (cellType == -1) {
                    if (nextLives == 1) {
                        continue;
                    }
                    --nextLives;
                    nextScore -= p;
                } else {
                    nextScore += values[nextRow][nextCol];
                }

                currentPath.push_back(moves[direction]);
                explore(nextRow, nextCol, nextLives, nextScore);
                currentPath.pop_back();
            }
        };

    explore(0, 0, v, values[0][0]);
    return best.found ? best : noSolution;
}
