import random
import colorama
from enum import Enum
from typing import NewType, Optional, List, Callable
from dataclasses import dataclass
from termcolor import colored


colorama.init()


class TurnResult(Enum):
    CONT = 1
    WIN = 2
    LOSE = 3


class MoveType(Enum):
    FLAG = 1
    DIG = 2


@dataclass
class Tile:
    Mine: bool
    Flagged: bool = False
    Dug: bool = False


@dataclass
class Coord:
    x: int
    y: int


@dataclass
class Move:
    moveType: MoveType
    coord: Coord


Board = NewType("Board", List[List[Tile]])


def makeBoard(size: int, mineChance: float = 0.05) -> Board:
    return Board([
        [
            Tile(Mine=random.random() < mineChance)
            for i in range(size)
        ]
        for i in range(size)
    ])


def displayBoard(board: Board, formatter: Callable[[Board, Coord], str]) -> str:
    output = ["\n"]
    output.append("   |" + "".join([colored(f"{i:^3}", "magenta") + "|"
                                    for i in range(len(board))]))
    rowSep = "---+" * (len(board) + 1)
    for y, row in enumerate(board):
        output.append(rowSep)
        output.append(
            colored(f"{y:^3}", "magenta") + "| " +
            " | ".join([formatter(board, Coord(x, y))
                        for x in range(len(row))])
            + " |")
    output.append(rowSep)
    return("\n".join(output))


def getAdjacent(board: Board, c: Coord) -> List[Coord]:
    def coordIsWithin(c: Coord):
        return c.x >= 0 and c.x < len(board) and c.y >= 0 and c.y < len(board)
    adjacent = [Coord(c.x - 1, c.y - 1), Coord(c.x, c.y - 1),
                Coord(c.x + 1, c.y - 1), Coord(c.x - 1, c.y),
                Coord(c.x + 1, c.y), Coord(c.x - 1, c.y + 1),
                Coord(c.x, c.y + 1), Coord(c.x + 1, c.y + 1)]
    return list(filter(coordIsWithin, adjacent))


def getAdjacentMines(board: Board, c: Coord) -> int:
    coords = getAdjacent(board, c)
    return len(list(filter(lambda c: board[c.y][c.x].Mine, coords)))


def revealFormatter(board: Board, c: Coord) -> str:
    if board[c.y][c.x].Mine:
        return colored("X", "red")
    else:
        numAdjacent = getAdjacentMines(board, c)
        return colored(str(numAdjacent), "yellow") if numAdjacent > 0 else " "


def defaultFormatter(board: Board, c: Coord) -> str:
    tile = board[c.y][c.x]
    if not tile.Dug:
        return colored("F", "red") if tile.Flagged else "#"
    if tile.Mine:
        return colored("X", "red")
    numAdjacent = getAdjacentMines(board, c)
    return colored(str(numAdjacent), "yellow") if numAdjacent > 0 else " "


def flag(board: Board, c: Coord) -> Board:
    newBoard = Board(board.copy())
    newBoard[c.y][c.x].Flagged = True
    return newBoard


def dig(board: Board, c: Coord) -> Board:
    newBoard = Board(board.copy())
    newBoard[c.y][c.x].Dug = True
    if newBoard[c.y][c.x].Mine:
        return newBoard
    if getAdjacentMines(board, c) == 0:
        adjCoords = getAdjacent(board, c)
        cleared = [c]
        while len(adjCoords) > 0:
            current = adjCoords.pop()
            newBoard[current.y][current.x].Dug = True
            cleared.append(current)
            if getAdjacentMines(board, current) == 0:
                adjCoords += [
                    coord for coord in getAdjacent(board, current)
                    if coord not in cleared
                ]
    return newBoard


def checkWin(board: Board) -> TurnResult:
    result = TurnResult.WIN
    for row in board:
        for tile in row:
            if tile.Dug:
                if tile.Mine:
                    return TurnResult.LOSE
            else:
                if not tile.Mine:
                    result = TurnResult.CONT
                else:
                    if not tile.Flagged:
                        result = TurnResult.CONT
    return result


def inputMove(board: Board) -> Move:
    valid = False
    tks = []
    while not valid:
        line = input("Enter move {d / f} x y : ")
        tks = line.split()
        if len(tks) == 3 and tks[0] in ["d", "f"]:
            if tks[1].isdigit() and tks[2].isdigit():
                if int(tks[1]) >= 0 and int(tks[1]) < len(board):
                    if int(tks[2]) >= 0 and int(tks[2]) < len(board):
                        valid = True
    moveType = MoveType.DIG if tks[0] == "d" else MoveType.FLAG
    coord = Coord(int(tks[1]), int(tks[2]))
    return Move(moveType, coord)


def playTurn(board: Board) -> Board:
    move = inputMove(board)
    if move.moveType == MoveType.DIG:
        return dig(board, move.coord)
    else:
        return flag(board, move.coord)


def playGame():
    states: List[Board] = []
    boardSize = int(input("Enter the size of the board: "))
    states.append(makeBoard(boardSize))
    over = False
    while not over:
        print(displayBoard(states[-1], defaultFormatter))
        states.append(playTurn(states[-1]))
        win = checkWin(states[-1])
        if win == TurnResult.WIN:
            print(displayBoard(states[-1], revealFormatter))
            print("You Win!")
            over = True
        if win == TurnResult.LOSE:
            print(displayBoard(states[-1], revealFormatter))
            print("You Lose!")
            over = True


playGame()
