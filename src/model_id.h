#pragma once

#include <stdint.h>

namespace bm {

enum class ModelId: uint32_t {
    NIL = 0,
    Cursor, CursorRed,
    Wood,
    BeardedMan,
    Xyz,            // debug xyz axis
    MarkPick,       // mark areas designated for digging
    MarkRoute,      // debug: red route pin
    MarkArea,       // green X to mark area for commands
};

} // ns bm
