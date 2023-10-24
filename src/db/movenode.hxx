#pragma once
#include <cstddef>
#include <cstdint>
#include <random>
#include <vector>

#include "move.hxx"
namespace db {
using MoveId = uint64_t;
using VariationId = uint64_t;
static std::random_device s_random_device;
static std::mt19937_64 s_engine(s_random_device());
static std::uniform_int_distribution<MoveId> s_uniform_distribution;

struct MoveNode
{
    MoveNode()
        : move(Move())
        , move_id(s_uniform_distribution(s_engine))
        , variation_id(s_uniform_distribution(s_engine))
        , variation_level(0)
    {}
    MoveNode(VariationId variation_id)
        : move(Move())
        , move_id(s_uniform_distribution(s_engine))
        , variation_id(variation_id)
        , variation_level(0)
    {}
    MoveNode(const Move &move, VariationId variation_id, size_t variation_level)
        : move(move)
        , move_id(s_uniform_distribution(s_engine))
        , variation_id(variation_id)
        , variation_level(variation_level)
    {}
    MoveNode(const Move &move, MoveId move_id, VariationId variation_id, size_t variation_level)
        : move(move)
        , move_id(move_id)
        , variation_id(variation_id)
        , variation_level(variation_level)
    {}
    Move move;
    MoveId move_id;
    VariationId variation_id;
    size_t variation_level;
};

} // namespace db
