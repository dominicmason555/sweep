#include <iostream>
#include <vector>
#include <forward_list>
#include <random>
#include <string>
#include <algorithm>
#include <regex>


#define FMT_HEADER_ONLY


#include "../lib/termcolor.hpp"
#include "../lib/fmt/format.h"


enum class TurnResult {
    CONT, WIN, LOSE
};


enum class MoveType {
    FLAG, DIG
};


struct Tile {
    bool Mine;
    bool Flagged = false;
    bool Dug = false;
};


struct Coord {
    int x;
    int y;
};


struct Move {
    MoveType moveType;
    Coord coord;
};


typedef std::vector<std::vector<Tile>> Board;


bool operator==(const Coord &c1, const Coord &c2) {
    return c1.x == c2.x && c1.y == c2.y;
}


Board makeBoard(int size, float mineChance = 10) {
    std::random_device device;
    std::mt19937 gen(device());
    std::uniform_int_distribution<int> dist(0, 99); // Random number between 0 and 99
    Board board(size);
    for (auto &row: board) { // Make `size` rows of length `size`
        row.reserve(size);
        for (int i = 0; i < size; i++) {
            row.push_back({dist(gen) < mineChance, false, false});
        } // If the random number is under the mineChance, that tile is a mine
    }
    return board;
}


std::vector<Coord> getAdjacent(const Board &board, Coord c) {
    std::vector<Coord> adjacent = {{c.x - 1, c.y - 1},
                                   {c.x,     c.y - 1},
                                   {c.x + 1, c.y - 1},
                                   {c.x - 1, c.y},
                                   {c.x + 1, c.y},
                                   {c.x - 1, c.y + 1},
                                   {c.x,     c.y + 1},
                                   {c.x + 1, c.y + 1},}; // All adjacent tiles even if they're outside the board
    adjacent.erase(std::remove_if(adjacent.begin(), adjacent.end(),
                                  [board](Coord c) {
                                      return c.x < 0 || c.x >= board.size()
                                             || c.y < 0 || c.y >= board.size();
                                  } // Remove the ones outside the board to prevent out-of-bounds errors
    ), adjacent.end());
    return adjacent;
}


int getAdjacentMines(const Board &board, Coord c) {
    auto adjacent = getAdjacent(board, c);
    return std::count_if(adjacent.begin(), adjacent.end(), [board](Coord c) { return board[c.y][c.x].Mine; });
}


void displayTile(const Board &board, Coord c, bool reveal) {
    if (reveal) {
        if (board[c.y][c.x].Mine)
            std::cout << termcolor::red << "X" << termcolor::reset;
        else {
            int adjacentMines = getAdjacentMines(board, c);
            if (adjacentMines > 0) {
                std::cout << termcolor::yellow << adjacentMines << termcolor::reset;
            } else {
                std::cout << " ";
            }
        }
    } else {
        if (!board[c.y][c.x].Dug) {
            if (board[c.y][c.x].Flagged) {
                std::cout << termcolor::red << "F" << termcolor::reset;
            } else {
                std::cout << "#";
            }
        } else {
            if (board[c.y][c.x].Mine) {
                std::cout << termcolor::red << "X" << termcolor::reset;
            } else {
                int adjacentMines = getAdjacentMines(board, c);
                if (adjacentMines > 0) {
                    std::cout << termcolor::yellow << adjacentMines << termcolor::reset;
                } else {
                    std::cout << " ";
                }
            }
        }
    }
}


void displayBoard(const Board &board, bool reveal) {
    std::cout << "\n   |";
    for (int i = 0; i < board.size(); i++)
        std::cout << termcolor::magenta << fmt::format("{0:^3}", i) << termcolor::reset << "|"; // X axis
    std::cout << "\n";
    for (int i = 0; i < board.size(); i++) {
        for (int j = 0; j <= board.size(); j++)
            std::cout << "---+"; // Separator line
        std::cout << "\n";
        std::cout << termcolor::magenta << fmt::format("{0:^3}", i) << termcolor::reset << "|"; // Y index
        for (int j = 0; j < board.size(); j++) {
            std::cout << " ";
            displayTile(board, {j, i}, reveal); // Show one character for each tile, if reveal then show the mines
            std::cout << " |";
        }
        std::cout << "\n";
    }
    for (int j = 0; j <= board.size(); j++)
        std::cout << "---+"; // Separator line
    std::cout << "\n";
}


Board flag(const Board &board, Coord c) {
    Board newBoard = board;
    newBoard[c.y][c.x].Flagged = !newBoard[c.y][c.x].Flagged;
    return newBoard;
}


Board dig(const Board &board, Coord c) {
    Board newBoard = board;
    newBoard[c.y][c.x].Dug = true;
    if (newBoard[c.y][c.x].Mine || getAdjacentMines(board, c) != 0) {
        return newBoard; // No exploring necessary on a mine or a number tile, only empty tiles
    }
    auto adjCoords = getAdjacent(board, c);
    std::forward_list<Coord> adjList(adjCoords.begin(), adjCoords.end()); // Tiles to explore get added here
    std::vector<Coord> cleared; // Explored tiles go here so they don't get explored twice
    while (!adjList.empty()) {
        auto current = adjList.front();
        adjList.pop_front();
        newBoard[current.y][current.x].Dug = true;
        cleared.push_back(current); // The lookup could be faster with an `std::set` and an `operator<`
        if (getAdjacentMines(board, current) == 0) { // Only add the adjacent tiles if none of them are mines
            for (auto coord : getAdjacent(board, current)) {
                if (std::find(cleared.begin(), cleared.end(), coord) == cleared.end()) {
                    adjList.push_front(coord); // Add non-cleared adjacent tiles to clearing list
                }
            }
        }
    }
    return newBoard;
}


TurnResult checkWin(const Board &board) {
    auto result = TurnResult::WIN;
    for (const auto &row : board) {
        for (const auto &tile : row) {
            if (tile.Dug) {
                if (tile.Mine) {
                    return TurnResult::LOSE;
                }
            } else {
                if (!tile.Mine) {
                    result = TurnResult::CONT;
                } else {
                    if (!tile.Flagged) {
                        result = TurnResult::CONT;
                    }
                }
            }
        }
    }
    return result;
}


Move inputMove(const Board &board) {
    bool valid = false;
    std::string moveStr;
    const std::regex inExpr("([df]) (\\d+) (\\d+)");
    std::smatch moveMatch;
    MoveType mt;
    Coord coord;
    std::string moveTypeStr;
    while (!valid) {
        std::cout << "Enter Move ({d/f} x y):";
        if (std::getline(std::cin, moveStr)) {
            if (std::regex_match(moveStr, moveMatch, inExpr)) {
                if (moveMatch[1].str() == "d") {
                    mt = MoveType::DIG;
                    moveTypeStr =  "Dig at";
                } else {
                    mt = MoveType::FLAG;
                    moveTypeStr =  "Flag";
                }
                coord.x = std::stoi(moveMatch[2].str());
                coord.y = std::stoi(moveMatch[3].str());
                if (coord.x >= 0 && coord.x < board.size() && coord.y >= 0 && coord.y < board.size()) {
                    valid = true;
                }
            }
        }
    }
    fmt::print("{0} ({1}, {2})", moveTypeStr, coord.x, coord.y);
    return {mt, coord};
}


Board playTurn (const Board &board) {
    Move move = inputMove(board);
    if (move.moveType == MoveType::DIG) {
        return dig(board, move.coord);
    } else {
        return flag(board, move.coord);
    }
}


int main() {
    int boardSize;
    std::cout << "Enter board size:";
    std::cin >> boardSize;
    std::forward_list<Board> states;
    states.push_front(makeBoard(boardSize));
    bool over = false;
    while (!over) {
        displayBoard(states.front(), false);
        states.push_front(playTurn(states.front()));
        TurnResult res = checkWin(states.front());
        if (res == TurnResult::WIN) {
            displayBoard(states.front(), true);
            std::cout << "You Win!" << std::endl;
            over = true;
        }
        if (res == TurnResult::LOSE) {
            displayBoard(states.front(), true);
            std::cout << "You Lose!" << std::endl;
            over = true;
        }
    }
    return 0;
}
