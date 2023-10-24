#pragma once

#include <array>
#include <cstdint>
#include <string>

namespace db {
// clang-format off
const std::array<std::string, 3> color_str =
{
    "WHITE", "BLACK", "COLOR_NONE"
};
const std::array<std::string, 13> piece_str =
{
    "WHITE_PAWN", "WHITE_KNIGHT", "WHITE_BISHOP", "WHITE_ROOK", "WHITE_QUEEN", "WHITE_KING",
    "BLACK_PAWN", "BLACK_KNIGHT", "BLACK_BISHOP", "BLACK_ROOK", "BLACK_QUEEN", "BLACK_KING",
    "PIECE_NONE",
};
const std::array<std::string, 13> piece_symbol =
{
    "♙","♘", "♗", "♖", "♕", "♔", "♟︎", "♞", "♝", "♜", "♛", "♚", "x"
};
const std::array<std::string, 7> piece_type_str =
{
    "PAWN", "KNIGHT", "BISHOP", "ROOK", "QUEEN", "KING","PIECE_TYPE_NONE",
};
const std::array<std::string, 65> square_str =
{
    "A1", "B1", "C1", "D1", "E1", "F1", "G1", "H1",
    "A2", "B2", "C2", "D2", "E2", "F2", "G2", "H2",
    "A3", "B3", "C3", "D3", "E3", "F3", "G3", "H3",
    "A4", "B4", "C4", "D4", "E4", "F4", "G4", "H4",
    "A5", "B5", "C5", "D5", "E5", "F5", "G5", "H5",
    "A6", "B6", "C6", "D6", "E6", "F6", "G6", "H6",
    "A7", "B7", "C7", "D7", "E7", "F7", "G7", "H7",
    "A8", "B8", "C8", "D8", "E8", "F8", "G8", "H8",
    "SQUARE_NONE"
};


using Bitboard = uint64_t;

enum Square: int
{
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8,
    SQUARE_NONE
};
const int RANK_WIDTH = 8;
const std::string fen_char_pieces = "PNBRQKpnbrqkx";
enum Rank: int
{
    RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8
};
enum File: int
{
    FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H
};
enum Piece
{
    WHITE_PAWN, WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN, WHITE_KING,
    BLACK_PAWN, BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN, BLACK_KING,
    PIECE_NONE,

};
enum PieceType
{
    PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, PIECE_TYPE_NONE,
};
const int PIECE_TYPES = 6;
enum Color
{
    WHITE, BLACK, COLOR_NONE
};

enum class Direction
{
    RIGHT = 1,
    LEFT = -RIGHT,
    UP = 8,
    DOWN = -UP,
    DOWN_LEFT = DOWN + LEFT,
    DOWN_RIGHT = DOWN + RIGHT,
    UP_LEFT = UP + LEFT,
    UP_RIGHT = UP + RIGHT
};
enum CastlingRights : int
{
    CASTLING_NONE = 0,
    WHITE_CASTLING_OOO = 1,
    WHITE_CASTLING_OO = WHITE_CASTLING_OOO << 1,
    BLACK_CASTLING_OOO = WHITE_CASTLING_OOO << 2,
    BLACK_CASTLING_OO = WHITE_CASTLING_OOO << 3,
    WHITE_CASTLING_ALL = WHITE_CASTLING_OOO | WHITE_CASTLING_OO,
    BLACK_CASTLING_ALL = BLACK_CASTLING_OOO | BLACK_CASTLING_OO,
    CASTLING_OO = WHITE_CASTLING_OO | BLACK_CASTLING_OO,
    CASTLING_OOO = WHITE_CASTLING_OOO | BLACK_CASTLING_OOO,
    CASTLING_ALL = WHITE_CASTLING_ALL | BLACK_CASTLING_ALL

};
// clang-format on
inline Rank &operator++(Rank &d)
{
    return d = Rank(int(d) + 1);
}
inline Rank &operator--(Rank &d)
{
    return d = Rank(int(d) - 1);
}

inline File &operator++(File &d)
{
    return d = File(int(d) + 1);
}
inline File &operator--(File &d)
{
    return d = File(int(d) - 1);
}

inline Square &operator++(Square &d)
{
    return d = Square(int(d) + 1);
}
inline Square &operator--(Square &d)
{
    return d = Square(int(d) - 1);
}

inline Piece &operator++(Piece &d)
{
    return d = Piece(int(d) + 1);
}
inline Piece &operator--(Piece &d)
{
    return d = Piece(int(d) - 1);
}

inline PieceType &operator++(PieceType &d)
{
    return d = PieceType(int(d) + 1);
}
inline PieceType &operator--(PieceType &d)
{
    return d = PieceType(int(d) - 1);
}

// inline Square& operator+(Square &s, Direction &d) { return s = Square(int(s) + int(d)); }

inline Square make_square(File f, Rank r)
{
    return Square(r * RANK_WIDTH + f);
}
// TODO: replace doing it manually with this function
inline Piece make_piece(PieceType p, Color c)
{
    if (p == PIECE_TYPE_NONE)
        return PIECE_NONE;
    return Piece(c == BLACK ? p + PIECE_TYPES : p);
}

inline PieceType type_of(Piece p)
{
    return PieceType(p >= PIECE_TYPES ? p - PIECE_TYPES : p);
}

inline Rank rank_of(Square s)
{
    return Rank(s / RANK_WIDTH);
}
inline File file_of(Square s)
{
    return File(s % RANK_WIDTH);
}
inline Color opposite(Color color)
{
    return color == BLACK ? WHITE : BLACK;
}
inline Color color_of(Piece p)
{
    return Color(p == PIECE_NONE ? COLOR_NONE : p >= PIECE_TYPES ? BLACK : WHITE);
}

inline std::string get_piece_symbol(Piece p)
{
    return fen_char_pieces.substr(p, 1);
}

constexpr Bitboard square_bitboard(Bitboard s)
{
    return 1ULL << s;
}

// #define popcount(x) __builtin_popcountll(x)

static Bitboard const file_mask_a = square_bitboard(A1) | square_bitboard(A2) | square_bitboard(A3) |
                                    square_bitboard(A4) | square_bitboard(A5) | square_bitboard(A6) |
                                    square_bitboard(A7) | square_bitboard(A8);
static Bitboard const file_mask_b = square_bitboard(B1) | square_bitboard(B2) | square_bitboard(B3) |
                                    square_bitboard(B4) | square_bitboard(B5) | square_bitboard(B6) |
                                    square_bitboard(B7) | square_bitboard(B8);
static Bitboard const file_mask_c = square_bitboard(C1) | square_bitboard(C2) | square_bitboard(C3) |
                                    square_bitboard(C4) | square_bitboard(C5) | square_bitboard(C6) |
                                    square_bitboard(C7) | square_bitboard(C8);
static Bitboard const file_mask_d = square_bitboard(D1) | square_bitboard(D2) | square_bitboard(D3) |
                                    square_bitboard(D4) | square_bitboard(D5) | square_bitboard(D6) |
                                    square_bitboard(D7) | square_bitboard(D8);
static Bitboard const file_mask_e = square_bitboard(E1) | square_bitboard(E2) | square_bitboard(E3) |
                                    square_bitboard(E4) | square_bitboard(E5) | square_bitboard(E6) |
                                    square_bitboard(E7) | square_bitboard(E8);
static Bitboard const file_mask_f = square_bitboard(F1) | square_bitboard(F2) | square_bitboard(F3) |
                                    square_bitboard(F4) | square_bitboard(F5) | square_bitboard(F6) |
                                    square_bitboard(F7) | square_bitboard(F8);
static Bitboard const file_mask_g = square_bitboard(G1) | square_bitboard(G2) | square_bitboard(G3) |
                                    square_bitboard(G4) | square_bitboard(G5) | square_bitboard(G6) |
                                    square_bitboard(G7) | square_bitboard(G8);
static Bitboard const file_mask_h = square_bitboard(H1) | square_bitboard(H2) | square_bitboard(H3) |
                                    square_bitboard(H4) | square_bitboard(H5) | square_bitboard(H6) |
                                    square_bitboard(H7) | square_bitboard(H8);

static Bitboard const rank_mask_1 = square_bitboard(A1) | square_bitboard(B1) | square_bitboard(C1) |
                                    square_bitboard(D1) | square_bitboard(E1) | square_bitboard(F1) |
                                    square_bitboard(G1) | square_bitboard(H1);
static Bitboard const rank_mask_2 = square_bitboard(A2) | square_bitboard(B2) | square_bitboard(C2) |
                                    square_bitboard(D2) | square_bitboard(E2) | square_bitboard(F2) |
                                    square_bitboard(G2) | square_bitboard(H2);
static Bitboard const rank_mask_3 = square_bitboard(A3) | square_bitboard(B3) | square_bitboard(C3) |
                                    square_bitboard(D3) | square_bitboard(E3) | square_bitboard(F3) |
                                    square_bitboard(G3) | square_bitboard(H3);
static Bitboard const rank_mask_4 = square_bitboard(A4) | square_bitboard(B4) | square_bitboard(C4) |
                                    square_bitboard(D4) | square_bitboard(E4) | square_bitboard(F4) |
                                    square_bitboard(G4) | square_bitboard(H4);
static Bitboard const rank_mask_5 = square_bitboard(A5) | square_bitboard(B5) | square_bitboard(C5) |
                                    square_bitboard(D5) | square_bitboard(E5) | square_bitboard(F5) |
                                    square_bitboard(G5) | square_bitboard(H5);
static Bitboard const rank_mask_6 = square_bitboard(A6) | square_bitboard(B6) | square_bitboard(C6) |
                                    square_bitboard(D6) | square_bitboard(E6) | square_bitboard(F6) |
                                    square_bitboard(G6) | square_bitboard(H6);
static Bitboard const rank_mask_7 = square_bitboard(A7) | square_bitboard(B7) | square_bitboard(C7) |
                                    square_bitboard(D7) | square_bitboard(E7) | square_bitboard(F7) |
                                    square_bitboard(G7) | square_bitboard(H7);
static Bitboard const rank_mask_8 = square_bitboard(A8) | square_bitboard(B8) | square_bitboard(C8) |
                                    square_bitboard(D8) | square_bitboard(E8) | square_bitboard(F8) |
                                    square_bitboard(G8) | square_bitboard(H8);

static constexpr std::array<Bitboard, 8> file_mask = {file_mask_a, file_mask_b, file_mask_c, file_mask_d,
                                                      file_mask_e, file_mask_f, file_mask_g, file_mask_h};
static constexpr std::array<Bitboard, 8> rank_mask = {rank_mask_1, rank_mask_2, rank_mask_3, rank_mask_4,
                                                      rank_mask_5, rank_mask_6, rank_mask_7, rank_mask_8};

} // namespace db
