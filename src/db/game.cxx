#include "game.hxx"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <fstream>
#include <ios>
#include <iostream>
#include <iterator>
#include <string>

#include "movenode.hxx"
namespace db {
Game::Game()
    : m_current_move_index(0)
    , m_current_move_id(0)
{
    m_moves.emplace_back(Move(), 0, 0, 0); // set the root MoveNode
    m_board.set_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR/ w KQkq - 0 1");
}

size_t Game::find_move(MoveId move_id) const
{
    auto move_node = std::find_if(m_moves.begin(), m_moves.end(), [move_id](const MoveNode &move_node) -> bool {
        return move_node.move_id == move_id;
    });
    return std::distance(m_moves.begin(), move_node);
}

void Game::add_move(const Move &move)
{
    m_board.do_move(move);
    assert(m_current_move_index < m_moves.size());
    MoveNode current_node = m_moves[m_current_move_index];
    // find end of current variation
    size_t end_of_variation = m_current_move_index;
    size_t end_of_subvariations = m_current_move_index + 1; // end of all variations above current

    bool passed_sub_vartions = false;
    for (auto node = m_moves.begin() + end_of_variation + 1; node != m_moves.end(); ++node) {
        if (node->variation_level < current_node.variation_level)
            break;
        if (node->variation_level > current_node.variation_level) {
            end_of_subvariations = std::distance(m_moves.begin(), node) + 1;
        } else if (node->variation_id == current_node.variation_id) {
            end_of_variation = std::distance(m_moves.begin(), node);
        }
    }
    // append to variation
    if (m_current_move_index == end_of_variation) {
        MoveNode node(move, current_node.variation_id, current_node.variation_level);
        m_moves.insert(m_moves.begin() + end_of_variation + 1, node);
        m_current_move_id = node.move_id;
        ++m_current_move_index;
        return;
    } else /* insert new variation */ {
        // check each variation
        VariationId current_variation = m_moves[end_of_subvariations].variation_id;
        size_t current_move_index = end_of_subvariations;
        for (auto node = m_moves.begin() + m_current_move_index; node != m_moves.begin() + end_of_subvariations;
             ++node) {
            if (node->variation_id == current_variation)
                continue;
            if (node->move == move) {
                current_move_index = std::distance(m_moves.begin(), node);
            } else {
                current_variation = node->variation_id;
            }
        }
        if (current_move_index == end_of_subvariations && m_moves[end_of_subvariations].move != move) {
            MoveNode node;
            node.move = move;
            node.variation_level = current_node.variation_level + 1;
            m_moves.insert(m_moves.begin() + end_of_subvariations, node);
        }
        m_current_move_id = m_moves[current_move_index].move_id;
        m_current_move_index = current_move_index;
        return;
    }
}

Move Game::forward()
{
    if (m_current_move_index == m_moves.size() - 1) {
        return {};
    }
    MoveNode current_node = m_moves[m_current_move_index];
    Move move;
    auto next = std::find_if(m_moves.begin() + m_current_move_index + 1, m_moves.end(),
                             [current_node](const MoveNode &move_node) -> bool {
                                 return move_node.variation_id == current_node.variation_id;
                             });
    if (next == m_moves.end()) {
        return {};
    }
    m_current_move_id = next->move_id;
    m_current_move_index = std::distance(m_moves.begin(), next);
    move = next->move;
    m_board.do_move(move);
    return move;
}

Move Game::back()
{
    if (m_current_move_index == 0) {
        return {};
    }
    MoveNode current_node = m_moves[m_current_move_index];
    Move move = current_node.move;
    m_board.undo_move(move);
    auto previous = std::find_if(std::reverse_iterator(m_moves.begin() + m_current_move_index), m_moves.rend(),
                                 [current_node](const MoveNode &move_node) -> bool {
                                     return move_node.variation_id == current_node.variation_id ||
                                            move_node.variation_level == current_node.variation_level - 1;
                                 });
    m_current_move_index = std::distance(m_moves.begin(), previous.base() - 1);
    m_current_move_id = previous->move_id;
    return move;
}

std::string Game::text() const
{
    dump_debug();
    std::string text;
    for (auto it = m_moves.begin() + 1; it != m_moves.end(); ++it) {
        if (it->variation_level > (it - 1)->variation_level) {
            text += "(";
            if (it->move.color == WHITE)
                text += std::to_string(it->move.full_move) + ". ";
            else
                text += std::to_string(it->move.full_move) + "... ";

        } else if (it->move.color == WHITE) {
            text += std::to_string(it->move.full_move) + ". ";
        }
        if (std::distance(m_moves.begin(), it) == m_current_move_index) {
            text += "[" + it->move.to_san() + "]";
        } else {
            text += it->move.to_san();
        }
        if (it + 1 != m_moves.end() && (it + 1)->variation_level < it->variation_level) {
            text += ")";
        }

        if (it + 1 == m_moves.end()) {
            text.append(it->variation_level, ')');
        } else if ((it + 1)->variation_id != it->variation_id && (it + 1)->variation_level == it->variation_level) {
            text += ")(";
        } else {
            text += " ";
        }
    }
    return text;
}

void Game::dump_debug() const
{
    std::ofstream dump_file("debug.toml", std::ios_base::out);
    for (auto move_node : m_moves) {
        dump_file << "[" << move_node.move.to_san() << "]\n";
        dump_file << "move_id = " << move_node.move_id << "\n";
        dump_file << "variation_id = " << move_node.variation_id << "\n";
        dump_file << "variation_level = " << move_node.variation_level << "\n";
        dump_file << "full_move = " << move_node.move.full_move << "\n";
        dump_file << "half_move = " << move_node.move.half_move_clock << "\n";
        dump_file << "\n";
    }
    dump_file.close();
}

} // namespace db