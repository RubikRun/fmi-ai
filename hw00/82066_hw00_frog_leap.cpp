#include <iostream>

#define MAXN 2000

void printBoard(const char *board, const int n) {
    const int size = n * 2 + 1;
    for (int i = 0; i < size; i++) {
        std::cout << board[i];
    }
    std::cout << "\n";
}

void setupBoard(char *board, const int n) {
    const int size = n * 2 + 1;
    for (int i = 0; i < n; i++) {
        board[i] = 'R';
        board[size - 1 - i] = 'L';
    }
    board[n] = '_';
}

bool isGoalState(const char *board, const int n) {
    if (board[n] != '_') {
        return false;
    }
    const int size = n * 2 + 1;
    for (int i = 0; i < n; i++) {
        if (board[i] != 'L' || board[size - 1 - i] != 'R') {
            return false;
        }
    }
    return true;
}

int findEmptyIndex(const char *board, const int n) {
    if (board[n] == '_') {
        return n;
    }
    const int size = n * 2 + 1;
    for (int i = 0; i < n; i++) {
        if (board[i] == '_') {
            return i;
        }
        if (board[size - 1 - i] == '_') {
            return size - 1 - i;
        }
    }
    return -1;
}

bool dfs(char *board, const int n) {    
    if (isGoalState(board, n)) {
        printBoard(board, n);
        return true;
    }
    const int size = n * 2 + 1;
    const int emptyIndex = findEmptyIndex(board, n);
    for (int delta = -2; delta <= 2; delta++) {
        const int index = emptyIndex + delta;
        if (index < 0 || index >= size
            || delta == 0
            || (delta < 0 && board[index] == 'L')
            || (delta > 0 && board[index] == 'R')
        ) {
            continue;
        }
        board[emptyIndex] = board[index];
        board[index] = '_';
        const bool dfsResult = dfs(board, n);
        board[index] = board[emptyIndex];
        board[emptyIndex] = '_';
        if (dfsResult) {
            printBoard(board, n);
            return true;
        }
    }
    return false;
}

void solve(const int n) {
    char board[MAXN * 2 + 1];
    setupBoard(board, n);
    dfs(board, n);
}

int main() {
    int n;
    std::cin >> n;

    solve(n);

    return 0;
}