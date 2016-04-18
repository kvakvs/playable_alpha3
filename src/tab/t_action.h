#pragma once

#include <stdint.h>
#include <QDebug>

namespace bm {
namespace ai {

enum class ActionType: uint16_t {
    None,
    Move,
    Mine,
};

// Actions cost for Astar planning
float get_action_cost(ActionType at);

QDebug operator<< (QDebug d, ActionType at);

} // ns bm::ai
} // ns bm