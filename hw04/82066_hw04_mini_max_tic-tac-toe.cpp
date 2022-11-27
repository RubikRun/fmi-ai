#include <iostream>
#include <cassert>
#include <vector>
#include <algorithm>
#include <climits>
using namespace std;

// Enum for the state of the game, whether it's currently playing and if not who won it
enum GameState {
    Playing,
    UserWon,
    PcWon,
    Draw
};

// Enum for the possible values of a tile on the board
enum TileValue {
    Empty,
    Circle,
    Cross
};

// A 2-dimensional position of a tile on the board
struct TilePosition {
    int row, col;
};

// Matches the tile of the winner to the game state
GameState whoWonWithTile(TileValue tile) {
    assert(tile != TileValue::Empty);
    tile == TileValue::Circle ? GameState::UserWon : GameState::PcWon;
}

// Returns the character corresponding to the given tile value
char getTileChar(TileValue tile) {
    switch(tile) {
        case TileValue::Empty: return ' ';
        case TileValue::Circle: return 'O';
        default: return 'X';
    }
}

// Structure for the 3x3 board of the game
struct Board
{
    // Creates an empty board
    Board() {
        for (int i = 0; i < 9; i++) {
            tiles[i] = TileValue::Empty;
        }
    }

    // Gets the value of a tile
    TileValue getAt(TilePosition tilePos) const {
        assert(isTilePositionValid(tilePos));
        return tiles[getTileIndex(tilePos)];
    }

    // Sets the value of a tile
    void setAt(TilePosition tilePos, TileValue value) {
        assert(isTilePositionValid(tilePos));
        tiles[getTileIndex(tilePos)] = value;
    }

    // Prints the board to the console
    void print() const {
        cout << "-------------\n";
        for (int row = 0; row < 3; row++) {
            for (int col = 0; col < 3; col++) {
                const TileValue tile = getAt({ row, col });
                cout << "| " << getTileChar(tile) << " ";
            }
            cout << "|\n";
            cout << "-------------\n";
        }
    }

    // Checks whether someone has won and returns the game state
    GameState getGameState() const {
        // Look for winning rows
        for (int row = 0; row < 3; row++) {
            const TileValue baseTile = getAt({ row, 0 });
            if (baseTile != TileValue::Empty && getAt({ row, 1 }) == baseTile && getAt({ row, 2 }) == baseTile) {
                return whoWonWithTile(baseTile);
            }
        }
        // Look for winning cols
        for (int col = 0; col < 3; col++) {
            const TileValue baseTile = getAt({ 0, col });
            if (baseTile != TileValue::Empty && getAt({ 1, col }) == baseTile && getAt({ 2, col }) == baseTile) {
                return whoWonWithTile(baseTile);
            }
        }
        // Check diagonals
        const TileValue baseTile = getAt({ 1, 1 });
        if (baseTile != TileValue::Empty && (
            (getAt({ 0, 0 }) == baseTile && getAt({ 2, 2 }) == baseTile) ||
            (getAt({ 0, 2 }) == baseTile && getAt({ 2, 0 }) == baseTile)
        )) {
            return whoWonWithTile(baseTile);
        }
        // At this point no one wins, so either it's a draw or still playing
        return isFull() ? GameState::Draw : GameState::Playing;
    }

    vector<TilePosition> getEmptyTiles() const {
        vector<TilePosition> emptyTiles;
        for (int row = 0; row < 3; row++) {
            for (int col = 0; col < 3; col++) {
                const TilePosition tilePos = { row, col };
                if (isTileEmpty(tilePos)) {
                    emptyTiles.push_back(tilePos);
                }
            }
        }
        return emptyTiles;
    }

    // Checks if the board is full, that is all the tiles are non-empty
    bool isFull() const {
        for (int i = 0; i < 9; i++) {
            if (tiles[i] == TileValue::Empty) {
                return false;
            }
        }
        return true;
    }

    // Checks if the tile is empty
    bool isTileEmpty(TilePosition tilePos) const {
        return getAt(tilePos) == TileValue::Empty;
    }

    // Checks if the tile position is valid, meaning both coordinates are in range [0, 2]
    static bool isTilePositionValid(TilePosition tilePos) {
        return tilePos.row >= 0 && tilePos.row < 3 && tilePos.col >= 0 && tilePos.col < 3;
    }

private:
    // Returns an index into the tiles array corresponding to the requested tile position
    int getTileIndex(TilePosition tilePos) const {
        return tilePos.row * 3 + tilePos.col;
    }

    // Array for the values of the 3x3=9 tiles of the board
    TileValue tiles[9];
};

// Asks the user who starts the game and reads their answer.
// Returns true if user starts, false if PC starts
bool inputUserStarts() {
    cout << "Who starts?\n";
    cout << "  (1) User\n";
    cout << "  (2) PC\n";
    int option;
    cin >> option;
    return option == 1;
}

// Scores the finish state of a game for the mini-max algorithm.
// Scores > 0 mean that PC won, scores < 0 mean that user won. A score of 0 means draw.
// The depth at which the win happened determines the exact number.
// Each player wants to win as soon as possible, so with fewer turns, less depth.
// That's why the depth is substracted from the absolute value of the score.
// The bounds [-10, 10] for the scores are chosen because the maximum depth of tic-tac-toe is 9.
int scoreState(GameState gameState, int depth) {
    assert(gameState != GameState::Playing);
    switch(gameState) {
        case GameState::UserWon: return depth - 10;
        case GameState::PcWon: return 10 - depth;
        default: return 0;
    };
}

// The mini-max algorithm with alpha-beta prunning.
// This function does a depth-first search of the game tree.
// It alternates between maximizing or minimizing in each next depth, because players take turns.
// When it's maximizing, the move that carries the highest score is chosen, and vice versa.
// The alpha and beta values keep track of the current highest score seen at a maximizing level, and current lowest score seen at a minimizing level.
// If at some point alpha > beta, we can stop there and "prune" the tree.
// When called from the outside, the alpha and beta values need to be initialized to the worst possible (or worse), so <= -10 and >= +10,
// and the depth needs to be 0.
// The function returns the best score and fills the next move parameter with the next move that leads to this best score.
int miniMax(const Board &nodeBoard, bool maximizing, int depth, int alpha, int beta, TilePosition &outNextMove) {
    GameState nodeState = nodeBoard.getGameState();
    // If the game has finished, we need to score it and return the score
    if (nodeState != GameState::Playing) {
        return scoreState(nodeState, depth);
    }
    // Get the tile value of the current player. If maximizing, it's the PC, otherwise the user.
    const TileValue playerTileValue = maximizing ? TileValue::Cross : TileValue::Circle;
    // Get a list of all possible moves. They are the empty tiles of the board.
    vector<TilePosition> moves = nodeBoard.getEmptyTiles();
    // We will fill the scores of each child node to this list
    vector<int> childScores(moves.size());
    // Traverse moves to find the best move
    int bestMoveIdx = -1;
    for (int i = 0; i < moves.size(); i++) {
        assert(Board::isTilePositionValid(moves[i]) && nodeBoard.isTileEmpty(moves[i]));
        // Construct the child node's board. It's the same as the current board but with the current move applied.
        Board childBoard = nodeBoard;
        childBoard.setAt(moves[i], playerTileValue);
        // Score the child node using the mini-max algorithm recursively with alternating maximizing value and the next depth.
        childScores[i] = miniMax(childBoard, !maximizing, depth + 1, alpha, beta, outNextMove);
        // Based on whether we are maximizing or not
        if (maximizing) {
            // Update the best move if there's no best move yet or if the current move is better
            if (bestMoveIdx == -1 || childScores[i] > childScores[bestMoveIdx]) {
                bestMoveIdx = i;
            }
            // Update the alpha with the current score if it's more
            alpha = max(alpha, childScores[i]);
        }
        else {
            if (bestMoveIdx == -1 || childScores[i] < childScores[bestMoveIdx]) {
                bestMoveIdx = i;
            }
            // Update the beta with the currrent score if it's less
            beta = min(beta, childScores[i]);
        }
        // If we reach this, we can break and not check the remaining moves
        if (alpha > beta) {
            break;
        }
    }
    assert(bestMoveIdx != -1);
    // The best move is found. Fill it to the next move parameter and return the best score.
    outNextMove = moves[bestMoveIdx];
    return childScores[bestMoveIdx];
}

// Does a single PC's turn.
void doPcTurn(Board &board) {
    TilePosition pcMove;
    miniMax(board, true, 0, -10, 10, pcMove);
    board.setAt(pcMove, TileValue::Cross);
    board.print();
}

// Asks user to choose a tile.
// Keeps asking until a valid and empty tile is chosen.
// Fills the output row and col parameters with the resulting tile.
void askUserForTile(Board &board, int &outRow, int &outCol) {
    cout << "What's your turn?\n";
    outRow = -1;
    outCol = -1;
    while (outRow == -1 || outCol == -1) {
        // Input user's answer which is a row and a column of the target tile
        int row, col;
        cin >> row >> col;
        // Decrease both coordinates by 1 because we need them to start from 0
        row--;
        col--;
        // If the input tile is valid and empty, use it
        if (Board::isTilePositionValid({ row, col }) && board.isTileEmpty({ row, col })) {
            outRow = row;
            outCol = col;
        }
        // Otherwise print an error message and repeat
        else {
            cout << "Invalid input. Try again.\n";
        }
    }
}

// Does a single user's turn.
void doUserTurn(Board &board) {
    // Ask user to choose a tile
    int row, col;
    askUserForTile(board, row, col);
    // Put a circle on the chosen tile
    board.setAt({ row, col }, TileValue::Circle);
    // Print the board
    board.print();
}

// Does a single turn in the game.
// Depending on whether the turn parameter is even or odd, it can be a user's turn or a PC's turn
void doTurn(int turn, Board &board) {
    if (turn % 2 == 0) {
        doUserTurn(board);
    }
    else {
        doPcTurn(board);
    }
}

int main() {
    // Ask user who starts the game
    const bool userStarts = inputUserStarts();
    // An even turn will mean it's user's turn, an odd turn will mean it's PC's turn.
    // Begin with turn of 0 or 1 depending on who starts.
    int turn = userStarts ? 0 : 1;
    // The board of the game
    Board board;
    // Make turns until the game is finished
    while (board.getGameState() == GameState::Playing) {
        doTurn(turn, board);
        turn++;
    }
    const GameState finishState = board.getGameState();
    // Print a message about who won the game
    cout << "Game Over! ";
    if (finishState == GameState::UserWon) {
        cout << "You won!\n";
    }
    else if (finishState == GameState::PcWon) {
        cout << "PC won!\n";
    }
    else {
        cout << "It's a draw.\n";
    }
    return 0;
}