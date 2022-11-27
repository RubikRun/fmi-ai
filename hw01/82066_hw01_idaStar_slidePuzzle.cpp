#include <iostream>
#include <cassert>
#include <vector>
#include <queue>
#include <cmath>
#include <chrono>
#include <cstdio>
using namespace std;

const int DIST_MAX = 2147483647;
const int MAX_NODES = 100000;

struct Position {
    int row;
    int col;
};

typedef int Move;

struct Board {
    Board(int size, vector<int> cells)
        : size(size)
        , cells(cells)
    {
        assert(cells.size() == size * size);
        // Traverse cells to find the position of the empty cell
        for (Position cell = { 0, 0 }; cell.row < size; cell.row++) {
            for (cell.col = 0; cell.col < size; cell.col++) {
                if (getAt(cell) == 0) {
                    emptyCell = cell;
                }
            }
        }
    }

    void print() const {
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                cout << getAt({ i, j }) << " ";
            }
            cout << "\n";
        }
        cout << "\n";
    }

    int getSize() const {
        return size;
    }

    const vector<int>& getCells() const {
        return cells;
    }

    int getAt(const Position &position) const {
        if (!isValidPosition(position)) {
            assert(false);
            return -1;
        }
        return cells[getCellIndex(position)];
    }

    bool doMove(Move move, const Board &goal, int &heurDelta) {
        switch(move) {
            case 0: return doLeft(goal, heurDelta); break;
            case 1: return doRight(goal, heurDelta); break;
            case 2: return doUp(goal, heurDelta); break;
            case 3: return doDown(goal, heurDelta);
        }
    }

    int calcManh(const Board &goal) {
        int manhSum = 0;
        for (Position cell = { 0, 0 }; cell.row < size; cell.row++) {
            for (cell.col = 0; cell.col < size; cell.col++) {
                if (goal.getAt(cell) == 0) {
                    continue;
                }
                manhSum += calcManhSingle(goal, cell);
            }
        }
    }

    int calcManhSingle(const Board &goal, const Position &position) const {
        int val = getAt(position);
        Position goalPos;
        // Traverse cells to find the position of cell in the goal board containing the value from the requested position in the original board
        for (Position cell = { 0, 0 }; cell.row < size; cell.row++) {
            for (cell.col = 0; cell.col < size; cell.col++) {
                if (goal.getAt(cell) == val) {
                    goalPos = cell;
                }
            }
        }
        return abs(goalPos.row - position.row) + abs(goalPos.col - position.col);
    }

private:
    void setAt(const Position &position, int value) {
        if (!isValidPosition(position)) {
            assert(false);
            return;
        }
        cells[getCellIndex(position)] = value;
    }

    int getCellIndex(const Position &position) const {
        return position.row * size + position.col;
    }

    bool isValidPosition(const Position &position) const {
        return (position.row >= 0 && position.row < size && position.col >= 0 && position.col < size);
    }

    bool doLeft(const Board &goal, int &heurDelta) {
        const Position newEmptyCell = { emptyCell.row, emptyCell.col + 1 };
        if (!isValidPosition(newEmptyCell)) {
            return false;
        }
        heurDelta = -calcManhSingle(goal, newEmptyCell);
        setAt(emptyCell, getAt(newEmptyCell));
        setAt(newEmptyCell, 0);
        heurDelta += calcManhSingle(goal, emptyCell);
        emptyCell = newEmptyCell;
        return true;
    }

    bool doRight(const Board &goal, int &heurDelta) {
        const Position newEmptyCell = { emptyCell.row, emptyCell.col - 1 };
        if (!isValidPosition(newEmptyCell)) {
            return false;
        }
        heurDelta = -calcManhSingle(goal, newEmptyCell);
        setAt(emptyCell, getAt(newEmptyCell));
        setAt(newEmptyCell, 0);
        heurDelta += calcManhSingle(goal, emptyCell);
        emptyCell = newEmptyCell;
        return true;
    }
    
    bool doUp(const Board &goal, int &heurDelta) {
        const Position newEmptyCell = { emptyCell.row + 1, emptyCell.col };
        if (!isValidPosition(newEmptyCell)) {
            return false;
        }
        heurDelta = -calcManhSingle(goal, newEmptyCell);
        setAt(emptyCell, getAt(newEmptyCell));
        setAt(newEmptyCell, 0);
        heurDelta += calcManhSingle(goal, emptyCell);
        emptyCell = newEmptyCell;
        return true;
    }

    bool doDown(const Board &goal, int &heurDelta) {
        const Position newEmptyCell = { emptyCell.row - 1, emptyCell.col };
        if (!isValidPosition(newEmptyCell)) {
            return false;
        }
        heurDelta = -calcManhSingle(goal, newEmptyCell);
        setAt(emptyCell, getAt(newEmptyCell));
        setAt(newEmptyCell, 0);
        heurDelta += calcManhSingle(goal, emptyCell);
        emptyCell = newEmptyCell;
        return true;
    }

private:
    int size;
    vector<int> cells;

    Position emptyCell;
};

struct Node {
    Node(const Board &board)
        : board(board)
    {}

    Board board;
    
    vector<Move> moves;
    int heur = DIST_MAX;

    bool operator<(const Node &other) const {
        return this->moves.size() + this->heur > other.moves.size() + other.heur;
    }
};

bool isMovesOpposite(Move aMove, Move bMove) {
    return (aMove != bMove && aMove / 2 == bMove / 2);
}

bool idaStar(const Board &start, const Board &goal, std::vector<Move> &movesOut) {
    priority_queue<Node> pq;
    Node startNode(start);
    startNode.heur = startNode.board.calcManh(goal);
    pq.push(startNode);

    while (!pq.empty()) {
        const Node currNode = pq.top();
        pq.pop();
        // Traverse the 4 possible moves
        for (Move move = 0; move < 4; move++) {
            Node neighNode = currNode;
            int heurDelta;
            if ((!currNode.moves.empty() && isMovesOpposite(move, currNode.moves.back()))
                || !neighNode.board.doMove(move, goal, heurDelta)) {
                continue;
            }
            neighNode.moves.push_back(move);
            if (neighNode.board.getCells() == goal.getCells()) {
                movesOut = neighNode.moves;
                return true;
            }
            neighNode.heur += heurDelta;
            //neighNode.heur = neighNode.board.calcManh(goal);
            pq.push(neighNode);
        }
    }

    return false;
}

Board getGoalBoard(int size, int emptyIndex) {
    vector<int> cells(size * size, -1);
    int val = 1;
    for (int i = 0; val < size * size; i++) {
        if (i + 1 == emptyIndex) {
            cells[i] = 0;
            continue;
        }
        cells[i] = val;
        val++;
    }
    if (emptyIndex == -1) {
        cells[size * size - 1] = 0;
    }

    return Board(size, cells);
}

void printMove(Move move) {
    switch(move) {
        case 0: cout << "left\n"; break;
        case 1: cout << "right\n"; break;
        case 2: cout << "up\n"; break;
        case 3: cout << "down\n"; break;
    }
}

int main() {
    int size;
    cin >> size;
    size = int(sqrt(size + 1));
    int emptyIndex;
    cin >> emptyIndex;
    vector<int> cells(size * size, -1);
    for (int i = 0; i < size * size; i++) {
        cin >> cells[i];
    }

    const std::chrono::system_clock::time_point beginTime = std::chrono::system_clock::now();

    const Board startBoard(size, cells);
    const Board goalBoard = getGoalBoard(size, emptyIndex);


    for (int i = 0; i < 1334134130; i++){}

    vector<Move> moves;
    if (idaStar(startBoard, goalBoard, moves)) {
        const std::chrono::system_clock::time_point endTime = std::chrono::system_clock::now();
        const std::chrono::milliseconds duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - beginTime);

        cout << moves.size() << "\n";
        for (Move move : moves) {
            printMove(move);
        }

        printf("Finding the path took %.2f seconds\n", duration.count() / 1000.f);
    }

    return 0;
}