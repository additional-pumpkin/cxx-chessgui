#pragma once
#include <cstddef>
#include <vector>

#include "bitboard.hxx"
#include "movenode.hxx"

namespace db {
class Game
{
public:
    Game();

    [[nodiscard]] size_t find_move(MoveId id) const;
    void add_move(const Move &move);
    Move forward();
    Move back();
    [[nodiscard]] MoveId current_move() const { return m_current_move_id; }
    [[nodiscard]] std::vector<MoveNode> get_moves() const { return m_moves; }
    [[nodiscard]] std::string text() const;
    void dump_debug() const;

private:
    size_t m_current_move_index;
    MoveId m_current_move_id;

    Board m_board;
    std::vector<MoveNode> m_moves;
};

} // namespace db