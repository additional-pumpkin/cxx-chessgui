#include "bitboard.hxx"

#include <bit>
#include <cstring>
#include <iostream>

#include "move.hxx"
#include "types.hxx"
namespace db {

constexpr Bitboard not_a_file = 18374403900871474942ULL;
constexpr Bitboard not_ab_file = 18229723555195321596ULL;
constexpr Bitboard not_h_file = 9187201950435737471ULL;
constexpr Bitboard not_gh_file = 4557430888798830399ULL;

#define occupancy(position) ((position).by_color[WHITE] | (position).by_color[BLACK])

constexpr Bitboard w_OO_mask = 0x60ULL;
constexpr Bitboard w_OOO_mask = 0xeULL;
constexpr Bitboard b_OO_mask = 0x6000000000000000ULL;
constexpr Bitboard b_OOO_mask = 0xe00000000000000ULL;

Board::Board()
{
    init();
}

std::string Board::print_bitboard(Bitboard bitboard)
{
    std::string s = "   +---+---+---+---+---+---+---+---+\n";
    for (Rank rank = RANK_8; rank >= RANK_1; --rank) {
        s += std::to_string(rank + 1) + "  ";
        for (File file = FILE_A; file <= FILE_H; ++file) {
            Square square = make_square(file, rank);
            s += (bitboard & square_bitboard(square) ? "| X " : "|   ");
        }
        s += "|\n   +---+---+---+---+---+---+---+---+\n";
    }
    s += "     A   B   C   D   E   F   G   H\n";
    return s;
}

std::string Board::print()
{
    std::string s;
    s += "by_piece:\n\n";
    for (PieceType p = PAWN; p <= KING; ++p) {
        s += piece_type_str.at(p) + "\n";
        s += print_bitboard(m_position.by_type[p]);
    }
    s += "\n\n\nby_color:\n\n";
    s += "WHITE\n";
    s += print_bitboard(m_position.by_color[WHITE]);
    s += "BLACK\n";
    s += print_bitboard(m_position.by_color[BLACK]);
    s += print_board_symbols();
    s += "Side to move: " + color_str.at(m_position.stm) + "\n";
    s += "En passant square: " + square_str.at(m_position.ep) + "\n";
    return s;
}

std::string print_castling_rights(int castling_rights)
{
    if (castling_rights == CASTLING_NONE)
        return "None";
    std::string s;
    s += "White: ";
    if (castling_rights & WHITE_CASTLING_OO)
        s += " OO";
    if (castling_rights & WHITE_CASTLING_OOO)
        s += " OOO";
    s += "   Black: ";
    if (castling_rights & BLACK_CASTLING_OO)
        s += " OO";
    if (castling_rights & BLACK_CASTLING_OOO)
        s += " OOO";

    return s;
}

std::string Board::print(const Position &position)
{
    std::string s;
    // s += "by_piece:\n\n";
    // for (PieceType p = PAWN; p <= KING; ++p)
    // {
    // s += piece_type_str.at(p) + "\n";
    // s += print_bitboard(position.by_type[p]);
    // }
    // s += "\n\n\nby_color:\n\n";
    // s += "WHITE\n";
    // s += print_bitboard(position.by_color[WHITE]);
    // s += "BLACK\n";
    // s += print_bitboard(position.by_color[BLACK]);
    s += print_board_symbols(position);
    s += "Side to move: " + color_str.at(position.stm) + "\n";
    s += "En passant square: " + square_str.at(position.ep) + "\n";
    s += "Castling rights: " + print_castling_rights(position.castling_rights) + "\n";
    s += "Half move clock: " + std::to_string(position.half_move_clock) + "\n";
    s += "Ply number: " + std::to_string(position.full_move) + "\n";

    return s;
}

std::string Board::print_board_symbols()
{
    std::string s = "   +---+---+---+---+---+---+---+---+\n";
    std::string symbol;
    for (Rank rank = RANK_8; rank >= RANK_1; --rank) {
        s += std::to_string(rank + 1) + "  ";
        for (File file = FILE_A; file <= FILE_H; ++file) {
            Square square = make_square(file, rank);
            symbol = get_piece_symbol(get_piece_at(square));
            if (symbol != "x")
                s += "| " + symbol + " ";
            else
                s += "|   ";
        }
        s += "|\n   +---+---+---+---+---+---+---+---+\n";
    }
    s += "     A   B   C   D   E   F   G   H\n";
    return s;
}
std::string Board::print_board_symbols(const Position &position)
{
    std::string s = "   +---+---+---+---+---+---+---+---+\n";
    std::string symbol;
    for (Rank rank = RANK_8; rank >= RANK_1; --rank) {
        s += std::to_string(rank + 1) + "  ";
        for (File file = FILE_A; file <= FILE_H; ++file) {
            Square square = make_square(file, rank);
            symbol = get_piece_symbol(get_piece_at(square, position));
            if (symbol != "x")
                s += "| " + symbol + " ";
            else
                s += "|   ";
        }
        s += "|\n   +---+---+---+---+---+---+---+---+\n";
    }
    s += "     A   B   C   D   E   F   G   H\n";
    return s;
}
void Board::clear_square(Square square)
{
    if (square == SQUARE_NONE)
        return;
    for (PieceType p = PAWN; p <= KING; ++p)
        m_position.by_type[p] &= ~square_bitboard(square);
    m_position.by_color[WHITE] &= ~square_bitboard(square);
    m_position.by_color[BLACK] &= ~square_bitboard(square);
    m_position.board[square] = PIECE_NONE;
}
void Board::clear_square(Square square, Position &position)
{
    if (square == SQUARE_NONE)
        return;
    for (PieceType p = PAWN; p <= KING; ++p)
        position.by_type[p] &= ~square_bitboard(square);
    position.by_color[WHITE] &= ~square_bitboard(square);
    position.by_color[BLACK] &= ~square_bitboard(square);
    position.board[square] = PIECE_NONE;
}
void Board::set_piece_at(Piece piece, Square square)
{
    clear_square(square);
    if (piece == PIECE_NONE || square == SQUARE_NONE)
        return;
    m_position.by_type[type_of(piece)] |= square_bitboard(square);
    m_position.by_color[color_of(piece)] |= square_bitboard(square);
    m_position.board[square] = piece;
}
void Board::set_piece_at(Piece piece, Square square, Position &position)
{
    clear_square(square, position);
    if (piece == PIECE_NONE || square == SQUARE_NONE)
        return;
    position.by_type[type_of(piece)] |= square_bitboard(square);
    position.by_color[color_of(piece)] |= square_bitboard(square);
    position.board[square] = piece;
}

Piece Board::get_piece_at(Square square)
{
    return m_position.board[square];
}
Piece Board::get_piece_at(Square square, const Position &position)
{
    return position.board[square];
}

void Board::set_ep_square(Square square)
{
    m_position.ep = square;
}

bool Board::is_piece_attack(Square from, Square to)
{
    Piece p = get_piece_at(from);
    bool legal = false;
    if (color_of(p) != m_position.stm)
        return false;
    switch (p) {
        case WHITE_PAWN:
            legal = m_pawn_attacks[WHITE][from] & m_position.by_color[BLACK] & square_bitboard(to) |
                    get_wpawn_pushes(square_bitboard(from), occupancy(m_position)) & square_bitboard(to) |
                    (square_bitboard(m_position.ep) & square_bitboard(to) &&
                     square_bitboard(from) << 7 & not_h_file & square_bitboard(m_position.ep) |
                         square_bitboard(from) << 9 & not_a_file & square_bitboard(m_position.ep));
            break;
        case BLACK_PAWN:
            legal = m_pawn_attacks[BLACK][from] & m_position.by_color[WHITE] & square_bitboard(to) |
                    get_bpawn_pushes(square_bitboard(from), occupancy(m_position)) & square_bitboard(to) |
                    (square_bitboard(m_position.ep) & square_bitboard(to) &&
                     square_bitboard(from) >> 7 & not_a_file & square_bitboard(m_position.ep) |
                         square_bitboard(from) >> 9 & not_h_file & square_bitboard(m_position.ep));
            break;
        case WHITE_KNIGHT:
        case BLACK_KNIGHT:
            legal = m_knight_attacks[from] & square_bitboard(to) & ~m_position.by_color[color_of(p)];
            break;
        case WHITE_BISHOP:
        case BLACK_BISHOP:
            legal = get_bishop_attacks(from, occupancy(m_position)) & square_bitboard(to) &
                    ~m_position.by_color[color_of(p)];
            break;
        case WHITE_ROOK:
        case BLACK_ROOK:
            legal =
                get_rook_attacks(from, occupancy(m_position)) & square_bitboard(to) & ~m_position.by_color[color_of(p)];
            break;
        case WHITE_QUEEN:
        case BLACK_QUEEN:
            legal = get_queen_attacks(from, occupancy(m_position)) & square_bitboard(to) &
                    ~m_position.by_color[color_of(p)];
            break;
        case WHITE_KING:
            legal =
                m_king_attacks[from] & square_bitboard(to) & ~m_position.attacks[BLACK] & ~m_position.by_color[WHITE] |
                (square_bitboard(from) & ~m_position.attacks[BLACK] &&
                 square_bitboard(to) & square_bitboard(C1) | square_bitboard(to) & square_bitboard(A1) &&
                 m_position.castling_rights & WHITE_CASTLING_OOO &&
                 !(w_OOO_mask & (occupancy(m_position)) | w_OOO_mask & m_position.attacks[BLACK])) |
                (square_bitboard(from) & ~m_position.attacks[BLACK] &&
                 square_bitboard(to) & square_bitboard(G1) | square_bitboard(to) & square_bitboard(H1) &&
                 m_position.castling_rights & WHITE_CASTLING_OO &&
                 !(w_OO_mask & (occupancy(m_position)) | w_OO_mask & m_position.attacks[BLACK]));
            break;
        case BLACK_KING:
            legal =
                m_king_attacks[from] & square_bitboard(to) & ~m_position.attacks[WHITE] & ~m_position.by_color[BLACK] |
                (square_bitboard(from) & ~m_position.attacks[WHITE] &&
                 square_bitboard(to) & square_bitboard(C8) | square_bitboard(to) & square_bitboard(A8) &&
                 m_position.castling_rights & BLACK_CASTLING_OOO &&
                 !(b_OOO_mask & (occupancy(m_position)) | b_OOO_mask & m_position.attacks[WHITE])) |
                (square_bitboard(from) & ~m_position.attacks[WHITE] &&
                 square_bitboard(to) & square_bitboard(G8) | square_bitboard(to) & square_bitboard(H8) &&
                 m_position.castling_rights & BLACK_CASTLING_OO &&
                 !(b_OO_mask & (occupancy(m_position)) | b_OO_mask & m_position.attacks[WHITE]));
            break;
        case PIECE_NONE:
            legal = false;
            break;
    }
    Piece promoted = PIECE_NONE;
    if (type_of(p) == PAWN && (rank_of(to) == RANK_8 || rank_of(to) == RANK_1)) {
        promoted = m_position.stm == WHITE ? WHITE_QUEEN : BLACK_QUEEN;
    }
    Move move;
    move.from = from;
    move.to = to;
    move.piece_moved = p;
    move.captured = get_piece_at(to);
    move.color = color_of(p);
    move.promoted = promoted;
    move.is_enpassant = false;
    move.is_castling = false;
    move.is_legal = legal;
    move.prev_ep = m_position.ep;
    move.castling_rights = m_position.castling_rights;
    if (move.piece_moved == db::Piece::WHITE_KING && move.from == db::Square::E1 &&
            (move.to == db::Square::A1 || move.to == db::Square::C1 || move.to == db::Square::H1 ||
             move.to == db::Square::G1) ||
        move.piece_moved == db::Piece::BLACK_KING && move.from == db::Square::E8 &&
            (move.to == db::Square::A8 || move.to == db::Square::C8 || move.to == db::Square::H8 ||
             move.to == db::Square::G8))
        move.is_castling = true;
    if (p == WHITE_PAWN)
        move.is_enpassant = square_bitboard(m_position.ep) & square_bitboard(to) &&
                            square_bitboard(from) << 7 & not_h_file & square_bitboard(m_position.ep) |
                                square_bitboard(from) << 9 & not_a_file & square_bitboard(m_position.ep);
    else if (p == BLACK_PAWN)
        move.is_enpassant = square_bitboard(m_position.ep) & square_bitboard(to) &&
                            square_bitboard(from) >> 7 & not_a_file & square_bitboard(m_position.ep) |
                                square_bitboard(from) >> 9 & not_h_file & square_bitboard(m_position.ep);

    Position test_position = test_move(move, m_position);

    if (test_position.by_type[KING] & test_position.by_color[m_position.stm] &
        get_attacks(test_position, opposite(move.color))) {
        legal = false;
    }
    return legal;
}
Move Board::prepare_move(Move move)
{
    // Here we should test the move to see if it is legal
    bool legal = false;
    Square from = move.from;
    Square to = move.to;
    if (move.color != m_position.stm) {
        move.is_legal = false;
        return move;
    }
    if (move.piece_moved == WHITE_PAWN)
        move.is_enpassant = square_bitboard(m_position.ep) & square_bitboard(to) &&
                            square_bitboard(from) << 7 & not_h_file & square_bitboard(m_position.ep) |
                                square_bitboard(from) << 9 & not_a_file & square_bitboard(m_position.ep);
    else if (move.piece_moved == BLACK_PAWN)
        move.is_enpassant = square_bitboard(m_position.ep) & square_bitboard(to) &&
                            square_bitboard(from) >> 7 & not_a_file & square_bitboard(m_position.ep) |
                                square_bitboard(from) >> 9 & not_h_file & square_bitboard(m_position.ep);
    if (move.is_enpassant)
        move.captured = move.color == BLACK ? WHITE_PAWN : BLACK_PAWN;

    legal = is_piece_attack(from, to);
    // to deal with checks we will try to do the move and see if it results in a check
    move.prev_ep = m_position.ep;
    move.is_legal = legal;
    move.full_move = m_position.full_move;

    return move;
}
bool Board::do_move(const Move &move)
{
    if (!move.is_legal)
        return false;
    // increment half move clock
    if (type_of(move.piece_moved) == PAWN || move.captured != PIECE_NONE)
        m_position.half_move_clock = 0;
    else
        ++m_position.half_move_clock;
    // increment full move number
    if (move.color == BLACK)
        ++m_position.full_move;
    // set enpassant square
    if (abs(move.to - move.from) == 16 && type_of(move.piece_moved) == PAWN)
        m_position.ep = Square(move.to > move.from ? move.to - 8 : move.from - 8);
    // perform enpassant move
    if (move.is_enpassant) {
        clear_square(move.from);
        clear_square(Square(rank_of(m_position.ep) == RANK_3 ? m_position.ep + 8 : m_position.ep - 8));
        set_piece_at(move.piece_moved, m_position.ep);
        m_position.ep = SQUARE_NONE;
    }
    // I dont remember what this does... TODO: remember
    if (move.prev_ep == m_position.ep)
        m_position.ep = SQUARE_NONE;

    // when a rook is move remove respective castling right
    if (type_of(move.piece_moved) == ROOK) {
        if (move.from == H1 && m_position.castling_rights & WHITE_CASTLING_OO)
            m_position.castling_rights &= ~WHITE_CASTLING_OO;
        else if (move.from == A1 && m_position.castling_rights & WHITE_CASTLING_OOO)
            m_position.castling_rights &= ~WHITE_CASTLING_OOO;
        else if (move.from == H8 && m_position.castling_rights & BLACK_CASTLING_OO)
            m_position.castling_rights &= ~BLACK_CASTLING_OO;
        else if (move.from == A8 && m_position.castling_rights & BLACK_CASTLING_OOO)
            m_position.castling_rights &= ~BLACK_CASTLING_OOO;
    }

    // when a rook is captured remove respective castling right
    if (type_of(move.captured) == ROOK) {
        if (move.to == H1 && m_position.castling_rights & WHITE_CASTLING_OO)
            m_position.castling_rights &= ~WHITE_CASTLING_OO;
        else if (move.to == A1 && m_position.castling_rights & WHITE_CASTLING_OOO)
            m_position.castling_rights &= ~WHITE_CASTLING_OOO;
        else if (move.to == H8 && m_position.castling_rights & BLACK_CASTLING_OO)
            m_position.castling_rights &= ~BLACK_CASTLING_OO;
        else if (move.to == A8 && m_position.castling_rights & BLACK_CASTLING_OOO)
            m_position.castling_rights &= ~BLACK_CASTLING_OOO;
    }

    if (move.piece_moved == WHITE_KING)
        m_position.castling_rights &= ~WHITE_CASTLING_ALL;
    else if (move.piece_moved == BLACK_KING)
        m_position.castling_rights &= ~BLACK_CASTLING_ALL;

    if (move.is_castling) {
        if (move.to == A1 || move.to == C1) {
            clear_square(A1);
            clear_square(E1);
            set_piece_at(WHITE_KING, C1);
            set_piece_at(WHITE_ROOK, D1);
            m_position.castling_rights &= ~WHITE_CASTLING_ALL;
        } else if (move.to == H1 || move.to == G1) {
            clear_square(H1);
            clear_square(E1);
            set_piece_at(WHITE_KING, G1);
            set_piece_at(WHITE_ROOK, F1);
            m_position.castling_rights &= ~WHITE_CASTLING_ALL;
        } else if (move.to == A8 || move.to == C8) {
            clear_square(A8);
            clear_square(E8);
            set_piece_at(BLACK_KING, C8);
            set_piece_at(BLACK_ROOK, D8);
            m_position.castling_rights &= ~BLACK_CASTLING_ALL;
        } else if (move.to == H8 || move.to == G8) {
            clear_square(H8);
            clear_square(E8);
            set_piece_at(BLACK_KING, G8);
            set_piece_at(BLACK_ROOK, F8);
            m_position.castling_rights &= ~BLACK_CASTLING_ALL;
        }
    }

    m_position.stm = opposite(m_position.stm);
    if (!move.is_castling && !move.is_enpassant) {
        clear_square(move.from);
        if (move.promoted == PIECE_NONE)
            set_piece_at(move.piece_moved, move.to);
        else
            set_piece_at(move.promoted, move.to);
        update_attacks();
        return true;
    }
    update_attacks();
    return true;
}
Position Board::test_move(const Move &move, Position position)
{
    // if (!move.is_legal)
    // return false;

    if (abs(move.to - move.from) == 16 && type_of(move.piece_moved) == PAWN)
        position.ep = Square(move.to > move.from ? move.to - 8 : move.from - 8);

    if (move.is_enpassant) {
        clear_square(move.from, position);
        clear_square(Square(rank_of(position.ep) == RANK_3 ? position.ep + 8 : position.ep - 8), position);
        set_piece_at(move.piece_moved, position.ep, position);
        position.ep = SQUARE_NONE;
    }

    if (move.prev_ep == position.ep)
        position.ep = SQUARE_NONE;

    if (type_of(move.piece_moved) == ROOK) {
        if (move.from == H1 && position.castling_rights & WHITE_CASTLING_OO)
            position.castling_rights &= ~WHITE_CASTLING_OO;
        else if (move.from == A1 && position.castling_rights & WHITE_CASTLING_OOO)
            position.castling_rights &= ~WHITE_CASTLING_OOO;
        else if (move.from == H8 && position.castling_rights & BLACK_CASTLING_OO)
            position.castling_rights &= ~BLACK_CASTLING_OO;
        else if (move.from == A8 && position.castling_rights & BLACK_CASTLING_OOO)
            position.castling_rights &= ~BLACK_CASTLING_OOO;
    }

    if (move.piece_moved == WHITE_KING)
        position.castling_rights &= ~WHITE_CASTLING_ALL;
    else if (move.piece_moved == BLACK_KING)
        position.castling_rights &= ~BLACK_CASTLING_ALL;

    if (move.is_castling) {
        switch (move.to) {
            case A1:
            case C1:
                clear_square(A1, position);
                clear_square(E1, position);
                set_piece_at(WHITE_KING, C1, position);
                set_piece_at(WHITE_ROOK, D1, position);
                position.castling_rights &= WHITE_CASTLING_ALL;
                break;
            case H1:
            case G1:
                clear_square(H1, position);
                clear_square(E1, position);
                set_piece_at(WHITE_KING, G1, position);
                set_piece_at(WHITE_ROOK, F1, position);
                position.castling_rights &= WHITE_CASTLING_ALL;
                break;
            case A8:
            case C8:
                clear_square(A8, position);
                clear_square(E8, position);
                set_piece_at(BLACK_KING, C8, position);
                set_piece_at(BLACK_ROOK, D8, position);
                position.castling_rights &= BLACK_CASTLING_ALL;
                break;
            case H8:
            case G8:
                clear_square(H8, position);
                clear_square(E8, position);
                set_piece_at(BLACK_KING, G8, position);
                set_piece_at(BLACK_ROOK, F8, position);
                position.castling_rights &= BLACK_CASTLING_ALL;
                break;
        }
    }
    position.stm = opposite(position.stm);
    if (!move.is_castling && !move.is_enpassant) {
        clear_square(move.from, position);
        if (move.promoted == PIECE_NONE)
            set_piece_at(move.piece_moved, move.to, position);
        else
            set_piece_at(move.promoted, move.to, position);
        update_attacks(position);
        return position;
    }
    update_attacks(position);
    return position;
}
bool Board::undo_move(const Move &move)
{
    if (!move.is_legal)
        return false;
    set_piece_at(move.piece_moved, move.from);
    if (!move.is_enpassant)
        set_piece_at(move.captured, move.to);
    else {
        clear_square(move.to);
        set_piece_at(move.captured, move.color == BLACK ? Square(move.to + 8) : Square(move.to - 8));
    }

    if (move.is_castling) {
        switch (move.to) {
            case A1:
            case C1:
                clear_square(C1);
                clear_square(D1);
                set_piece_at(WHITE_KING, E1);
                set_piece_at(WHITE_ROOK, A1);
                break;
            case H1:
            case G1:
                clear_square(G1);
                clear_square(F1);
                set_piece_at(WHITE_KING, E1);
                set_piece_at(WHITE_ROOK, H1);
                break;
            case A8:
            case C8:
                clear_square(C8);
                clear_square(D8);
                set_piece_at(BLACK_KING, E8);
                set_piece_at(BLACK_ROOK, A8);
                break;
            case H8:
            case G8:
                clear_square(G8);
                clear_square(F8);
                set_piece_at(BLACK_KING, E8);
                set_piece_at(BLACK_ROOK, H8);
        }
    }
    m_position.castling_rights = move.castling_rights;
    m_position.ep = move.prev_ep;
    m_position.full_move = move.full_move;
    m_position.half_move_clock = move.half_move_clock;
    m_position.stm = move.color;
    update_attacks();
    return true;
}
Bitboard Board::get_wpawn_pushes(Bitboard pawns, Bitboard occupancy)
{
    Bitboard pushes = pawns << 8 & ~occupancy;
    pushes |= pushes << 8 & ~occupancy & rank_mask[RANK_4];
    return pushes;
}
Bitboard Board::get_bpawn_pushes(Bitboard pawns, Bitboard occupancy)
{
    Bitboard pushes = pawns >> 8 & ~occupancy;
    pushes |= pushes >> 8 & ~occupancy & rank_mask[RANK_5];
    return pushes;
}

std::vector<Move> Board::generate_moves(const Position &position)
{
    std::vector<Move> move_list;
    Square from = SQUARE_NONE, to = SQUARE_NONE;
    Bitboard movers = 0, moves = 0;
    // castle moves
    if (position.stm == WHITE) {
        if (position.by_type[KING] & position.by_color[WHITE] & ~position.attacks[BLACK] &&
            position.castling_rights & WHITE_CASTLING_OOO &&
            !(w_OOO_mask & (occupancy(position)) | w_OOO_mask & position.attacks[BLACK])) {
            Move m;
            m.is_castling = true;
            m.piece_moved = WHITE_KING;
            m.color = WHITE;
            m.from = E1;
            m.to = C1;
            m.is_legal = true;
            Position test_position = test_move(m, position);
            if (!(test_position.by_type[KING] & test_position.by_color[position.stm] &
                  get_attacks(test_position, opposite(m.color)))) {
                move_list.push_back(m);
            }
        }
        if (position.by_type[KING] & position.by_color[WHITE] & ~position.attacks[BLACK] &&
            position.castling_rights & WHITE_CASTLING_OO &&
            !(w_OO_mask & (occupancy(position)) | w_OO_mask & position.attacks[BLACK])) {
            Move m;
            m.is_castling = true;
            m.piece_moved = WHITE_KING;
            m.color = WHITE;
            m.from = E1;
            m.to = G1;
            m.is_legal = true;
            Position test_position = test_move(m, position);
            if (!(test_position.by_type[KING] & test_position.by_color[position.stm] &
                  get_attacks(test_position, opposite(m.color)))) {
                move_list.push_back(m);
            }
        }
        movers = position.by_type[PAWN] & position.by_color[position.stm];

        if (position.ep != SQUARE_NONE) {
            moves = m_pawn_attacks[BLACK][position.ep] & movers;
            while (moves) {
                from = Square(63 - std::countl_zero(moves));
                moves &= ~square_bitboard(from);
                Move m;
                m.from = from;
                m.to = position.ep;
                m.piece_moved = make_piece(PAWN, position.stm);
                m.color = position.stm;
                m.captured = make_piece(PAWN, opposite(position.stm));
                m.is_legal = true;
                m.is_enpassant = true;
                Position test_position = test_move(m, position);
                if (!(test_position.by_type[KING] & test_position.by_color[position.stm] &
                      get_attacks(test_position, opposite(m.color)))) {
                    move_list.push_back(m);
                }
            }
        }

        moves = (movers << 9) & not_a_file & position.by_color[opposite(position.stm)];
        while (moves) {
            to = Square(63 - std::countl_zero(moves));
            moves &= ~square_bitboard(to);
            if (rank_of(to) != RANK_8) {
                Move m;
                m.from = Square(to - 9);
                m.to = to;
                m.piece_moved = make_piece(PAWN, position.stm);
                m.color = position.stm;
                m.captured = get_piece_at(to, position);
                m.is_legal = true;
                Position test_position = test_move(m, position);
                if (!(test_position.by_type[KING] & test_position.by_color[position.stm] &
                      get_attacks(test_position, opposite(m.color)))) {
                    move_list.push_back(m);
                }

            } else {
                Move m;
                m.from = Square(to - 9);
                m.to = to;
                m.piece_moved = make_piece(PAWN, position.stm);
                m.color = position.stm;
                m.captured = get_piece_at(to, position);
                m.is_legal = true;
                m.promoted = WHITE_QUEEN;
                Position test_position = test_move(m, position);
                if (!(test_position.by_type[KING] & test_position.by_color[position.stm] &
                      get_attacks(test_position, opposite(m.color)))) {
                    move_list.push_back(m);
                    m.promoted = WHITE_KNIGHT;
                    move_list.push_back(m);
                    m.promoted = WHITE_ROOK;
                    move_list.push_back(m);
                    m.promoted = WHITE_BISHOP;
                    move_list.push_back(m);
                }
            }
        }

        moves = (movers << 7) & not_h_file & position.by_color[opposite(position.stm)];
        while (moves) {
            to = Square(63 - std::countl_zero(moves));
            moves &= ~square_bitboard(to);
            if (rank_of(to) != RANK_8) {
                Move m;
                m.from = Square(to - 7);
                m.to = to;
                m.piece_moved = make_piece(PAWN, position.stm);
                m.color = position.stm;
                m.captured = get_piece_at(to, position);
                m.is_legal = true;
                Position test_position = test_move(m, position);
                if (!(test_position.by_type[KING] & test_position.by_color[position.stm] &
                      get_attacks(test_position, opposite(m.color)))) {
                    move_list.push_back(m);
                }
            } else {
                Move m;
                m.from = Square(to - 7);
                m.to = to;
                m.piece_moved = make_piece(PAWN, position.stm);
                m.color = position.stm;
                m.captured = get_piece_at(to, position);
                m.is_legal = true;
                m.promoted = WHITE_QUEEN;
                Position test_position = test_move(m, position);
                if (!(test_position.by_type[KING] & test_position.by_color[position.stm] &
                      get_attacks(test_position, opposite(m.color)))) {
                    move_list.push_back(m);
                    m.promoted = WHITE_KNIGHT;
                    move_list.push_back(m);
                    m.promoted = WHITE_ROOK;
                    move_list.push_back(m);
                    m.promoted = WHITE_BISHOP;
                    move_list.push_back(m);
                }
            }
        }

        moves = ((position.by_type[PAWN] & position.by_color[position.stm]) << 8) & ~occupancy(position);
        movers = moves;
        while (moves) {
            to = Square(63 - std::countl_zero(moves));
            moves &= ~square_bitboard(to);
            if (rank_of(to) != RANK_8) {
                Move m;
                m.from = Square(to - 8);
                m.to = to;
                m.piece_moved = make_piece(PAWN, position.stm);
                m.color = position.stm;
                m.is_legal = true;
                Position test_position = test_move(m, position);
                if (!(test_position.by_type[KING] & test_position.by_color[position.stm] &
                      get_attacks(test_position, opposite(m.color)))) {
                    move_list.push_back(m);
                }
            } else {
                Move m;
                m.from = Square(to - 8);
                m.to = to;
                m.piece_moved = make_piece(PAWN, position.stm);
                m.color = position.stm;
                m.is_legal = true;
                m.promoted = WHITE_QUEEN;
                Position test_position = test_move(m, position);
                if (!(test_position.by_type[KING] & test_position.by_color[position.stm] &
                      get_attacks(test_position, opposite(m.color)))) {
                    move_list.push_back(m);
                    m.promoted = WHITE_KNIGHT;
                    move_list.push_back(m);
                    m.promoted = WHITE_ROOK;
                    move_list.push_back(m);
                    m.promoted = WHITE_BISHOP;
                    move_list.push_back(m);
                }
            }
        }

        moves = ((movers & rank_mask[RANK_3]) << 8) & ~occupancy(position);
        while (moves) {
            to = Square(63 - std::countl_zero(moves));
            moves &= ~square_bitboard(to);
            Move m;
            m.from = Square(to - 16);
            m.to = to;
            m.piece_moved = make_piece(PAWN, position.stm);
            m.color = position.stm;
            m.is_legal = true;
            Position test_position = test_move(m, position);
            if (!(test_position.by_type[KING] & test_position.by_color[position.stm] &
                  get_attacks(test_position, opposite(m.color)))) {
                move_list.push_back(m);
            }
        }

    } else {
        if (position.by_type[KING] & position.by_color[BLACK] & ~position.attacks[WHITE] &&
            position.castling_rights & BLACK_CASTLING_OOO &&
            !(b_OOO_mask & (occupancy(position)) | b_OOO_mask & position.attacks[WHITE])) {
            Move m;
            m.is_castling = true;
            m.piece_moved = BLACK_KING;
            m.color = BLACK;
            m.from = E8;
            m.to = C8;
            m.is_legal = true;
            Position test_position = test_move(m, position);
            if (!(test_position.by_type[KING] & test_position.by_color[position.stm] &
                  get_attacks(test_position, opposite(m.color)))) {
                move_list.push_back(m);
            }
        }
        if (position.by_type[KING] & position.by_color[BLACK] & ~position.attacks[WHITE] &&
            position.castling_rights & BLACK_CASTLING_OO &&
            !(b_OO_mask & (occupancy(position)) | b_OO_mask & position.attacks[WHITE])) {
            Move m;
            m.is_castling = true;
            m.piece_moved = BLACK_KING;
            m.color = BLACK;
            m.from = E8;
            m.to = G8;
            m.is_legal = true;
            Position test_position = test_move(m, position);
            if (!(test_position.by_type[KING] & test_position.by_color[position.stm] &
                  get_attacks(test_position, opposite(m.color)))) {
                move_list.push_back(m);
            }
        }

        movers = position.by_type[PAWN] & position.by_color[position.stm];

        if (position.ep != SQUARE_NONE) {
            moves = m_pawn_attacks[WHITE][position.ep] & movers;
            while (moves) {
                from = Square(63 - std::countl_zero(moves));
                moves &= ~square_bitboard(from);
                Move m;
                m.from = from;
                m.to = position.ep;
                m.piece_moved = make_piece(PAWN, position.stm);
                m.color = position.stm;
                m.captured = make_piece(PAWN, opposite(position.stm));
                m.is_legal = true;
                m.is_enpassant = true;
                Position test_position = test_move(m, position);
                if (!(test_position.by_type[KING] & test_position.by_color[position.stm] &
                      get_attacks(test_position, opposite(m.color)))) {
                    move_list.push_back(m);
                }
            }
        }

        moves = (movers >> 9) & not_h_file & position.by_color[opposite(position.stm)];
        while (moves) {
            to = Square(63 - std::countl_zero(moves));
            moves &= ~square_bitboard(to);
            if (rank_of(to) != RANK_1) {
                Move m;
                m.from = Square(to + 9);
                m.to = to;
                m.piece_moved = make_piece(PAWN, position.stm);
                m.color = position.stm;
                m.captured = get_piece_at(to, position);
                m.is_legal = true;
                Position test_position = test_move(m, position);
                if (!(test_position.by_type[KING] & test_position.by_color[position.stm] &
                      get_attacks(test_position, opposite(m.color)))) {
                    move_list.push_back(m);
                }
            } else {
                Move m;
                m.from = Square(to + 9);
                m.to = to;
                m.piece_moved = make_piece(PAWN, position.stm);
                m.color = position.stm;
                m.captured = get_piece_at(to, position);
                m.is_legal = true;
                m.promoted = BLACK_QUEEN;
                Position test_position = test_move(m, position);
                if (!(test_position.by_type[KING] & test_position.by_color[position.stm] &
                      get_attacks(test_position, opposite(m.color)))) {
                    move_list.push_back(m);
                    m.promoted = BLACK_KNIGHT;
                    move_list.push_back(m);
                    m.promoted = BLACK_ROOK;
                    move_list.push_back(m);
                    m.promoted = BLACK_BISHOP;
                    move_list.push_back(m);
                }
            }
        }

        moves = (movers >> 7) & not_a_file & position.by_color[opposite(position.stm)];
        while (moves) {
            to = Square(63 - std::countl_zero(moves));
            moves &= ~square_bitboard(to);
            if (rank_of(to) != RANK_1) {
                Move m;
                m.from = Square(to + 7);
                m.to = to;
                m.piece_moved = make_piece(PAWN, position.stm);
                m.color = position.stm;
                m.captured = get_piece_at(to, position);
                m.is_legal = true;
                Position test_position = test_move(m, position);
                if (!(test_position.by_type[KING] & test_position.by_color[position.stm] &
                      get_attacks(test_position, opposite(m.color)))) {
                    move_list.push_back(m);
                }
            } else {
                Move m;
                m.from = Square(to + 7);
                m.to = to;
                m.piece_moved = make_piece(PAWN, position.stm);
                m.color = position.stm;
                m.captured = get_piece_at(to, position);
                m.is_legal = true;
                m.promoted = BLACK_QUEEN;
                Position test_position = test_move(m, position);
                if (!(test_position.by_type[KING] & test_position.by_color[position.stm] &
                      get_attacks(test_position, opposite(m.color)))) {
                    move_list.push_back(m);
                    m.promoted = BLACK_KNIGHT;
                    move_list.push_back(m);
                    m.promoted = BLACK_ROOK;
                    move_list.push_back(m);
                    m.promoted = BLACK_BISHOP;
                    move_list.push_back(m);
                }
            }
        }

        moves = ((position.by_type[PAWN] & position.by_color[position.stm]) >> 8) & ~occupancy(position);
        movers = moves;
        while (moves) {
            to = Square(63 - std::countl_zero(moves));
            moves &= ~square_bitboard(to);
            if (rank_of(to) != RANK_1) {
                Move m;
                m.from = Square(to + 8);
                m.to = to;
                m.piece_moved = make_piece(PAWN, position.stm);
                m.color = position.stm;
                m.is_legal = true;
                Position test_position = test_move(m, position);
                if (!(test_position.by_type[KING] & test_position.by_color[position.stm] &
                      get_attacks(test_position, opposite(m.color)))) {
                    move_list.push_back(m);
                }
            } else {
                Move m;
                m.from = Square(to + 8);
                m.to = to;
                m.piece_moved = make_piece(PAWN, position.stm);
                m.color = position.stm;
                m.is_legal = true;
                m.promoted = BLACK_QUEEN;
                Position test_position = test_move(m, position);
                if (!(test_position.by_type[KING] & test_position.by_color[position.stm] &
                      get_attacks(test_position, opposite(m.color)))) {
                    move_list.push_back(m);
                    m.promoted = BLACK_KNIGHT;
                    move_list.push_back(m);
                    m.promoted = BLACK_ROOK;
                    move_list.push_back(m);
                    m.promoted = BLACK_BISHOP;
                    move_list.push_back(m);
                }
            }
        }

        moves = ((movers & rank_mask[RANK_6]) >> 8) & ~occupancy(position);
        while (moves) {
            to = Square(63 - std::countl_zero(moves));
            moves &= ~square_bitboard(to);
            Move m;
            m.from = Square(to + 16);
            m.to = to;
            m.piece_moved = make_piece(PAWN, position.stm);
            m.color = position.stm;
            m.is_legal = true;
            Position test_position = test_move(m, position);
            if (!(test_position.by_type[KING] & test_position.by_color[position.stm] &
                  get_attacks(test_position, opposite(m.color)))) {
                move_list.push_back(m);
            }
        }
    }

    // pawn pushes
    // (square_bitboard(position.ep) && position.by_type[PAWN] & position.by_color[WHITE] << 7 & not_h_file &
    // square_bitboard(position.ep) | position.by_type[PAWN] & position.by_color[WHITE] << 9 & not_a_file &
    // square_bitboard(position.ep));

    // knight moves
    movers = position.by_type[KNIGHT] & position.by_color[position.stm];
    while (movers) {
        from = Square(63 - std::countl_zero(movers));
        movers &= ~square_bitboard(from);
        moves = m_knight_attacks[from] & ~position.by_color[position.stm];
        while (moves) {
            to = Square(63 - std::countl_zero(moves));
            moves &= ~square_bitboard(to);
            Move m;
            m.from = from;
            m.to = to;
            m.piece_moved = make_piece(KNIGHT, position.stm);
            m.captured = get_piece_at(to, position);
            m.color = position.stm;
            m.is_legal = true;
            Position test_position = test_move(m, position);
            if (!(test_position.by_type[KING] & test_position.by_color[position.stm] &
                  get_attacks(test_position, opposite(m.color)))) {
                move_list.push_back(m);
            }
        }
    }

    // bishop moves
    movers = position.by_type[BISHOP] & position.by_color[position.stm];
    while (movers) {
        from = Square(63 - std::countl_zero(movers));
        movers &= ~square_bitboard(from);
        moves = get_bishop_attacks(from, occupancy(position)) & ~position.by_color[position.stm];
        while (moves) {
            to = Square(63 - std::countl_zero(moves));
            moves &= ~square_bitboard(to);
            Move m;
            m.from = from;
            m.to = to;
            m.piece_moved = make_piece(BISHOP, position.stm);
            m.captured = get_piece_at(to, position);
            m.color = position.stm;
            m.is_legal = true;
            Position test_position = test_move(m, position);
            if (!(test_position.by_type[KING] & test_position.by_color[position.stm] &
                  get_attacks(test_position, opposite(m.color)))) {
                move_list.push_back(m);
            }
        }
    }

    // rook moves
    movers = position.by_type[ROOK] & position.by_color[position.stm];
    while (movers) {
        from = Square(63 - std::countl_zero(movers));
        movers &= ~square_bitboard(from);
        moves = get_rook_attacks(from, occupancy(position)) & ~position.by_color[position.stm];
        while (moves) {
            to = Square(63 - std::countl_zero(moves));
            moves &= ~square_bitboard(to);
            Move m;
            m.from = from;
            m.to = to;
            m.piece_moved = make_piece(ROOK, position.stm);
            m.captured = get_piece_at(to, position);
            m.color = position.stm;
            m.is_legal = true;
            Position test_position = test_move(m, position);
            if (!(test_position.by_type[KING] & test_position.by_color[position.stm] &
                  get_attacks(test_position, opposite(m.color)))) {
                move_list.push_back(m);
            }
        }
    }

    // queen moves
    movers = position.by_type[QUEEN] & position.by_color[position.stm];
    while (movers) {
        from = Square(63 - std::countl_zero(movers));
        movers &= ~square_bitboard(from);
        moves = get_queen_attacks(from, occupancy(position)) & ~position.by_color[position.stm];
        while (moves) {
            to = Square(63 - std::countl_zero(moves));
            moves &= ~square_bitboard(to);
            Move m;
            m.from = from;
            m.to = to;
            m.piece_moved = make_piece(QUEEN, position.stm);
            m.captured = get_piece_at(to, position);
            m.color = position.stm;
            m.is_legal = true;
            Position test_position = test_move(m, position);
            if (!(test_position.by_type[KING] & test_position.by_color[position.stm] &
                  get_attacks(test_position, opposite(m.color)))) {
                move_list.push_back(m);
            }
        }
    }

    // king moves
    from = Square(63 - std::countl_zero(position.by_type[KING] & position.by_color[position.stm]));
    moves = m_king_attacks[from] & ~position.by_color[position.stm] & ~position.attacks[opposite(position.stm)];
    while (moves) {
        to = Square(63 - std::countl_zero(moves));
        moves &= ~square_bitboard(to);
        Move m;
        m.from = from;
        m.to = to;
        m.piece_moved = make_piece(KING, position.stm);
        m.captured = get_piece_at(to, position);
        m.color = position.stm;
        m.is_legal = true;
        Position test_position = test_move(m, position);
        if (!(test_position.by_type[KING] & test_position.by_color[position.stm] &
              get_attacks(test_position, opposite(m.color)))) {
            move_list.push_back(m);
        }
    }

    return move_list;
}

void Board::remove_illegal(const Move &move, Bitboard &b)
{
    Move m = move;
    if (b == 0)
        return;
    Bitboard b2 = b;
    for (auto sq = Square(63 - std::countl_zero(b2)); b2 != 0;
         b2 &= ~square_bitboard(sq), sq = Square(63 - std::countl_zero(b2))) {
        int zeros = std::countl_zero(b2);
        if (!is_piece_attack(sq, move.to))
            b &= ~square_bitboard(sq);
    }
}

void Board::prepare_for_print(Move &move)
{
    if (type_of(move.piece_moved) != PAWN && type_of(move.piece_moved) != KING) {
        Bitboard others = 0;
        switch (type_of(move.piece_moved)) {
            case KNIGHT:
                others = m_position.by_type[KNIGHT] & m_knight_attacks[move.to];
                break;
            case BISHOP:
                others = m_position.by_type[BISHOP] & get_bishop_attacks(move.to, occupancy(m_position));
                break;
            case ROOK:
                others = m_position.by_type[ROOK] & get_rook_attacks(move.to, occupancy(m_position));
                break;
            case QUEEN:
                others = m_position.by_type[QUEEN] & get_queen_attacks(move.to, occupancy(m_position));
                break;
        }

        others ^= square_bitboard(move.from);
        others &= m_position.by_color[move.color];
        if (others) {
            remove_illegal(move, others);
        }
        if (others) {
            if (others & rank_mask[rank_of(move.from)])
                move.needs_file = true;

            if (others & file_mask[file_of(move.from)])
                move.needs_rank = true;
            else
                move.needs_file = true;
        }
    }
    Position test_position = test_move(move, m_position);
    Bitboard attacks = get_attacks(test_position, move.color);
    if (test_position.by_type[KING] & test_position.by_color[opposite(move.color)] &
        get_attacks(test_position, move.color)) {
        // Bitboard attacks = get_attacks(test_position, move.color);
        auto move_list = generate_moves(test_position);
        if (generate_moves(test_position).empty()) {
            move.gives_mate = true;
        } else {
            move.gives_check = true;
        }
    }
}

bool Board::parse_fen(const std::string &fen, Position &position)
{
    position = {};
    uint curr_char = 0;
    // Field 1: parse piece positions
    for (Square square = A8; fen[curr_char] != ' ';) {
        char fen_char = fen[curr_char];
        // match ascii pieces within FEN string
        size_t id;
        // match empty square numbers within FEN string
        if (isdigit(fen_char)) {
            int offset = fen_char - '0';
            square = Square(square + offset);
            ++curr_char;
        }

        // match rank separator
        else if (fen_char == '/') {
            ++curr_char;
            square = Square(square - 16);
        } else if ((id = fen_char_pieces.find(fen_char)) != std::string::npos) {
            set_piece_at(Piece(id), square, position);
            ++square;
            ++curr_char;
        }

        else {
            // CHESSOPS_CORE_ERROR("Field 1: Malformed fen positions\n"); // error
            return false;
        }
    }
    ++curr_char;
    // Field 2: parse side to move
    if (fen[curr_char] == 'w' || fen[curr_char] == 'b') {
        position.stm = fen[curr_char] == 'w' ? WHITE : BLACK;
        ++curr_char;
    } else {
        // CHESSOPS_CORE_ERROR("Field 2: Invalid side to move\n"); // error
        return false;
    }

    if (fen[curr_char] != ' ') {
        // CHESSOPS_CORE_ERROR("Field 2: There should be a space at the end of field 2\n"); // error
        return false;
    }

    ++curr_char;

    // Field 3: parse castling rights

    while (fen[curr_char] != ' ') {
        switch (fen[curr_char]) {
            case '-':
                position.castling_rights = CASTLING_NONE;
                break;
            case 'K':
                position.castling_rights |= WHITE_CASTLING_OO;
                break;
            case 'Q':
                position.castling_rights |= WHITE_CASTLING_OOO;
                break;
            case 'k':
                position.castling_rights |= BLACK_CASTLING_OO;
                break;
            case 'q':
                position.castling_rights |= BLACK_CASTLING_OOO;
                break;

            default: /*CHESSOPS_CORE_ERROR("Field 3: Invalid character: {0}", fen[curr_char]);*/
                return false;
        }
        ++curr_char;
    }
    ++curr_char;
    // Field 4: parse enpassant square
    if (fen[curr_char] != '-') {
        if (fen[curr_char] >= 'a' && fen[curr_char] <= 'h' && fen[curr_char + 1] >= '1' && fen[curr_char + 1] <= '8') {
            File f = File(fen[curr_char] - 'a');
            Rank r = Rank(fen[curr_char + 1] - '1');
            position.ep = make_square(f, r);
            curr_char += 2;
        } else {
            // CHESSOPS_CORE_ERROR("Field 4: Invalid en passant square\n"); // error
            return false;
        }
    } else {
        position.ep = SQUARE_NONE;
        ++curr_char;
    }
    ++curr_char;
    // CHESSOPS_CORE_INFO("Field 5 has: \"{0}\"", fen.substr(curr_char));
    // field 5 half move number
    if (curr_char >= fen.length() || !isdigit(fen.at(curr_char))) {
        // CHESSOPS_CORE_ERROR("Field 5: Not a number\n"); // error
        return false;
    }
    std::string s;
    do {
        s += fen.at(curr_char);
        ++curr_char;
    } while ((isdigit(fen.at(curr_char))));

    position.half_move_clock = stoi(s);
    ++curr_char;

    // CHESSOPS_CORE_INFO("Field 6 has: \"{0}\"", fen.substr(curr_char));
    // field 6 full move number
    if (curr_char >= fen.length() || !isdigit(fen.at(curr_char))) {
        // CHESSOPS_CORE_ERROR("Field 6: Not a number\n"); // error
        return false;
    }
    s = "";
    do {
        s += fen.at(curr_char);
        ++curr_char;

    } while ((fen.length() != curr_char && isdigit(fen.at(curr_char))));
    int move_number = stoi(s);
    // position.full_move = position.stm == WHITE ? move_number * 2 : move_number * 2;
    position.full_move = move_number;
    ++curr_char;

    update_attacks();
    return true;
}

std::string Board::get_position_fen(const Position &position)
{
    std::string fen;
    char curr_char = 0;
    // Field 1: Piece placement
    for (Square square = A8;;) {
        Piece p = get_piece_at(square, position);
        int empty = 0;
        Rank current_rank = rank_of(square);
        for (; p == PIECE_NONE && rank_of(square) == current_rank; ++empty, p = get_piece_at(++square, position))
            ;
        if (empty != 0)
            fen.append(std::to_string(empty));
        for (; p != PIECE_NONE && rank_of(square) == current_rank;
             fen.push_back(fen_char_pieces.at(p)), p = get_piece_at(++square, position))
            ;
        if (rank_of(square) == current_rank)
            continue;
        fen.push_back('/');
        if (square - 1 == H1)
            break;
        current_rank = rank_of(square);
        square = Square(square - 16); // TODO: maybe overload operator-=
    }

    fen.push_back(' ');
    // Field 2: Side to move
    fen.push_back(position.stm == BLACK ? 'b' : 'w');
    fen.push_back(' ');
    // Field 3: Castling Rights
    if (position.castling_rights == CASTLING_NONE) {
        fen.push_back(' ');
    } else {
        if (position.castling_rights & WHITE_CASTLING_OO) {
            fen.push_back('K');
        }
        if (position.castling_rights & WHITE_CASTLING_OOO) {
            fen.push_back('Q');
        }
        if (position.castling_rights & BLACK_CASTLING_OO) {
            fen.push_back('k');
        }
        if (position.castling_rights & BLACK_CASTLING_OOO) {
            fen.push_back('q');
        }
    }
    fen.push_back(' ');
    // Field 4: enpassant square

    if (position.ep == SQUARE_NONE) {
        fen.push_back('-');
    } else {
        std::string sqr = square_str.at(position.ep);
        sqr[0] = (char)tolower(sqr[0]); // This is really really stupid...
        fen.append(sqr);
    }
    fen.push_back(' ');
    // Field 5: half move number
    fen.append(std::to_string(m_position.half_move_clock));
    fen.push_back(' ');

    // Field 6: full move number
    fen.append(std::to_string(m_position.full_move));

    return fen;
}

Piece Board::get_piece_at_from_bb(Square s)
{
    bool is_white = false;
    if (m_position.by_color[WHITE] & square_bitboard(s))
        is_white = true;

    if (m_position.by_type[PAWN] & square_bitboard(s)) {
        if (is_white)
            return WHITE_PAWN;
        else
            return BLACK_PAWN;
    }

    if (m_position.by_type[KNIGHT] & square_bitboard(s)) {
        if (is_white)
            return WHITE_KNIGHT;
        else
            return BLACK_KNIGHT;
    }

    if (m_position.by_type[BISHOP] & square_bitboard(s)) {
        if (is_white)
            return WHITE_BISHOP;
        else
            return BLACK_BISHOP;
    }

    if (m_position.by_type[ROOK] & square_bitboard(s)) {
        if (is_white)
            return WHITE_ROOK;
        else
            return BLACK_ROOK;
    }

    if (m_position.by_type[KING] & square_bitboard(s)) {
        if (is_white)
            return WHITE_KING;
        else
            return BLACK_KING;
    }

    if (m_position.by_type[QUEEN] & square_bitboard(s)) {
        if (is_white)
            return WHITE_QUEEN;
        else
            return BLACK_QUEEN;
    }
    return PIECE_NONE;
}

void Board::init()
{
    init_leaper_attacks();
    m_position.castling_rights = CASTLING_NONE;
    set_ep_square(SQUARE_NONE);
    m_position.stm = COLOR_NONE;
    // update_attacks(); // maybe this is not needed anymore
}

void Board::init_leaper_attacks()
{
    for (Square sq = A1; sq <= H8; ++sq) {
        // pawn attacks
        m_pawn_attacks[WHITE][sq] = 0;
        m_pawn_attacks[BLACK][sq] = 0;
        m_pawn_attacks[WHITE][sq] |= (square_bitboard(sq) << 7) & not_h_file;
        m_pawn_attacks[WHITE][sq] |= (square_bitboard(sq) << 9) & not_a_file;
        m_pawn_attacks[BLACK][sq] |= (square_bitboard(sq) >> 7) & not_a_file;
        m_pawn_attacks[BLACK][sq] |= (square_bitboard(sq) >> 9) & not_h_file;

        // knight attacks
        Bitboard l1 = square_bitboard(sq) >> 1 & not_h_file;
        Bitboard l2 = square_bitboard(sq) >> 2 & not_gh_file;
        Bitboard r1 = square_bitboard(sq) << 1 & not_a_file;
        Bitboard r2 = square_bitboard(sq) << 2 & not_ab_file;
        Bitboard h1 = l1 | r1;
        Bitboard h2 = l2 | r2;
        m_knight_attacks[sq] = (h1 << 16) | (h1 >> 16) | (h2 << 8) | (h2 >> 8);

        // king attacks

        Bitboard king_attacks = square_bitboard(sq) >> 1 & not_h_file | square_bitboard(sq) << 1 & not_a_file;
        Bitboard king_set = square_bitboard(sq) | king_attacks;
        king_attacks |= king_set << 8 | king_set >> 8;
        m_king_attacks[sq] = king_attacks;
    }
}

bool Board::is_enemy_piece_attack(Square from, Square to, const Position &position)
{
    Piece p = get_piece_at(from, position);
    // if (color_of(p) == position.stm)
    // return false;
    switch (p) {
        case WHITE_PAWN:
            return m_pawn_attacks[WHITE][from] & square_bitboard(to);
        case BLACK_PAWN:
            return m_pawn_attacks[BLACK][from] & square_bitboard(to);
        case WHITE_KNIGHT:
        case BLACK_KNIGHT:
            return m_knight_attacks[from] & square_bitboard(to);
        case WHITE_BISHOP:
        case BLACK_BISHOP:
            return get_bishop_attacks(from, occupancy(position)) & square_bitboard(to);
        case WHITE_ROOK:
        case BLACK_ROOK:
            return get_rook_attacks(from, occupancy(position)) & square_bitboard(to);
        case WHITE_QUEEN:
        case BLACK_QUEEN:
            return get_queen_attacks(from, occupancy(position)) & square_bitboard(to);
        case WHITE_KING:
        case BLACK_KING:
            return m_king_attacks[from] & square_bitboard(to);
        case PIECE_NONE:
            return false;
    }
    return false;
}

void Board::update_attacks()
{
    Color color = opposite(m_position.stm);
    m_position.attacks[color] = 0;
    for (Square from = A1; from <= H8; ++from) {
        if (square_bitboard(from) & m_position.by_color[color]) {
            for (Square to = A1; to <= H8; ++to) {
                if (from == to) {
                    continue;
                }
                if (is_enemy_piece_attack(from, to, m_position)) {
                    m_position.attacks[color] |= square_bitboard(to);
                }
            }
        }
    }
}

void Board::update_attacks(Position &position)
{
    Color color = opposite(position.stm);
    position.attacks[color] = 0;
    for (Square from = A1; from <= H8; ++from) {
        if (square_bitboard(from) & position.by_color[color]) {
            for (Square to = A1; to <= H8; ++to) {
                if (from == to) {
                    continue;
                }
                if (is_enemy_piece_attack(from, to, position)) {
                    position.attacks[color] |= square_bitboard(to);
                }
            }
        }
    }
}

Bitboard Board::get_attacks(const Position &position, Color color)
{
    Bitboard attacks = 0;
    for (Square from = A1; from <= H8; ++from) {
        if (square_bitboard(from) & position.by_color[color]) {
            for (Square to = A1; to <= H8; ++to) {
                if (from == to) {
                    continue;
                }
                if (is_enemy_piece_attack(from, to, position)) {
                    attacks |= square_bitboard(to);
                }
            }
        }
    }
    return attacks;
}

} // namespace db
