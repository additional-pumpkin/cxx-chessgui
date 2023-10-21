#include "game.hxx"

#include <QtCore>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <ranges>
#include <string>
namespace db {
Game::Game()
    : m_current_move_index(0)
    , m_current_move_id(0)
{
    m_moves.emplace_back(Move(), 0, 0, 0); // set the root MoveNode
    m_board.set_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR/ w KQkq - 0 1");
}

size_t Game::find_move(MoveId id) const
{
    auto move_node = std::find_if(m_moves.begin(), m_moves.end(),
                                  [id](const MoveNode &move_node) { return move_node.move_id == id; });
    return std::distance(m_moves.begin(), move_node);
}

void Game::add_move(const Move &move)
{
    qDebug() << text();
    m_board.do_move(move);
    assert(m_current_move_index < m_moves.size());
    // end of current variation
    bool end_of_variation = true;
    for (auto it = m_moves.begin() + m_current_move_index + 1; it != m_moves.end(); ++it) {
        if (it->variation_id == m_moves[m_current_move_index].variation_id) {
            end_of_variation = false;
            break;
        }
    }
    if (end_of_variation) {
        MoveNode node;
        node.move = move;
        node.variation_id = m_moves[m_current_move_index].variation_id;
        node.variation_level = m_moves[m_current_move_index].variation_level;
        m_moves.insert(m_moves.begin() + m_current_move_index + 1, node);
        m_current_move_id = node.move_id;
        m_current_move_index = std::distance(m_moves.begin(), m_moves.begin() + (signed)m_current_move_index + 1);
        return;
    } else {
        MoveNode node;
        node.move = move;
        node.variation_level = m_moves[m_current_move_index].variation_level + 1;
        auto position =
            std::find_if(m_moves.begin() + m_current_move_index + 2, m_moves.end(), [this](const MoveNode &move_node) {
                return move_node.variation_id == m_moves[m_current_move_index].variation_id;
            });
        if (position == m_moves.end()) {
            m_moves.insert(m_moves.begin() + m_current_move_index + 1, node);
            return;
        }
        m_moves.insert(position, node);
        m_current_move_id = node.move_id;
        m_current_move_index = std::distance(m_moves.begin(), position);

        return;
    }
}

Move Game::forward()
{
    if (m_current_move_index == m_moves.size() - 1) {
        return {};
    }
    Move move;
    auto next =
        std::find_if(m_moves.begin() + m_current_move_index + 1, m_moves.end(), [this](const MoveNode &move_node) {
            return move_node.variation_id == m_moves[m_current_move_index].variation_id;
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
    Move move;
    move = m_moves[m_current_move_index].move;
    if (m_moves[m_current_move_index - 1].variation_id == m_moves[m_current_move_index].variation_id) {
        --m_current_move_index;
        m_current_move_id = m_moves[m_current_move_index].move_id;
    } else {
        auto previous =
            std::find_if(std::reverse_iterator(m_moves.begin() + m_current_move_index), m_moves.rend(),
                         [this](const MoveNode &move_node) {
                             return move_node.variation_level == m_moves[m_current_move_index].variation_level ||
                                    move_node.variation_level == m_moves[m_current_move_index].variation_level - 1;
                         });
        m_current_move_index = std::distance(m_moves.begin(), previous.base()) - 1;

        m_current_move_id = previous->move_id;
    }
    m_board.undo_move(move);
    return move;
}

std::string Game::text() const
{
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

} // namespace db