#include <iostream>
#include <vector>
#include <random>
#include <string>


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


Board makeBoard(int size, float mineChance = 10) {
    std::random_device device;
    std::mt19937 gen(device());
    std::uniform_int_distribution<int> dist(0, 99);
    Board board(size);
    for (auto &row: board) {
        row.reserve(size);
        for (int i = 0; i < size; i++) {
            row.push_back({dist(gen) < mineChance, false, false});
        }
    }
    return board;
}


void displayTile(const Tile &tile, bool reveal) {
    if (reveal) {
        if (tile.Mine)
            std::cout << termcolor::red << "X" << termcolor::reset;
        else {
            std::cout << " ";
        }
    }
}


void displayBoard(const Board &board, bool reveal) {
    std::cout << "\n   |";
    for (int i = 0; i < board.size(); i++)
        std::cout << termcolor::magenta << fmt::format("{0:^3}", i) << termcolor::reset << "|";
    std::cout << "\n";
    for (int i = 0; i < board.size(); i++) {
        for (int j = 0; j <= board.size(); j++)
            std::cout << "---+";
        std::cout << "\n";
        std::cout << termcolor::magenta << fmt::format("{0:^3}", i) << termcolor::reset << "|";
        for (int j = 0; j < board.size(); j++) {
            std::cout << " ";
            displayTile(board[j][i], reveal);
            std::cout << " |";
        }
        std::cout << "\n";
    }
}


int main() {
    auto board = makeBoard(11);
    std::cout << termcolor::red << "Hello, World!" << termcolor::reset << std::endl;
    displayBoard(board, true);
    return 0;
}
