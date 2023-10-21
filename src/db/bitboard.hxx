#pragma once
#include <Pext.hpp>
#include <algorithm>
#include <vector>

#include "move.hxx"
#include "types.hxx"

namespace db {

struct Position
{
    Position()
        : by_type{0}
        , by_color{0}
        , stm(COLOR_NONE)
        , ep(SQUARE_NONE)
        , castling_rights(CASTLING_NONE)
        , half_move_clock(0)
        , full_move(1)
        , attacks{0}
    {
        std::fill(&board[0], &board[0] + sizeof(board) / sizeof(board[0]), PIECE_NONE);
    }

    Bitboard by_type[6];
    Bitboard by_color[2];
    Piece board[65]; // faster piece retrieval. the 65th Piece is to support SQUARE_NONE and it should always be
                     // PIECE_NONE
    Color stm;
    Square ep; // enpassant square
    uint8_t castling_rights;
    // TODO: think of better names maybe
    uint32_t half_move_clock; // number of moves since last pawn move or capture
    uint32_t full_move;       // full move number in game (incremented after each half move)

    Bitboard attacks[2]; // all attacks for each color
};

// A bitboard based board representation
class Board
{
public:
    Board();

    static std::string print_bitboard(Bitboard bitboard);
    std::string print();
    std::string print(const Position &position);
    std::string print_board_symbols();
    std::string print_board_symbols(const Position &position);

    void clear_square(Square square);
    void clear_square(Square square, Position &position);

    void set_piece_at(Piece piece, Square square);
    void set_piece_at(Piece piece, Square square, Position &position);

    Piece get_piece_at(Square square);
    Piece get_piece_at(Square square, const Position &position);
    void set_ep_square(Square square);
    Square get_ep_square() { return m_position.ep; }

    bool is_check()
    {
        return m_position.by_color[m_position.stm] & m_position.by_type[KING] &
               m_position.attacks[opposite(m_position.stm)];
    }
    bool is_piece_attack(Square from, Square to);

    Move prepare_move(Move move);
    bool do_move(const Move &move);
    Position test_move(const Move &move, Position position);
    bool undo_move(const Move &move);
    bool set_fen(const std::string &fen) { return parse_fen(fen, m_position); }
    bool set_fen(const std::string &fen, Position &position) { return parse_fen(fen, position); }
    std::string get_fen() { return get_position_fen(m_position); }
    std::string get_fen(const Position &position) { return get_position_fen(position); }
    Color get_stm() { return m_position.stm; }
    uint8_t get_castling_rights() { return m_position.castling_rights; }
    bool debug_is_enemy_attack(Square sq) { return m_position.attacks[opposite(m_position.stm)] & square_bitboard(sq); }
    Position get_position() { return m_position; }

    std::vector<Move> generate_moves() { return generate_moves(m_position); }
    std::vector<Move> generate_moves(const Position &position);

    // removes illegal moves to aid disambiguation
    void remove_illegal(const Move &move, Bitboard &b);
    // prepares move for conversion to SAN (disambiguation)
    void prepare_for_print(Move &move);
    // assumes fen is valid
private:
    bool parse_fen(const std::string &fen, Position &position); // returns false if it fails
    std::string get_position_fen(const Position &position);

    Bitboard get_rook_attacks(Square square, Bitboard occupancy)
    {
        return Chess_Lookup::Lookup_Pext::Rook(square, occupancy);
    }
    Bitboard get_bishop_attacks(Square square, Bitboard occupancy)
    {
        return Chess_Lookup::Lookup_Pext::Bishop(square, occupancy);
    }
    Bitboard get_queen_attacks(Square square, Bitboard occupancy)
    {
        return Chess_Lookup::Lookup_Pext::Queen(square, occupancy);
    }

    Bitboard get_wpawn_pushes(Bitboard pawns, Bitboard occupancy);
    Bitboard get_bpawn_pushes(Bitboard pawns, Bitboard occupancy);

    void init();
    void init_leaper_attacks();

    bool is_enemy_piece_attack(Square from, Square to, const Position &position);

    void update_attacks();
    void update_attacks(Position &position);

    Bitboard get_attacks(const Position &position, Color color);

    Position m_position;

    Piece get_piece_at_from_bb(Square s);

    // leaper attacks precalculated
    Bitboard m_pawn_attacks[2][64];
    Bitboard m_knight_attacks[64];
    Bitboard m_king_attacks[64];
};
} // namespace db
