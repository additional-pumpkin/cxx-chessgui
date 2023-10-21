#pragma once

#include "types.hxx"
namespace db {
struct Move
{
    Move()
        : from(SQUARE_NONE)
        , to(SQUARE_NONE)
        , piece_moved(PIECE_NONE)
        , captured(PIECE_NONE)
        , color(COLOR_NONE)
        , promoted(PIECE_NONE)
        , is_enpassant(false)
        , is_castling(false)
        , is_legal(false)
        , half_move_clock(0)
        , full_move(0)
        , prev_ep(SQUARE_NONE)
        , castling_rights(CASTLING_NONE)
        , needs_file(false)
        , needs_rank(false)
        , gives_check(false)
        , gives_mate(false)
    {}

    Square from;
    Square to;
    Piece piece_moved;
    Piece captured;
    Color color;
    Piece promoted; // PIECE_NONE if no promotion occured
    bool is_enpassant;
    bool is_castling;
    bool is_legal;

    uint32_t half_move_clock; // number of moves since last pawn move or capture
    uint32_t full_move;       // ply number in game (incremented after each half move)
    Square prev_ep;           // previous en-passant square
    uint8_t castling_rights;

    // SAN information
    bool needs_file;
    bool needs_rank;
    bool gives_check;
    bool gives_mate;

    bool operator==(const Move &rhs)
    {
        return this->from == rhs.from && this->to == rhs.to && this->piece_moved == rhs.piece_moved &&
               this->captured == rhs.captured && this->color == rhs.color && this->promoted == rhs.promoted &&
               this->is_enpassant == rhs.is_enpassant && this->is_castling == rhs.is_castling &&
               this->is_legal == rhs.is_legal && this->half_move_clock == rhs.half_move_clock &&
               this->full_move == rhs.full_move && this->prev_ep == rhs.prev_ep &&
               this->castling_rights == rhs.castling_rights;
    }
    [[nodiscard]] std::string to_san() const
    {
        std::string san;
        if (type_of(this->piece_moved) != PAWN) {
            san.push_back(fen_char_pieces[type_of(this->piece_moved)]);
        }
        if (this->needs_file)
            san.push_back(file_of(this->from) + 'a');
        if (this->needs_rank)
            san.push_back(rank_of(this->from) + '1');
        if (this->captured != PIECE_NONE && type_of(this->piece_moved) == PAWN) {
            san.push_back(file_of(this->from) + 'a');
            san.push_back('x');
        } else if (this->captured != PIECE_NONE) {
            san.push_back('x');
        }
        san.push_back(file_of(this->to) + 'a');
        san.push_back(rank_of(this->to) + '1');
        if (this->promoted != PIECE_NONE) {
            san.push_back('=');
            san.push_back(fen_char_pieces[type_of(promoted)]);
        }
        if (this->gives_check) {
            san.push_back('+');
        } else if (this->gives_mate) {
            san.push_back('#');
        }

        return san;
    }

    [[nodiscard]] std::string to_symbol_san() const
    {
        std::string san;
        if (type_of(this->piece_moved) != PAWN) {
            san.append(piece_symbol[type_of(this->piece_moved)]);
        }
        if (this->needs_file)
            san.push_back(file_of(this->from) + 'a');
        if (this->needs_rank)
            san.push_back(rank_of(this->from) + '1');
        if (this->captured != PIECE_NONE && type_of(this->piece_moved) == PAWN) {
            san.push_back(file_of(this->from) + 'a');
            san.push_back('x');
        } else if (this->captured != PIECE_NONE) {
            san.push_back('x');
        }
        san.push_back(file_of(this->to) + 'a');
        san.push_back(rank_of(this->to) + '1');
        if (this->promoted != PIECE_NONE) {
            san.push_back('=');
            san.append(piece_symbol[type_of(promoted)]);
        }
        if (this->gives_check) {
            san.push_back('+');
        } else if (this->gives_mate) {
            san.push_back('#');
        }

        return san;
    }
};

} // namespace db